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

#ifndef __EMSCRIPTEN__
# include <openssl/ssl.h>
#endif




void mod_rdp::acl_update()
{
    if (this->enable_server_cert_external_validation) {
        auto const& message = this->ini.get<cfg::mod_rdp::server_cert_response>();
        if (message.empty()) {
            return;
        }

        if (message == "Ok" || message == "ok") {
            LOG(LOG_INFO, "Certificate was valid according to authentifier");
//            result = CertificateResult::valid;
        }
        else {
            LOG(LOG_INFO, "Certificate was invalid according to authentifier: %s", message);
//            result = CertificateResult::invalid;
            throw Error(ERR_TRANSPORT_TLS_CERTIFICATE_INVALID);
        }

        LOG(LOG_INFO, "rdp::graphic_events_.create_action_executor");
        this->private_rdp_negociation->graphic_event = this->graphic_events_.create_action_executor(this->time_base)
        .on_action(jln::one_shot([this](gdi::GraphicApi&) {
            bool const is_finish = this->private_rdp_negociation->rdp_negociation.recv_data(this->buf);
            if (is_finish) {
                this->negociation_result = this->private_rdp_negociation->rdp_negociation.get_result();
                if (this->buf.remaining()) {
                    LOG(LOG_INFO, "rdp::lambda::graphic_events_.create_action_executor");
                    this->private_rdp_negociation->graphic_event = this->graphic_events_.create_action_executor(this->time_base)
                    .on_action(jln::one_shot([this](gdi::GraphicApi& gd){
                        this->draw_event_impl(gd, sesman);
                    }));
                }
            }
        }));
    }
    return;
}
