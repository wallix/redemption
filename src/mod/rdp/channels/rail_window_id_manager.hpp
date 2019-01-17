/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"

#include <map>

class RemoteProgramsWindowIdManager
{
    // TODO very inneficient,  use std::vector/flat_map
    mutable std::map<uint32_t, uint32_t> client_to_server;
    mutable std::map<uint32_t, uint32_t> server_to_client;

    mutable uint32_t next_unused_client_window_id = 0x800000;

public:
    enum : uint32_t {
        INVALID_WINDOW_ID = 0xFFFFFFF0,

        RESERVED_WINDOW_ID_0 = 0x0,
        RESERVED_WINDOW_ID_1 = 0xFFFFFFFE,
        RESERVED_WINDOW_ID_2 = 0xFFFFFFFF
    };

    uint32_t get_client_window_id(uint32_t server_window_id) const {
        auto iter = this->server_to_client.find(server_window_id);
        if (iter != this->server_to_client.end()) {
            return iter->second;
        }

        LOG(LOG_ERR, "RemoteProgramsWindowIdManager::get_client_window_id: Unknown server window. WindowId=0x%X", server_window_id);
        throw Error(ERR_UNEXPECTED);

        return RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;
    }

    uint32_t get_client_window_id_ex(uint32_t server_window_id) const {
        auto iter = this->server_to_client.find(server_window_id);
        if (iter != this->server_to_client.end()) {
            return iter->second;
        }

        return this->map_server_window(server_window_id);
    }

    uint32_t get_server_window_id(uint32_t client_window_id) const {

        auto iter = this->client_to_server.find(client_window_id);
        if (iter != this->client_to_server.end()) {
            return iter->second;
        }

        LOG(LOG_ERR, "RemoteProgramsWindowIdManager::get_server_window_id: Unknown client window. WindowId=0x%X", client_window_id);
        throw Error(ERR_UNEXPECTED);

        return RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;
    }

    bool is_client_only_window(uint32_t window_id) const {
        return (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID ==
            this->get_server_window_id(window_id));
    }

    uint32_t register_client_window() {
        this->increment_next_unused_client_window_id();


        const uint32_t client_window_id = this->next_unused_client_window_id;

        this->increment_next_unused_client_window_id();

        this->client_to_server[client_window_id] = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

        //LOG(LOG_INFO, "RemoteProgramsWindowIdManager::register_client_window: ClientWindowId=0x%X", client_window_id);

        return client_window_id;
    }

private:
    uint32_t map_server_window(uint32_t server_window_id) const {
        assert(this->server_to_client.find(server_window_id) == this->server_to_client.end());

        if ((RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_0 == server_window_id) ||
            (RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_1 == server_window_id) ||
            (RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_2 == server_window_id)) {
            return server_window_id;
        }

        uint32_t client_window_id = RemoteProgramsWindowIdManager::INVALID_WINDOW_ID;

        if (this->client_to_server.find(server_window_id) == this->client_to_server.end()) {
            client_window_id = server_window_id;
        }
        else {
            this->increment_next_unused_client_window_id();

            client_window_id = this->next_unused_client_window_id;

            this->increment_next_unused_client_window_id();
        }

        this->client_to_server[client_window_id] = server_window_id;
        this->server_to_client[server_window_id] = client_window_id;

        LOG(LOG_INFO, "RemoteProgramsWindowIdManager::map_server_window: ServerWindowId=0x%X ClientWindowId=0x%X", server_window_id, client_window_id);

        return client_window_id;
    }

    void increment_next_unused_client_window_id() const {
        while ((RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_0 == this->next_unused_client_window_id) ||
               (RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_1 == this->next_unused_client_window_id) ||
               (RemoteProgramsWindowIdManager::RESERVED_WINDOW_ID_2 == this->next_unused_client_window_id) ||
               (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID    == this->next_unused_client_window_id) ||
               (this->client_to_server.find(this->next_unused_client_window_id) != this->client_to_server.end())) {
            this->next_unused_client_window_id++;
        }
    }

public:
    void unregister_client_only_window(uint32_t window_id) {
        auto iter = this->client_to_server.find(window_id);
        if (iter != this->client_to_server.end()) {
            if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID == iter->second) {
                this->client_to_server.erase(window_id);

                //LOG(LOG_INFO, "RemoteProgramsWindowIdManager::unregister_client_window: ClientWindowId=0x%X", window_id);

                return;
            }

            LOG(LOG_ERR, "RemoteProgramsWindowIdManager::unregister_client_window: Not a client only window. WindowId=0x%X", window_id);
            throw Error(ERR_UNEXPECTED);
        }

        LOG(LOG_ERR, "RemoteProgramsWindowIdManager::unregister_client_window: Unknown client window. WindowId=0x%X", window_id);
        throw Error(ERR_UNEXPECTED);
    }

    void unregister_server_window(uint32_t window_id) {
        auto iter = this->server_to_client.find(window_id);
        if (iter != this->server_to_client.end()) {
            if (RemoteProgramsWindowIdManager::INVALID_WINDOW_ID != iter->second) {
                this->client_to_server.erase(iter->second);

                //LOG(LOG_INFO, "RemoteProgramsWindowIdManager::unregister_server_window: ClientWindowId=0x%X", iter->second);
            }

            this->server_to_client.erase(window_id);

            //LOG(LOG_INFO, "RemoteProgramsWindowIdManager::unregister_server_window: ServerWindowId=0x%X", window_id);

            return;
        }

        LOG(LOG_ERR, "RemoteProgramsWindowIdManager::unregister_server_window: Unknown server window. WindowId=0x%X", window_id);
        throw Error(ERR_UNEXPECTED);
    }
};
