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
    Copyright (C) Wallix 2020
    Author(s): 
*/

#include <memory>
#include <string>
#include <vector>

#include <openssl/x509v3.h>

#include "scard/scard_x509.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_x509_name_list extract_subject_alt_names(bytes_view certificate_data)
{
    auto certificate_data_ptr = certificate_data.data();

    // parse X.509 certificate
    X509 *certificate = d2i_X509(nullptr, &certificate_data_ptr, certificate_data.size());
    if (!certificate)
    {
        throw scard_x509_exception("Failed to parse X.509 certificate.");
    }

    std::unique_ptr<X509, decltype(&X509_free)> certificate_ptr(
        certificate, &X509_free);
    
    // find subjectAltName extension
    STACK_OF(GENERAL_NAME) *names = static_cast<STACK_OF(GENERAL_NAME) *>(
        X509_get_ext_d2i(certificate, NID_subject_alt_name, nullptr, nullptr));
    if (!names)
    {
        throw scard_x509_exception("Could not find subjectAltName extension in certificate.");
    }

    std::unique_ptr<STACK_OF(GENERAL_NAME), decltype(&GENERAL_NAMES_free)> names_ptr(
        names, &GENERAL_NAMES_free);

    // count distinct names within the extension
    const auto n_names = sk_GENERAL_NAME_num(names);

    scard_x509_name_list x509_names;

    for (int i = 0; i < n_names; ++i)
    {
        // retrieve the current name value
        const auto *name = sk_GENERAL_NAME_value(names, i);
        if (!name)
        {
            continue;
        }

        // parse the retrieved value
        switch (name->type)
        {
            case GEN_DNS:
            {
                // decode value
                const auto *name_data = reinterpret_cast<const char *>(
                    ASN1_STRING_get0_data(name->d.dNSName));
                const auto name_length = ASN1_STRING_length(name->d.dNSName);

                x509_names.push_back(
                    { scard_x509_name_type::dns, std::string(name_data, name_length) }
                );
            } break;

            case GEN_EMAIL:
            {
                // decode value
                const auto *name_data = reinterpret_cast<const char *>(
                    ASN1_STRING_get0_data(name->d.rfc822Name));
                const auto name_length = ASN1_STRING_length(name->d.rfc822Name);

                x509_names.push_back(
                    { scard_x509_name_type::email, std::string(name_data, name_length) }
                );
            } break;

            case GEN_OTHERNAME:
            {
                /*
                std::vector<char> oid;

                // get the text representation of the OID
                // note: two passes are needed to do this; the first pass
                // allows to obtain the OID length and the second pass
                // allows to retrieve the whole OID value.
                {
                    oid.resize(1);

                    // get the OID length
                    const auto length = OBJ_obj2txt(oid.data(), oid.size(),
                        name->d.otherName->type_id, 1) + 1;

                    oid.resize(length);

                    // get the OID value
                    OBJ_obj2txt(oid.data(), length,
                        name->d.otherName->type_id, 1);
                }
                */

                // determine the object type
                auto nid = OBJ_obj2nid(name->d.otherName->type_id);

                // parse the object value according to its type
                switch (nid)
                {
                    case NID_ms_upn:
                    {
                        // decode ASN.1 string value
                        const auto *name_data = reinterpret_cast<const char *>(
                            ASN1_STRING_get0_data(name->d.otherName->value->value.asn1_string));
                        const auto name_length = ASN1_STRING_length(
                            name->d.otherName->value->value.asn1_string);
                        
                        x509_names.push_back(
                            { scard_x509_name_type::ms_upn, std::string(name_data, name_length) }
                        );
                    } break;

                    default:
                    {
                        // TODO Output raw data?
                    } break;
                }
            } break;

            case GEN_URI:
            {
                // decode value
                const auto *name_data = reinterpret_cast<const char *>(
                    ASN1_STRING_get0_data(name->d.uniformResourceIdentifier));
                const auto name_length = ASN1_STRING_length(name->d.uniformResourceIdentifier);

                x509_names.push_back(
                    { scard_x509_name_type::uri, std::string(name_data, name_length) }
                );
            } break;

            default:
                break;
        }
    }

    return x509_names;
}

std::string name_type_to_string(const scard_x509_name_type &type)
{
    switch (type)
    {
        case scard_x509_name_type::dns:         return "DNS";
        case scard_x509_name_type::email:       return "e-mail";
        case scard_x509_name_type::ms_upn:      return "Microsoft UPN";
        case scard_x509_name_type::uri:         return "URI";
        case scard_x509_name_type::unknown:     return "unknown";
        default:                                throw std::logic_error("Unknown X.509 name type.");
    }
}