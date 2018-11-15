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
   Copyright (C) Wallix 2017
   Author(s): Christophe Grosjean, Raphael ZHOU
*/

#pragma once

#include "core/error.hpp"
#include "utils/log.hpp"

#include <map>
#include <utility>

template <typename T, unsigned int next_usable,
    unsigned int first_invariable, unsigned int last_invariable>
class IDManager
{
private:
    // PERF very slow
    std::map<T, T>                  src_to_dest;
    std::map<T, std::pair<T, bool>> dest_to_src;

    T next_usable_dest_id = static_cast<T>(next_usable);

    unsigned int verbose;

public:
    IDManager(unsigned verbose_ = 0) : verbose(verbose_) {}

    T get_dest_id(T src_id) const {
        auto iter = this->src_to_dest.find(src_id);
        if (this->src_to_dest.end() != iter) {
            return iter->second;
        }

        LOG(LOG_ERR, "IDManager::get_dest_id(...): Unknown source Id! Id=0x%X",
            src_id);
        throw Error(ERR_UNEXPECTED);
    }

    T get_dest_id_ex(T src_id) {
        auto iter = this->src_to_dest.find(src_id);
        if (this->src_to_dest.end() != iter) {
            return iter->second;
        }

        return this->map_src_id(src_id);
    }

    T get_src_id(T dest_id) const {
        auto iter = this->dest_to_src.find(dest_id);
        if (this->dest_to_src.end() != iter) {
            if (iter->second.second) {
                return iter->second.first;
            }

            LOG(LOG_ERR,
                "IDManager::get_src_id(...): Destination only Id! Id=0x%X",
                dest_id);
            throw Error(ERR_UNEXPECTED);
        }

        LOG(LOG_ERR,
            "IDManager::get_src_id(...): Unknown destination Id! Id=0x%X",
            dest_id);
        throw Error(ERR_UNEXPECTED);
    }

    bool is_dest_only_id(T dest_id) const {
        auto iter = this->dest_to_src.find(dest_id);
        if (this->dest_to_src.end() != iter) {
            return !iter->second.second;
        }

        LOG(LOG_ERR,
            "IDManager::get_src_id(...): Unknown destination Id! Id=0x%X",
            dest_id);
        throw Error(ERR_UNEXPECTED);
    }

private:
    T map_src_id(T src_id) {
        assert(this->src_to_dest.end() == this->src_to_dest.find(src_id));

        T dest_id;

        if (((src_id >= first_invariable) && (src_id <= last_invariable)) ||
            (this->dest_to_src.end() == this->dest_to_src.find(src_id))) {
            dest_id = src_id;

            if (dest_id == this->next_usable_dest_id) {
                this->update_next_usable_dest_id();
            }
        }
        else {
            dest_id = this->next_usable_dest_id;

            this->update_next_usable_dest_id();
        }

        this->dest_to_src[dest_id] = {src_id, true};
        this->src_to_dest[src_id]  = dest_id;

        if (this->verbose) {
            LOG(LOG_INFO,
                "IDManager::map_src_id(...): src_id=0x%X dest_id=0x%X",
                src_id, dest_id);
        }

        return dest_id;
    }

public:
    T reg_dest_only_id() {
        const T dest_id = this->next_usable_dest_id;

        this->update_next_usable_dest_id();

        this->dest_to_src[dest_id] = {0, false};

        if (this->verbose) {
            LOG(LOG_INFO, "IDManager::reg_dest_only_id(...): dest_id=0x%X",
                dest_id);
        }

        return dest_id;
    }

    void unreg_dest_only_id(T dest_id) {
        auto iter = this->dest_to_src.find(dest_id);
        if (this->dest_to_src.end() != iter) {
            if (!iter->second.second) {
                this->dest_to_src.erase(iter);

                if (this->verbose) {
                    LOG(LOG_INFO,
                        "IDManager::unreg_dest_only_id(...): dest_id=0x%X",
                        dest_id);
                }

                return;
            }

            LOG(LOG_ERR,
                "IDManager::unreg_dest_only_id(...): "
                    "Not a destination only Id! id=0x%X",
                dest_id);
            throw Error(ERR_UNEXPECTED);
        }

        LOG(LOG_ERR,
            "IDManager::unreg_dest_only_id(...): "
                "Unknown destination Id! id=0x%X",
            dest_id);
        throw Error(ERR_UNEXPECTED);
    }

    void unreg_src_id(T src_id) {
        auto iter = this->src_to_dest.find(src_id);
        if (this->src_to_dest.end() != iter) {
            this->dest_to_src.erase(iter->second);

            if (this->verbose) {
                LOG(LOG_INFO,
                    "IDManager::unreg_src_id(...): dest_id=0x%X", iter->second);
            }

            this->src_to_dest.erase(iter);

            if (this->verbose) {
                LOG(LOG_INFO,
                    "IDManager::unreg_src_id(...): src_id=0x%X", src_id);
            }

            return;
        }

        LOG(LOG_ERR,
            "IDManager::unreg_dest_only_id(...): Unknown source Id! id=0x%X",
            src_id);
        throw Error(ERR_UNEXPECTED);
    }

private:
    void update_next_usable_dest_id() {
        do {
            this->next_usable_dest_id++;
        }
        while (
                ((this->next_usable_dest_id >= first_invariable) &&
                 (this->next_usable_dest_id <= last_invariable)) ||
                (this->dest_to_src.end() !=
                 this->dest_to_src.find(this->next_usable_dest_id))
            );
    }
};
