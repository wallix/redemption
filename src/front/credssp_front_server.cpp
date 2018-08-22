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
Copyright (C) Wallix 2010-2018
Author(s): Jonathan Poelen
*/

#include "core/session_reactor.hpp"
#include "core/RDP/nla/nla.hpp"
#include "front/credssp_front_server.hpp"

struct CredsspFrontServer::D
{
    D(
        SessionReactor& session_reactor, Transport& trans, Random& rand,
        TpduBuffer& buf, Translation::language_t lang, bool& wait_ntlm_password,
        std::function<void(bool)>&& f)
    : session_reactor(session_reactor)
    , wait_ntlm_password(wait_ntlm_password)
    , buf(buf)
    , credssp_server(
        trans, false, false, rand, this->timeobj, extra_message, lang,
        [this, f = std::move(f)](SEC_WINNT_AUTH_IDENTITY& identity){
            // LOG(LOG_DEBUG, "nla");
            // LOG(LOG_DEBUG, "user=%s", identity.User.get_data());
            // LOG(LOG_DEBUG, "pass=%s", identity.Password.get_data());
            // LOG(LOG_DEBUG, "domain=%s", identity.Domain.get_data());

            this->wait_ntlm_password = true;
            this->identity = &identity;

            this->sesman_credssp_server = this->session_reactor.create_sesman_event()
            .on_action(jln::one_shot([this, &f](Inifile& /*ini*/){
                this->wait_ntlm_password = false;
                f(!this->set_password("x"));
            }));

            return Ntlm_SecurityFunctionTable::PasswordCallback::Wait;
        }, true)
    {
        this->credssp_server.credssp_server_authenticate_init();
        this->run();
    }

    bool run()
    {
        rdpCredsspServer::State st = rdpCredsspServer::State::Cont;
        while (this->buf.next_credssp() && rdpCredsspServer::State::Cont == st) {
            InStream in_stream(this->buf.current_pdu_buffer());
            st = this->credssp_server.credssp_server_authenticate_next(in_stream);
        }
        return check_continue(st);
    }

private:
    bool set_password(char const* password)
    {
        assert(this->identity);
        this->identity->SetPasswordFromUtf8(byte_ptr_cast(password));
        InStream in_stream;
        return check_continue(
                this->credssp_server.credssp_server_authenticate_next(in_stream))
            && this->run();
    }

    static bool check_continue(rdpCredsspServer::State st)
    {
        switch (st) {
        case rdpCredsspServer::State::Err: throw Error(ERR_NLA_AUTHENTICATION_FAILED);
        case rdpCredsspServer::State::Cont: return true;
        case rdpCredsspServer::State::Finish: return false;
        }

        REDEMPTION_UNREACHABLE();
    }

    SessionReactor& session_reactor;
    SessionReactor::SesmanEventPtr sesman_credssp_server;
    bool& wait_ntlm_password;
    TpduBuffer& buf;
    rdpCredsspServer credssp_server;
    // TODO should be front parameter
    TimeSystem timeobj;
    // TODO should be front parameter
    std::string extra_message;
    SEC_WINNT_AUTH_IDENTITY* identity = nullptr;
};


void CredsspFrontServer::start(
    SessionReactor& session_reactor, Transport& trans, Random& rand, TpduBuffer& buf,
    Translation::language_t language, bool& wait_ntlm_password, std::function<void(bool)> f)
{
    this->d = std::make_unique<D>(
        session_reactor, trans, rand, buf, language, wait_ntlm_password, std::move(f));
}

bool CredsspFrontServer::run()
{
    if (this->d->run()) {
        return true;
    }
    this->d.reset();
    return false;
}

CredsspFrontServer::CredsspFrontServer() = default;
CredsspFrontServer::~CredsspFrontServer() = default;
