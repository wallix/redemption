/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Author(s): Christophe Grosjean

   Module: authhook.cpp
   Description : sample post-login authentication data processing

   This code is not actually used in production code, it's just a
   minimal tcp server sample whose purpose is to test the protocol.
   It shows how the post-processing of authentication data works.

   If no such hook is provided redemption is supposed to works seamlessly
   on it's own with consistant behavior.

*/

#ifndef _REDEMPTION_ACL_DICO_HPP_
#define _REDEMPTION_ACL_DICO_HPP_

#include <string.h>
#include <map>

#include "log.hpp"
#include "config.hpp"

#include "keywords.hpp"

enum {
    DICOBUFSIZE = 8192,
};

enum t_ask {
    V_DEFAULT,
    V_UNDEFINED,
    V_NEEDED,
    V_ASKED,
    V_DEFINED
};

struct KeywordValue {
    t_ask ask;
    char value[DICOBUFSIZE];
};


struct KeyComp {
  bool operator() (const char * const & lhs, const char * const & rhs) const
  {return strcmp(lhs,rhs) < 0;}
};

class Dico {
    public:
    typedef std::map<const char *, KeywordValue *, KeyComp> t_kmap;
    t_kmap map;

    TODO(" get should return a non modifiable string (const char * const) and return "" if ASK to avoid showing it")
    char * get(const char * key) {
        TODO(" be be raising an exception would be better")
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            LOG(LOG_DEBUG, "'%s' Not found in context\n", key);
            return NULL;
        }
        char * v = it->second->value;
        if (v[0] == '!'){
            return v + 1;
        }
        else if ((v[0] == 'A'||v[0] == 'a')
              && (v[1] == 'S'||v[1] == 's')
              && (v[2] == 'K'||v[2] == 'k')
              && (v[3] == 0)){
            return v+3;
        }
        else {
            return v;
        }
    }

    bool get_bool(const char * key) {
        TODO(" may be raising an exception would be better")
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            return false;
        }
        const char * found = it->second->value+1;
        TODO(" define a set of keywords and look if found is in this set")
        return ((found[0] == 0)
        || (strncasecmp("1", found, 2) == 0)
        || (strncasecmp("on", found, 3) == 0)
        || (strncasecmp("yes", found, 4) == 0)
        || (strncasecmp("true", found, 5) == 0));
    }

    bool is_asked(const char * key) {
        TODO(" may be raising an exception would be better")
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            return false; // if key not found we say value if greater
        }
        const char * found = it->second->value;
        bool res = (strncasecmp("ask", found, 3) == 0);
        return res;
    }

    bool cmp(const char * key, const char * value) {
        TODO(" may be raising an exception would be better")
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            return false; // if key not found we say value if greater
        }
        const char * found = it->second->value;

        return 0 == strncmp((*found=='!')?found+1:found, value, DICOBUFSIZE-1);
    }

    void cpy(const char * key, int value) {
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            return;
        }
        char * found = it->second->value;
        found[0] = '!';
        found++;
        sprintf(found, "%d", value);
    }

    void cpy(const char * key, const char * value) {
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            return;
        }
        char * found = it->second->value;
        found[0] = '!';
//        LOG(LOG_INFO, "copying %s to %s", value, key);
        size_t len = strlen(value);
        if (len >= DICOBUFSIZE-2){
            len = DICOBUFSIZE-2;
            found[DICOBUFSIZE-1] = 0;
        }
        strncpy(found+1, value, len);
        found[len+1] = 0;
    }

    void ask(const char * key) {
        TODO(" may be raising an exception would be better")
        const t_kmap::iterator & it = this->map.find(key);
        if (it == this->map.end()){
            return;
        }
        char * found = it->second->value;
        memcpy(found, "ASK", 4);
    }

    Dico(ProtocolKeyword * KeywordsDefinitions, unsigned nbkeywords)
    {
        TODO(" check keywords are not duplicated  if it happens it will cause memory leak")
        for (unsigned i = 0; i < nbkeywords; i++){
                KeywordValue * item = new KeywordValue();
                item->ask = V_DEFAULT;
                strncpy(item->value, KeywordsDefinitions[i].default_value, DICOBUFSIZE-1);
                this->map[KeywordsDefinitions[i].keyword] = item;
//                LOG(LOG_INFO, "adding keyword to context: %s at %p\n", KeywordsDefinitions[i].keyword, item);
        }
//        LOG(LOG_INFO, "allocated %d entries\n", nbkeywords);
    }

    ~Dico()
    {
        t_kmap::iterator it;
        int counter = 0;
        for (it = this->map.begin() ; it != this->map.end() ; ++it){
//            LOG(LOG_INFO, "freeing keyword %s at %p\n", it->first, it->second);
            delete it->second;
            counter++;
        }
//        LOG(LOG_INFO, "freed %d entries\n", counter);
//        LOG(LOG_INFO, "Dico deleted");

    }

};

#endif
