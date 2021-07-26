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
    Copyright (C) Wallix 2021
    Author(s): Florent Plard
*/

#pragma once

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "pkcs11-helper-1.0/pkcs11.h"

#include "scard/scard_utils.hpp"
#include "utils/sugar/bytes_view.hpp"


///////////////////////////////////////////////////////////////////////////////


///
typedef std::vector<uint8_t> byte_array;

///
class scard_pkcs11_exception : public std::runtime_error
{
public:
    ///
    scard_pkcs11_exception(const char *message)
        :
        std::runtime_error(message),
        _rv(0xFFFFFFFF)
    {
    }

    ///
    scard_pkcs11_exception(const char *message, CK_RV return_value)
        :
        std::runtime_error(message),
        _rv(return_value)
    {
    }

    ///
    scard_pkcs11_exception(const std::string& message)
        :
        std::runtime_error(message),
        _rv(0xFFFFFFFF)
    {
    }

    ///
    scard_pkcs11_exception(const std::string& message, CK_RV return_value)
        :
        std::runtime_error(message),
        _rv(return_value)
    {
    }

private:
    /// PKCS#11 API return value.
    const CK_RV _rv;
};

///
typedef CK_ULONG scard_pkcs11_slot_id;

///
class scard_pkcs11_slot
{
public:
    ///
    scard_pkcs11_slot(scard_pkcs11_slot_id id)
        :
        _id(id),
        _login_required(false)
    {
    }

    ///
    inline scard_pkcs11_slot_id get_id() const
    {
        return _id;
    }

    ///
    inline const std::string & get_description() const
    {
        return _description;
    }

    ///
    void set_description(bytes_view value)
    {
        const auto trimmed_value = trim(std::string(
            value.begin(), value.end()));
        
        _description = trimmed_value;
    }

    ///
    inline const std::string & get_token_label() const
    {
        return _token_label;
    }

    ///
    inline bool requires_login() const
    {
        return _login_required;
    }

    ///
    void set_token_label(bytes_view value)
    {
        const auto trimmed_value = trim(std::string(
            value.begin(), value.end()));
        
        _token_label.assign(trimmed_value.begin(), trimmed_value.end());
    }

    ///
    inline const std::string & get_token_serial() const
    {
        return _token_serial;
    }

    ///
    void set_token_serial(bytes_view value)
    {
        const auto trimmed_value = trim(std::string(
            value.begin(), value.end()));
        
        _token_serial.assign(trimmed_value.begin(), trimmed_value.end());
    }

    ///
    void set_flags(bool login_required)
    {
        _login_required = login_required;
    }

private:
    ///
    const scard_pkcs11_slot_id _id;

    ///
    std::string _description;

    ///
    std::string _token_label;

    ///
    std::string _token_serial;

    ///
    bool _login_required;
};

///
typedef std::vector<scard_pkcs11_slot> scard_pkcs11_slot_list;

///
typedef CK_SESSION_HANDLE scard_pkcs11_session_handle;


///
class scard_pkcs11_session
{
public:
    ///
    scard_pkcs11_session(scard_pkcs11_session_handle handle)
        :
        _handle(handle)
    {
    }

    ///
    inline scard_pkcs11_session_handle get_handle() const
    {
        return _handle;
    }

private:
    ///
    const scard_pkcs11_session_handle _handle;
};

///
typedef CK_ATTRIBUTE_TYPE scard_pkcs11_attribute_type;

///
class scard_pkcs11_attribute
{
public:
    ///
    scard_pkcs11_attribute(scard_pkcs11_attribute_type type,
        std::size_t size = 0)
        :
        _type(type),
        _value(size)
    {
    }

    ///
    scard_pkcs11_attribute(scard_pkcs11_attribute_type type,
        const void * value, std::size_t size)
        :
        _type(type),
        _value(size)
    {
        if (value && size)
        {
            std::memcpy(_value.data(), value, size);
        }
    }

    ///
    scard_pkcs11_attribute(scard_pkcs11_attribute_type type,
        bytes_view value)
        :
        _type(type),
        _value(value.begin(), value.end())
    {
    }

    ///
    scard_pkcs11_attribute(scard_pkcs11_attribute_type type,
        byte_array value)
        :
        _type(type),
        _value(std::move(value))
    {
    }

    ///
    inline const void * data() const
    {
        return _value.data();
    }

    ///
    inline void * data()
    {
        return _value.data();
    }

    ///
    inline scard_pkcs11_attribute_type get_type() const
    {
        return _type;
    }

    ///
    inline bytes_view get_value() const
    {
        return _value;
    }

    ///
    void set_value(const void *value)
    {
        if (value)
        {
            std::memcpy(_value.data(), value, _value.size());
        }
        else
        {
            _value.clear();
        }
    }

    ///
    void set_value(const void *value, std::size_t size)
    {
        if (value)
        {
            _value.resize(size);
            std::memcpy(_value.data(), value, size);
        }
        else
        {
            _value.clear();
        }
    }

    ///
    void set_value(bytes_view value)
    {
        _value.assign(value.begin(), value.end());
    }

    ///
    void set_value(byte_array value)
    {
        _value = std::move(value);
    }

    ///
    inline std::size_t get_size() const
    {
        return _value.size();
    }

    ///
    void set_size(std::size_t size)
    {
        _value.resize(size);
    }

private:
    ///
    const scard_pkcs11_attribute_type _type;

    ///
    byte_array _value;
};

///
typedef std::vector<
    scard_pkcs11_attribute_type
> scard_pkcs11_attribute_type_list;

///
typedef std::vector<
    scard_pkcs11_attribute
> scard_pkcs11_attribute_list;

///
typedef CK_OBJECT_HANDLE scard_pkcs11_object_handle;

///
class scard_pkcs11_object
{
public:
    ///
    scard_pkcs11_object(scard_pkcs11_object_handle handle)
        :
        _handle(handle)
    {
    }

    ///
    inline scard_pkcs11_object_handle get_handle() const
    {
        return _handle;
    }

    ///
    scard_pkcs11_attribute & get_attribute(scard_pkcs11_attribute_type attribute_type)
    {
        auto it = _attributes.find(attribute_type);
        if (it == _attributes.end())
        {
            throw scard_pkcs11_exception("No such attribute.");
        }

        return it->second;
    }

    ///
    scard_pkcs11_attribute & set_attribute(scard_pkcs11_attribute_type attribute_type,
        std::size_t attribute_size)
    {
        auto it = _attributes.find(attribute_type);
        if (it == _attributes.end())
        {
            it = _attributes.emplace(attribute_type, scard_pkcs11_attribute(
                attribute_type, attribute_size
            )).first;
        }
        else
        {
            it->second.set_size(attribute_size);
        }

        return it->second;
    }

    ///
    scard_pkcs11_attribute & set_attribute(scard_pkcs11_attribute_type attribute_type,
        void * attribute_value, std::size_t attribute_size)
    {
        auto it = _attributes.find(attribute_type);
        if (it == _attributes.end())
        {
            it = _attributes.emplace(attribute_type, scard_pkcs11_attribute(
                attribute_type, attribute_value, attribute_size
            )).first;
        }
        else
        {
            it->second.set_value(attribute_value, attribute_size);
        }

        return it->second;
    }

    ///
    scard_pkcs11_attribute & set_attribute(scard_pkcs11_attribute_type attribute_type,
        bytes_view attribute_value)
    {
        auto it = _attributes.find(attribute_type);
        if (it == _attributes.end())
        {
            it = _attributes.emplace(attribute_type, scard_pkcs11_attribute(
                attribute_type, attribute_value
            )).first;
        }
        else
        {
            it->second.set_value(attribute_value);
        }

        return it->second;
    }

    ///
   scard_pkcs11_attribute & set_attribute(scard_pkcs11_attribute_type attribute_type,
        const byte_array &attribute_value)
    {
        auto it = _attributes.find(attribute_type);
        if (it == _attributes.end())
        {
            it = _attributes.emplace(attribute_type, scard_pkcs11_attribute(
                attribute_type, std::move(attribute_value)
            )).first;
        }
        else
        {
            it->second.set_value(attribute_value);
        }

        return it->second;
    }

private:
    ///
    const scard_pkcs11_object_handle _handle;

    ///
    std::unordered_map<
        scard_pkcs11_attribute_type, scard_pkcs11_attribute
    > _attributes;
};

///
typedef std::vector<
    scard_pkcs11_object
> scard_pkcs11_object_list;

/// These ones appear to be missing in pkcs11-helper library
typedef unsigned long CK_CERTIFICATE_CATEGORY;
#define CK_CAT_UNSPECIFIED  0
#define CK_CAT_USER         1
#define CK_CAT_AUTHORITY    2
#define CK_CAT_OTHER        3

///
enum scard_pkcs11_certificate_type
{
    certificate_type_unknown = 0,
    certificate_type_vendor_defined,
    certificate_type_wtls,
    certificate_type_x509,
    certificate_type_x509_attributes
};

///
enum scard_pkcs11_key_type
{
    key_type_unknown = 0,
    key_type_rsa,
    key_type_dsa,
    key_type_dh,
    key_type_ec,
    key_type_x9_42_dh,
    key_type_kea,
    key_type_generic_secret,
    key_type_rc2,
    key_type_rc4,
    key_type_des,
    key_type_des2,
    key_type_des3,
    key_type_cast,
    key_type_cast3,
    key_type_cast128,
    key_type_rc5,
    key_type_idea,
    key_type_skipjack,
    key_type_baton,
    key_type_juniper,
    key_type_cdmf,
    key_type_aes,
    key_type_blowfish,
    key_type_twofish,
    key_type_vendor_defined
};

///
enum scard_pkcs11_certificate_category
{
    certificate_category_unknown = -1,
    certificate_category_unspecified,
    certificate_category_user,
    certificate_category_authority,
    certificate_category_other
};

///
class scard_pkcs11_certificate
{
public:
    ///
    scard_pkcs11_certificate(bytes_view id, bytes_view label,
        bytes_view type, bytes_view key_type, bytes_view category,
        bytes_view issuer, bytes_view subject,
        bytes_view data)
        : 
        _id(id.begin(), id.end()),
        _label(label.begin(), label.end()),
        _type(certificate_type_unknown),
        _key_type(key_type_unknown),
        _category(certificate_category_unknown),
        _issuer(issuer.begin(), issuer.end()),
        _subject(subject.begin(), subject.end()),
        _data(data.begin(), data.end())
    {
        if (type.size() == sizeof(CK_CERTIFICATE_TYPE))
        {
            _type = type_from_raw(
                *reinterpret_cast<const CK_CERTIFICATE_TYPE *>(type.data())
            );
        }

        if (category.size() == sizeof(CK_CERTIFICATE_CATEGORY))
        {
            _category = category_from_raw(
                *reinterpret_cast<const CK_CERTIFICATE_CATEGORY *>(category.data())
            );
        }

        if (key_type.size() == sizeof(CK_KEY_TYPE))
        {
            _key_type = key_type_from_raw(
                *reinterpret_cast<const CK_KEY_TYPE *>(key_type.data())
            );
        }
    }

    ///
    inline bytes_view get_id() const
    {
        return _id;
    }

    ///
    std::string get_id_as_string(std::string_view byte_separator = "") const
    {
        return bytes_to_hex_string(_id, byte_separator);
    }

    ///
    inline const std::string & get_label() const
    {
        return _label;
    }

    ///
    inline scard_pkcs11_certificate_type get_type() const
    {
        return _type;
    }

    ///
    std::string get_type_as_string() const
    {
        return type_to_string(_type);
    }

    ///
    inline scard_pkcs11_key_type get_key_type() const
    {
        return _key_type;
    }

    ///
    std::string get_key_type_as_string() const
    {
        return key_type_to_string(_key_type);
    }

    ///
    inline scard_pkcs11_certificate_category get_category() const
    {
        return _category;
    }

    ///
    std::string get_category_as_string() const
    {
        return category_to_string(_category);
    }

    ///
    inline bytes_view get_issuer() const
    {
        return _issuer;
    }

    ///
    inline bytes_view get_subject() const
    {
        return _subject;
    }

    ///
    inline bytes_view get_data() const
    {
        return _data;
    }

private:
    ///
    const byte_array _id;

    ///
    const std::string _label;

    ///
    scard_pkcs11_certificate_type _type;

    ///
    scard_pkcs11_key_type _key_type;

    ///
    scard_pkcs11_certificate_category _category;

    ///
    const byte_array _issuer;

    ///
    const byte_array _subject;

    ///
    const byte_array _data;

    ///
    static scard_pkcs11_certificate_type type_from_raw(CK_CERTIFICATE_TYPE raw_type)
    {
        switch (raw_type)
        {
            case CKC_VENDOR_DEFINED:
                return certificate_type_vendor_defined;
            case CKC_WTLS:
                return certificate_type_wtls;
            case CKC_X_509:
                return certificate_type_x509;
            case CKC_X_509_ATTR_CERT:
                return certificate_type_x509_attributes;
            default:
                throw scard_pkcs11_exception("Unexpected certificate type.");
        }
    }

    ///
    static std::string type_to_string(scard_pkcs11_certificate_type type)
    {
        switch (type)
        {
            case certificate_type_unknown:
                return "unknown";
            case certificate_type_vendor_defined:
                return "vendor defined";
            case certificate_type_wtls:
                return "WTLS";
            case certificate_type_x509:
                return "X.509";
            case certificate_type_x509_attributes:
                return "X.509 attributes";
            default:
                throw scard_pkcs11_exception("Unexpected certificate type.");
        }
    }

    ///
    static scard_pkcs11_key_type key_type_from_raw(CK_KEY_TYPE raw_key_type)
    {
        switch (raw_key_type)
        {
            case CKK_RSA:
                return key_type_rsa;
            case CKK_DSA:
                return key_type_dsa;
            case CKK_DH:
                return key_type_dh;
            case CKK_EC:
                return key_type_ec;
            case CKK_X9_42_DH:
                return key_type_x9_42_dh;
            case CKK_KEA:
                return key_type_kea;
            case CKK_GENERIC_SECRET:
                return key_type_generic_secret;
            case CKK_RC2:
                return key_type_rc2;
            case CKK_RC4:
                return key_type_rc4;
            case CKK_DES:
                return key_type_des;
            case CKK_DES2:
                return key_type_des2;
            case CKK_DES3:
                return key_type_des3;
            case CKK_CAST:
                return key_type_cast;
            case CKK_CAST3:
                return key_type_cast3;
            case CKK_CAST128:
                return key_type_cast128;
            case CKK_RC5:
                return key_type_rc5;
            case CKK_IDEA:
                return key_type_idea;
            case CKK_SKIPJACK:
                return key_type_skipjack;
            case CKK_BATON:
                return key_type_baton;
            case CKK_JUNIPER:
                return key_type_juniper;
            case CKK_CDMF:
                return key_type_cdmf;
            case CKK_AES:
                return key_type_aes;
            case CKK_BLOWFISH:
                return key_type_blowfish;
            case CKK_TWOFISH:
                return key_type_twofish;
            case CKK_VENDOR_DEFINED:
                return key_type_vendor_defined;
            default:
                throw scard_pkcs11_exception("Unexpected certificate key type.");
        }
    }

    ///
    static std::string key_type_to_string(scard_pkcs11_key_type key_type)
    {
        switch (key_type)
        {
            case key_type_unknown:
                return "unknown";
            case key_type_rsa:
                return "RSA";
            case key_type_dsa:
                return "DSA";
            case key_type_dh:
                return "DH";
            case key_type_ec:
                return "EC";
            case key_type_x9_42_dh:
                return "X9.42 DH";
            case key_type_kea:
                return "KEA";
            case key_type_generic_secret:
                return "generic secret";
            case key_type_rc2:
                return "RC2";
            case key_type_rc4:
                return "RC4";
            case key_type_des:
                return "DES";
            case key_type_des2:
                return "DES2";
            case key_type_des3:
                return "DES3";
            case key_type_cast:
                return "CAST";
            case key_type_cast3:
                return "CAST3";
            case key_type_cast128:
                return "CAST128";
            case key_type_rc5:
                return "RC5";
            case key_type_idea:
                return "IDEA";
            case key_type_skipjack:
                return "SkipJack";
            case key_type_baton:
                return "BATON";
            case key_type_juniper:
                return "Juniper";
            case key_type_cdmf:
                return "CDMF";
            case key_type_aes:
                return "AES";
            case key_type_blowfish:
                return "BlowFish";
            case key_type_twofish:
                return "TwoFish";
            case key_type_vendor_defined:
                return "vendor defined";
            default:
                throw scard_pkcs11_exception("Unexpected certificate key type.");
        }
    }

    ///
    static scard_pkcs11_certificate_category category_from_raw(CK_CERTIFICATE_CATEGORY raw_category)
    {
        switch (raw_category)
        {
            case CK_CAT_UNSPECIFIED:
                return certificate_category_unspecified;
            case CK_CAT_USER:
                return certificate_category_user;
            case CK_CAT_AUTHORITY:
                return certificate_category_authority;
            case CK_CAT_OTHER:
                return certificate_category_other;
            default:
                throw scard_pkcs11_exception("Unexpected certificate category.");
        }
    }

    ///
    static std::string category_to_string(scard_pkcs11_certificate_category category)
    {
        switch (category)
        {
            case certificate_category_authority:
                return "authority";
            case certificate_category_other:
                return "other entity";
            case certificate_category_unknown:
                return "unknown";
            case certificate_category_unspecified:
                return "unspecified";
            case certificate_category_user:
                return "token user";
            default:
                throw scard_pkcs11_exception("Unexpected certificate category.");
        }
    }
};

///
typedef std::vector<
    scard_pkcs11_certificate
> scard_pkcs11_certificate_list;

///
typedef CK_USER_TYPE scard_pkcs11_user_type;