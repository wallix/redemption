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

#include "get_printable_password.hpp"
#include "cfgloader.hpp"
// BASE64 TRY
// #include "base64.hpp"

#include "parser.hpp"
#include <set>
#include <map>
#include <string>

struct FieldObserver : public ConfigurationHolder {
    /******************************************************
     * BaseField is an abstract class which carries:
     * - some flags indicating some meta state of the field.
     * - a pointer to a Inifile it is attached to.
     * - an authid_t if it is attached to a Inifile.
     ******************************************************
     */

    using authid_t = unsigned;
    static constexpr authid_t AUTHID_UNKNOWN = 0;

    class BaseField {
        BaseField(const BaseField &) = delete;
        BaseField & operator = (const BaseField &) = delete;

    protected:
        mutable bool      asked;         // the value is asked in the context
        mutable bool      modified;      // the value has been modified since last use
        FieldObserver *   ini;           // Inifile to which the field is attached
        authid_t          authid;        // Auth Id of the field in the Inifile
        BaseField()
            : asked(false)
            , modified(true)
            , ini(nullptr)
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
        void modify_from_acl() {
            if (this->ini)
                this->ini->notify_from_acl();
        }
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
        //inline void unask() {
        //    this->asked = false;
        //}

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

        authid_t get_authid() {
            return this->authid;
        }


        virtual void set_from_cstr(const char * cstr) = 0;
        // set from acl is specified to not change the modified flag
        virtual void set_from_acl(const char * cstr) = 0;

        virtual const char* get_value() = 0;
    };
    /*************************************
     * Field which contains a String type
     *************************************
     */
    class StringField : public BaseField {
    protected:
        std::string data;
    public:
        StringField() : BaseField()
        {
        }

        void set(std::string const & string) {
            this->set_from_cstr(string.c_str());
        }
        void set_empty() {
            if (!this->data.empty()){
                this->modify();
                this->data.clear();
            }
        }
        void set_from_acl(const char * cstr) override {
            this->modify_from_acl();
            this->data = cstr;
            this->asked = false;
        }
        void set_from_cstr(const char * cstr) override {
            if (this->asked || this->data.compare(cstr)) {
                this->modify();
                this->data = cstr;
            }
            this->asked = false;
        }
        bool is_empty(){
            return this->data.empty();
        }
        const std::string & get() const {
            return this->data;
        }

        const char * get_cstr() const {
            return this->get().c_str();
        }

        const char * get_value() override {
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
        uint32_t data;
        char buff[20];
    public:
        UnsignedField(): BaseField()
                       , data(0) {
        }

        void set(uint32_t that) {
            if (this->data != that || this->asked) {
                this->modify();
                this->data = that;
            }
            this->asked = false;
        }

        void set_from_acl(const char * cstr) override {
            this->modify_from_acl();
            this->data = ulong_from_cstr(cstr);
            this->asked = false;
        }
        void set_from_cstr(const char * cstr) override {
            this->set(ulong_from_cstr(cstr));
        }

        uint32_t get() const {
            return this->data;
        }

        const char * get_value() override {
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
                this->data = that;
            }
            this->asked = false;
        }
        void set_from_acl(const char * cstr) override {
            this->modify_from_acl();
            this->data = _long_from_cstr(cstr);
            this->asked = false;
        }

        void set_from_cstr(const char * cstr) override {
            this->set(_long_from_cstr(cstr));
        }

        signed get() const {
            return this->data;
        }
        const char * get_value() override {
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
                this->data = that;
            }
            this->asked = false;
        }
        void set_from_acl(const char * cstr) override {
            this->modify_from_acl();
            this->data = bool_from_cstr(cstr);
            this->asked = false;
        }
        void set_from_cstr(const char * cstr) override {
            this->set(bool_from_cstr(cstr));
        }

        bool get() const {
            return this->data;
        }
        const char * get_value() override {
            if (this->is_asked()) {
                return "ASK";
            }
            return this->data?"True":"False";
        }
    };


    class SetField {
        std::set<BaseField * > set_field;

    public:
        SetField() = default;

        void insert(BaseField * bfield) {
            this->set_field.insert(bfield);
        }

        void erase(BaseField * bfield) {
            this->set_field.erase(bfield);
        }

        bool empty() const {
            return this->set_field.empty();
        }

        bool find(BaseField * bfield) const {
            return (this->set_field.find(bfield) != this->set_field.end());
        }

        void clear() {
            this->set_field.clear();
        }
        size_t size() const {
            return this->set_field.size();
        }

        template<class Function>
        void foreach(Function funct) const {
            for (auto & x : this->set_field) {
                funct(x);
            }
        }
    };


protected:
    // flag indicating if a Field attached to this inifile has been changed
    bool something_changed;       // value has changed within proxy
    bool new_from_acl;            // value has changed from acl (sesman)

    // list of Field which has been changed
    //std::set< BaseField * > changed_set;
    SetField changed_set;

    // Map associating authid with a Field.
    std::map< authid_t, BaseField *> field_list;



public:
    FieldObserver() = default;
    FieldObserver(FieldObserver const &) = delete;
    FieldObserver & operator = (FieldObserver const &) = delete;

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

    void notify_from_acl() {
        this->new_from_acl = true;
    }
    bool check_from_acl() {
        bool res = this->new_from_acl;
        this->new_from_acl = false;
        return res;
    }

    bool check() const {
        return this->something_changed;
    }

    const SetField & get_changed_set() const {
        return this->changed_set;
    }

    void reset() {
        this->something_changed = false;
        this->changed_set.clear();
    }

    void attach_field(BaseField* field, authid_t authid){
        field_list[authid] = field;
    }
};

#endif
