/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2018
Author(s): Jonathan Poelen
*/

#include "core/session_reactor.hpp"
#include "gdi/screen_functions.hpp"
#include "mod/rdp/rdp.hpp"
#include "mod/rdp/rdp_negociation.hpp"

#ifndef __EMSCRIPTEN__
# include <openssl/ssl.h>
#endif

namespace
{

#ifndef __EMSCRIPTEN__
    bool cert_to_escaped_string(X509& cert, std::string& output)
    {
        // TODO unique_ptr<BIO, BIO_delete>
        BIO* bio = BIO_new(BIO_s_mem());
        if (!bio) {
            return false;
        }

        if (!PEM_write_bio_X509(bio, &cert)) {
            BIO_free(bio);
            return false;
        }

        std::size_t pem_len = BIO_number_written(bio);
        output.resize(pem_len);
        std::fill(output.data(), output.data() + pem_len, 0);

        BIO_read(bio, output.data(), pem_len);
        BIO_free(bio);

        std::replace(output.begin(), output.end(), '\n', '\x01');

        return true;
    }
#endif

    struct PrivateRdpNegociation
    {
        RdpNegociation rdp_negociation;
        GraphicEventPtr graphic_event;
        const std::chrono::seconds open_session_timeout;
#ifndef __EMSCRIPTEN__
        SesmanEventPtr sesman_event;
        CertificateResult result = CertificateResult::wait;
#endif

        template<class... Ts>
        explicit PrivateRdpNegociation(
            std::chrono::seconds open_session_timeout,
            char const* program, char const* directory,
            Ts&&... xs)
        : rdp_negociation(static_cast<Ts&&>(xs)...)
        , open_session_timeout(open_session_timeout)
        {
            this->rdp_negociation.set_program(program, directory);
        }
    };

    using PrivateRdpNegociationPtr = std::unique_ptr<PrivateRdpNegociation>;
} // anonymous namespace

void mod_rdp::init_negociate_event_(
    const ClientInfo & info, Random & gen, TimeObj & timeobj,
    const ModRDPParams & mod_rdp_params, const TLSClientParams & tls_client_params, char const* program, char const* directory,
    const std::chrono::seconds open_session_timeout)
{
    auto check_error = [this](
        auto /*ctx*/, jln::ExitR er,
        gdi::GraphicApi&, PrivateRdpNegociationPtr& private_rdp_negociation
    ){
        if (er.status != jln::ExitR::Exception) {
            return er.to_result();
        }

        switch (er.error.id) {
            case ERR_TRANSPORT_TLS_CERTIFICATE_CHANGED:
            case ERR_TRANSPORT_TLS_CERTIFICATE_MISSED:
            case ERR_TRANSPORT_TLS_CERTIFICATE_CORRUPTED:
            case ERR_TRANSPORT_TLS_CERTIFICATE_INACCESSIBLE:
            case ERR_NLA_AUTHENTICATION_FAILED:
                return er.to_result();
            default: break;
        }

        const char * statestr = "UNKNOWN_STATE";
        const char * statedescr = "Unknown state.";
        switch (private_rdp_negociation->rdp_negociation.get_state()) {
            #define CASE(e, trkey)                      \
                case RdpNegociation::State::e:          \
                    statestr = "RDP_" #e;               \
                    statedescr = TR(trkey, this->lang); \
                break
            CASE(NEGO_INITIATE, trkeys::err_mod_rdp_nego);
            CASE(NEGO, trkeys::err_mod_rdp_nego);
            CASE(BASIC_SETTINGS_EXCHANGE, trkeys::err_mod_rdp_basic_settings_exchange);
            CASE(CHANNEL_CONNECTION_ATTACH_USER, trkeys::err_mod_rdp_channel_connection_attach_user);
            CASE(CHANNEL_JOIN_CONFIRM, trkeys::mod_rdp_channel_join_confirme);
            CASE(GET_LICENSE, trkeys::mod_rdp_get_license);
            CASE(TERMINATED, trkeys::err_mod_rdp_nego);
            #undef CASE
        }

        str_append(this->close_box_extra_message_ref, ' ', statedescr, " (", statestr, ')');

        LOG(LOG_ERR, "Creation of new mod 'RDP' failed at %s state. %s",
            statestr, statedescr);
        er.error = Error(ERR_SESSION_UNKNOWN_BACKEND);
        return er.to_result();
    };

    this->fd_event = this->graphic_fd_events_
    .create_top_executor(this->session_reactor, this->trans.get_fd(), std::make_unique<PrivateRdpNegociation>(
        open_session_timeout, program, directory,
        this->channels.channels_authorizations, this->channels.mod_channel_list,
        this->channels.auth_channel, this->channels.checkout_channel,
        this->decrypt, this->encrypt, this->logon_info,
        this->channels.enable_auth_channel,
        this->trans, this->front, info, this->redir_info,
        gen, timeobj, mod_rdp_params, this->report_message, this->license_store,
#ifndef __EMSCRIPTEN__
        this->channels.drive.file_system_drive_manager.has_managed_drive()
        || this->channels.session_probe.enable_session_probe,
        this->channels.remote_app.convert_remoteapp_to_desktop,
#else
        false,
        false,
#endif
        tls_client_params
    ))
    .set_timeout(std::chrono::milliseconds(0))
    .on_exit(check_error)
    .on_action(jln::exit_with_error<ERR_RDP_PROTOCOL>() /* replaced by on_timeout action*/)
    .on_timeout([this, enable_server_cert_external_validation
        = mod_rdp_params.enable_server_cert_external_validation
    ](
          JLN_TOP_TIMER_CTX ctx,
          gdi::GraphicApi& gd,
          PrivateRdpNegociationPtr& private_rdp_negociation
    ){
        RdpNegociation& rdp_negociation = private_rdp_negociation->rdp_negociation;

        gdi_clear_screen(gd, this->get_dim());
        LOG(LOG_INFO, "RdpNego::NEGO_STATE_INITIAL");

#ifndef __EMSCRIPTEN__
        if (enable_server_cert_external_validation) {
            rdp_negociation.set_cert_callback([this, &private_rdp_negociation](
                X509& certificate
            ) {
                auto& result = private_rdp_negociation->result;

                if (result != CertificateResult::wait) {
                    return result;
                }

                std::string blob_str;
                if (!cert_to_escaped_string(certificate, blob_str)) {
                    LOG(LOG_ERR, "cert_to_string failed");
                    return result;
                }

                // LOG(LOG_INFO, "cert pem: %s", blob_str);

                this->vars.set_acl<cfg::mod_rdp::server_cert>(blob_str);
                this->vars.get_mutable_ref<cfg::mod_rdp::server_cert_response>() = "";
                this->vars.ask<cfg::mod_rdp::server_cert_response>();

                private_rdp_negociation->sesman_event = this->sesman_events_
                .create_action_executor(this->session_reactor)
                .on_action([&result, &private_rdp_negociation, this](auto ctx, Inifile& ini){
                    auto const& message = ini.get<cfg::mod_rdp::server_cert_response>();
                    if (message.empty()) {
                        return ctx.ready();
                    }

                    if (message == "Ok" || message == "ok") {
                        LOG(LOG_INFO, "certificate was valid according to authentifier");
                        result = CertificateResult::valid;
                    }
                    else {
                        LOG(LOG_INFO, "certificate was invalid according to authentifier: %s", message);
                        result = CertificateResult::invalid;
                        throw Error(ERR_TRANSPORT_TLS_CERTIFICATE_INVALID);
                    }

                    private_rdp_negociation->graphic_event
                    = this->graphic_events_
                    .create_action_executor(this->session_reactor)
                    .on_action(jln::one_shot([this, &private_rdp_negociation](gdi::GraphicApi&) {
                        RdpNegociation& rdp_negociation = private_rdp_negociation->rdp_negociation;

                        bool const is_finish = rdp_negociation.recv_data(this->buf);
                        if (is_finish) {
                            this->negociation_result = rdp_negociation.get_result();
                            if (this->buf.remaining()) {
                                private_rdp_negociation->graphic_event = this->graphic_events_
                                .create_action_executor(this->session_reactor)
                                .on_action(jln::one_shot([this](gdi::GraphicApi& gd){
                                    this->draw_event_impl(gd);
                                }));
                            }
                        }
                    }));

                    return ctx.terminate();
                });

                return result;
            });
        }
#endif

        // TODO: check why we have this ctx here instead of direct access to reactor through this
        // if it's an underlying lifetime issue the have a potential problem
        // (but as lifetime of reactor and containers is session wide, we should not
        // have any problem if using a pointer)
        rdp_negociation.start_negociation();

        return ctx.replace_action([this](
            JLN_TOP_CTX ctx, gdi::GraphicApi&, PrivateRdpNegociationPtr& private_rdp_negociation
        ){
            bool const is_finish = private_rdp_negociation->rdp_negociation.recv_data(this->buf);

            // RdpNego::recv_next_data set a new fd if tls
            int const fd = this->trans.get_fd();
            if (fd >= 0) {
                ctx.set_fd(fd);
            }

            if (!is_finish) {
                return ctx.need_more_data();
            }

            this->negociation_result = private_rdp_negociation->rdp_negociation.get_result();
            if (this->buf.remaining()) {
                private_rdp_negociation->graphic_event = this->graphic_events_
                .create_action_executor(ctx.get_reactor())
                .on_action(jln::one_shot([this](gdi::GraphicApi& gd){
                    this->draw_event_impl(gd);
                }));
            }

            // TODO replace_event()
            return ctx.disable_timeout()
            .replace_exit(jln::propagate_exit())
            .replace_action([this](JLN_TOP_CTX ctx, gdi::GraphicApi& gd, PrivateRdpNegociationPtr& private_rdp_negociation){
                private_rdp_negociation.reset();
                this->draw_event(gd);
                return ctx.replace_action([this](JLN_TOP_CTX ctx, gdi::GraphicApi& gd, PrivateRdpNegociationPtr&){
                    this->draw_event(gd);
                    return ctx.need_more_data();
                });
            });
        })
        .set_or_disable_timeout(private_rdp_negociation->open_session_timeout, [this](
            JLN_TOP_TIMER_CTX ctx, gdi::GraphicApi&, PrivateRdpNegociationPtr&
        ){
            if (this->error_message) {
                *this->error_message = "Logon timer expired!";
            }

            this->report_message.report("CONNECTION_FAILED", "Logon timer expired.");

#ifndef __EMSCRIPTEN__
            if (this->channels.session_probe.enable_session_probe) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                this->disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);
            }
#endif

            LOG(LOG_ERR,
                "Logon timer expired on %s. The session will be disconnected.",
                this->logon_info.hostname());
            return ctx.exception(Error(ERR_RDP_OPEN_SESSION_TIMEOUT));
        });
    });
}
