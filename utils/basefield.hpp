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
  Copyright (C) Wallix 2012
  Author(s): Christophe Grosjean, Raphael Zhou, Meng Tan

*/
#ifndef _REDEMPTION_BASE_FIELD_HPP_
#define _REDEMPTION_BASE_FIELD_HPP_

#include "cfgloader.hpp"

struct FieldObserver : public ConfigurationHolder {

    /******************************************************
     * BaseField is an abstract class which carries:
     * - some flags indicating some meta state of the field.
     * - a pointer to a Inifile it is attached to.
     * - an authid_t if it is attached to a Inifile.
     ******************************************************
     */

    class BaseField {
    protected:
        bool              asked;         // the value is asked in the context
        bool              modified;      // the value has been modified since last use
        bool              read;          // the value has been read since last set (modified)
        FieldObserver *   ini;           // Inifile to which the field is attached
        authid_t          authid;        // Auth Id of the field in the Inifile
        BaseField()
            : asked(false)
            , modified(true)
            , read(false)
            , ini(NULL)
            , authid(AUTHID_UNKNOWN)
        {
        }
        virtual ~BaseField(){
            if (this->ini){
                this->ini->remove_field(authid);
            }
        }
        /**********************
         * notify the Inifile that the field has been changed
         */
        void modify() {
            this->modified = true;
            if (this->ini)
                this->ini->notify(this);
        }
        void use_notify() {
            this->modified = false;
            if (this->ini)
                this->ini->use_notify(this);
        }
        inline void unask() {
            this->asked = false;
        }

    public:
        /*******************************
         * link this field to an Inifile
         *******************************
         */
        void attach_ini(FieldObserver * p_ini, authid_t authid = AUTHID_UNKNOWN) {
            this->ini = p_ini;
            if (authid != AUTHID_UNKNOWN
                && this->ini) {
                this->authid = authid;
                this->ini->attach_field(this,authid);
                this->modify();
            }

        }
        void set_to_send_field() {
            if (this->authid != AUTHID_UNKNOWN
                && this->ini) {
                this->ini->to_send_set.insert(this->authid);
            }
        }
        /****************************************
         * Use this field to mark it as modified
         ****************************************
         */
        void use() {
            if (this->modified){
                this->use_notify();
            }
        }

        /*******************************************************
         * Check if the field has been modified (since last use)
         *******************************************************
         */
        bool has_changed() {
            // if (!this->asked)
            //     this->use();
            return this->modified;
        }
        /**************************
         * Set the field as asked.
         **************************
         */
        void ask() {
            if (!this->asked) {
                this->asked = true;
                this->modify();
            }
        }
        /***************************************************
         * Set the field as asked.but not mark it as changed
         ***************************************************
         */
        void ask_from_acl() {
            this->asked = true;
        }
        /******************************
         * Check if the field is asked
         ******************************
         */

        bool is_asked() {
            return this->asked;
        }
        /**************************************************************
         * Check if the field has been read (since last effective set)
         **************************************************************
         */
        bool has_been_read() {
            return this->read;
        }


        authid_t get_authid() {
            return this->authid;
        }


        virtual void set_from_cstr(const char * cstr) = 0;
        // set from acl is specified to not change the modified flag
        virtual void set_from_acl(const char * cstr) = 0;

        virtual const char* get_value() = 0;

        const char* get_serialized(char * buff, size_t size) {
            const char * key = string_from_authid(this->authid);
            int n;
            if (this->is_asked()) {
                n = snprintf(buff, size, "%s\nASK\n",key);
                LOG(LOG_INFO, "sending %s=ASK", key);
            }
            else {
                const char * tmp = this->get_value();
                if ((strncasecmp("password", (char*)key, 8) == 0)
                    ||(strncasecmp("target_password", (char*)key, 15) == 0)){
                    LOG(LOG_INFO, "sending %s=<hidden>", key);
                }
                else {
                    LOG(LOG_INFO, "sending %s=%s", key, tmp);
                }
                n = snprintf(buff, size, "%s\n!%s\n",key,tmp);
            }
            if (n >= (int)size || n < 0) {
                LOG(LOG_ERR, "Sending Data to ACL Error: Buffer overflow,"
                    " should have write %u bytes but buffer size is %u bytes", n, size);
                throw Error(ERR_ACL_MESSAGE_TOO_BIG);
            }
            return buff;
        }
    };
    /*************************************
     * Field which contains a String type
     *************************************
     */
    class StringField : public BaseField {
    protected:
        redemption::string data;
    public:
        StringField() : BaseField()
        {
        }

        void set(redemption::string & string) {
            this->set_from_cstr(string.c_str());
        }
        void set_empty() {
            if (!this->data.is_empty()){
                this->modify();
                this->read = false;
                this->data.empty();
            }
        }
        virtual void set_from_acl(const char * cstr) {
            this->asked = false;
            this->data.copy_c_str(cstr);
        }
        virtual void set_from_cstr(const char * cstr) {
            if (strcmp(this->data.c_str(),cstr) || this->asked) {
                this->modify();
                this->read = false;
                this->data.copy_c_str(cstr);
            }
            this->asked = false;
        }
        bool is_empty(){
            return this->data.is_empty();
        }
        const redemption::string & get() {
            this->read = true;
            return this->data;
        }

        const char * get_cstr() {
            return this->get().c_str();
        }

        virtual const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            return this->get().c_str();
        }

    };
    /************************************************
     * Field which contains an Unsigned Integer type
     * implemented as a 32 bits data
     ************************************************
     */
    class UnsignedField : public BaseField {
    protected:
        uint32_t data;
        char buff[20];
    public:
        UnsignedField(): BaseField()
                       , data(0) {
        }

        void set(uint32_t that) {
            if (this->data != that || this->asked) {
                this->modify();
                this->read = false;
                this->data = that;
            }
            this->asked = false;
        }

        virtual void set_from_acl(const char * cstr) {
            this->asked = false;
            this->data = ulong_from_cstr(cstr);
        }
        virtual void set_from_cstr(const char * cstr) {
            this->set(ulong_from_cstr(cstr));
        }

        void set_from_level_cstr(const char * cstr) {
            this->set(level_from_cstr(cstr));
        }

        void set_from_logtype_cstr(const char * cstr) {
            this->set(logtype_from_cstr(cstr));
        }

        const uint32_t get() {
            this->read = true;
            return this->data;
        }

        virtual const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            snprintf(buff, sizeof(buff), "%u", this->data);
            return buff;
        }
    };
    /*************************************
     * Field which contains a Signed Integer type
     * implementation according to compiler
     *************************************
     */

    class SignedField : public BaseField {
    protected:
        signed data;
        char buff[20];
    public:
        SignedField(): BaseField()
                     , data(0) {
        }

        void set(signed that) {
            if (this->data != that || this->asked) {
                this->modify();
                this->read = false;
                this->data = that;
            }
            this->asked = false;
        }
        virtual void set_from_acl(const char * cstr) {
            this->asked = false;
            this->data = _long_from_cstr(cstr);
        }

        virtual void set_from_cstr(const char * cstr) {
            this->set(_long_from_cstr(cstr));
        }

        const signed get() {
            this->read = true;
            return this->data;
        }
        virtual const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            snprintf(buff, sizeof(buff), "%d", this->data);
            return buff;
        }
    };

    /*************************************
     * Field which contains a Boolean type
     *************************************
     */
    class BoolField : public BaseField {
    protected:
        bool data;
    public:
        BoolField(): BaseField()
                   , data(false) {
        }

        void set(bool that) {
            if (this->data != that || this->asked) {
                this->modify();
                this->read = false;
                this->data = that;
            }
            this->asked = false;
        }
        virtual void set_from_acl(const char * cstr) {
            this->asked = false;
            this->data = bool_from_cstr(cstr);
        }
        virtual void set_from_cstr(const char * cstr) {
            this->set(bool_from_cstr(cstr));
        }

        const bool get() {
            this->read = true;
            return this->data;
        }
        virtual const char * get_value() {
            if (this->is_asked()) {
                return "ASK";
            }
            return this->data?"True":"False";
        }
    };


    class SetField {
        std::set<BaseField * > set_field;

        template<typename Functor>
        struct FuncRef {
            Functor& func;

            FuncRef(Functor& fun)
            : func(fun)
            {}

            void operator()(BaseField * bf) {
                this->func(bf);
            }
        };

    public:
        SetField() {
        }

        void insert(BaseField * bfield) {
            this->set_field.insert(bfield);
        }

        void erase(BaseField * bfield) {
            this->set_field.erase(bfield);
        }

        bool empty(){
            return this->set_field.empty();
        }

        bool find(BaseField * bfield) {
            return (this->set_field.find(bfield) != this->set_field.end());
        }

        void clear() {
            this->set_field.clear();
        }
        size_t size() {
            return this->set_field.size();
        }

        template<class Function>
        void foreach(Function funct) {
            std::for_each(set_field.begin(), set_field.end(), FuncRef<Function>(funct));
        }
    };


protected:
    // flag indicating if a Field attached to this inifile has been changed
    bool something_changed;

    // list of Field which has been changed
    //std::set< BaseField * > changed_set;
    SetField changed_set;

    // Map associating authid with a Field.
    std::map< authid_t, BaseField *> field_list;



public:
    std::set< authid_t > to_send_set;

    const std::map< authid_t, BaseField *>& get_field_list() {
        return this->field_list;
    }
    void remove_field(authid_t authid) {
        this->field_list.erase(authid);
    }

    void notify(BaseField * field) {
        if (this->to_send_set.find(field->get_authid()) != this->to_send_set.end()) {
            this->something_changed = true;
            this->changed_set.insert(field);
        }
    }

    void use_notify(BaseField * field) {
        this->changed_set.erase(field);
        if (this->changed_set.empty()) {
            this->something_changed = false;
        }
    }

    bool check() {
        return this->something_changed;
    }

    // std::set< BaseField * > get_changed_set() {
    SetField & get_changed_set() {
        return this->changed_set;
    }

    void reset() {
        this->something_changed = false;
        changed_set.clear();
    }

    void attach_field(BaseField* field, authid_t authid){
        field_list[authid] = field;
    }
};

#endif
