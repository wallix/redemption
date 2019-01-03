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


struct Private_RdpNegociation
{
    RdpNegociation rdp_negociation;
    SessionReactor::GraphicEventPtr graphic_event;

    template<class... Ts>
    explicit Private_RdpNegociation(char const* program, char const* directory, Ts&&... xs)
      : rdp_negociation(static_cast<Ts&&>(xs)...)
    {
        this->rdp_negociation.set_program(program, directory);
    }

    operator RdpNegociation& () noexcept { return rdp_negociation; }
};

void mod_rdp::init_negociate_event_(
    const ClientInfo & info, TimeObj & timeobj, const ModRDPParams & mod_rdp_params,
    char const* program, char const* directory)
{
    auto check_error = [this](
        auto /*ctx*/, jln::ExitR er,
        gdi::GraphicApi&, RdpNegociation& rdp_negociation
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
        switch (rdp_negociation.get_state()) {
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
            #undef CASE
        }

        str_append(this->close_box_extra_message_ref, ' ', statedescr, " (", statestr, ')');

        LOG(LOG_ERR, "Creation of new mod 'RDP' failed at %s state. %s",
            statestr, statedescr);
        er.error = Error(ERR_SESSION_UNKNOWN_BACKEND);
        return er.to_result();
    };

    this->fd_event = this->session_reactor
    .create_graphic_fd_event(this->trans.get_fd(), jln::emplace<Private_RdpNegociation>(
        program, directory,
        this->channels.authorization_channels, this->channels.mod_channel_list,
        this->channels.auth_channel, this->channels.checkout_channel,
        this->decrypt, this->stc.encrypt, this->logon_info,
        this->channels.enable_auth_channel,
        this->trans, this->front, info, this->redir_info,
        this->gen, timeobj, mod_rdp_params, this->report_message,
        (this->channels.file_system_drive_manager.has_managed_drive() || this->channels.enable_session_probe)
    ))
    .set_timeout(std::chrono::milliseconds(0))
    .on_exit(check_error)
    .on_action(jln::exit_with_error<ERR_RDP_PROTOCOL>() /* replaced by on_timeout action*/)
    .on_timeout([this](JLN_TOP_TIMER_CTX ctx, gdi::GraphicApi& gd, RdpNegociation& rdp_negociation){
        gdi_clear_screen(gd, this->get_dim());
        LOG(LOG_INFO, "RdpNego::NEGO_STATE_INITIAL");
        rdp_negociation.start_negociation();

        return ctx.replace_action([this](
            JLN_TOP_CTX ctx, gdi::GraphicApi&, Private_RdpNegociation& private_rdp_negociation
        ){
            RdpNegociation& rdp_negociation = private_rdp_negociation;
            bool const is_finish = rdp_negociation.recv_data(this->buf);

            // RdpNego::recv_next_data set a new fd if tls
            int const fd = this->trans.get_fd();
            if (fd >= 0) {
                ctx.set_fd(fd);
            }

            if (!is_finish) {
                return ctx.need_more_data();
            }

            this->negociation_result = rdp_negociation.get_result();
            if (this->buf.remaining()) {
                private_rdp_negociation.graphic_event = ctx.get_reactor().create_graphic_event()
                .on_action(jln::one_shot([this](gdi::GraphicApi& gd){
                    this->draw_event_impl(this->session_reactor.get_current_time().tv_sec, gd);
                }));
            }

            // TODO replace_event()
            return ctx.disable_timeout()
            .replace_exit(jln::propagate_exit())
            .replace_action([this](auto ctx, gdi::GraphicApi& gd, Private_RdpNegociation& private_rdp_negociation){
                private_rdp_negociation.graphic_event.reset();
                this->draw_event(ctx.get_current_time().tv_sec, gd);
                return ctx.need_more_data();
            });
        })
        .set_or_disable_timeout(this->open_session_timeout, [this](
            JLN_TOP_TIMER_CTX ctx, gdi::GraphicApi&, RdpNegociation&
        ){
            if (this->error_message) {
                *this->error_message = "Logon timer expired!";
            }

            this->report_message.report("CONNECTION_FAILED", "Logon timer expired.");

            if (this->channels.enable_session_probe) {
                const bool disable_input_event     = false;
                const bool disable_graphics_update = false;
                this->disable_input_event_and_graphics_update(
                    disable_input_event, disable_graphics_update);
            }

            LOG(LOG_ERR,
                "Logon timer expired on %s. The session will be disconnected.",
                this->logon_info.hostname());
            return ctx.exception(Error(ERR_RDP_OPEN_SESSION_TIMEOUT));
        });
    });
}
