#include <cinttypes>
#include <string>
#include <string_view>
#include <vector>

#include "scard/scard_pkcs11_enumerator.hpp"
#include "scard/scard_pkcs11_identity.hpp"
#include "scard/scard_pkcs11_module.hpp"
#include "scard/scard_pkcs11_utils.hpp"
#include "scard/scard_utils.hpp"
#include "scard/scard_x509.hpp"
#include "utils/log.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_pkcs11_enumerator::scard_pkcs11_enumerator(
    std::string_view module_path,
    scard_pkcs11_enumerator_handler *handler_ptr)
    :
    _module_path(module_path),
    _handler_ptr(handler_ptr)
{
}

scard_pkcs11_identity_list scard_pkcs11_enumerator::enumerate() const
{
    scard_pkcs11_identity_list identities;

    // notify handler
    if (_handler_ptr)
    {
        _handler_ptr->handle_pkcs11_enumeration_start();
    }

    try
    {
        LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
            "loading PKCS#11 module '%s'", _module_path.c_str());

        // load module
        auto module = scard_pkcs11_module::load(_module_path);

        LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
            "PKCS#11 module loaded.");

        LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
            "listing slots with token...");

        // list slots
        auto slots = module.list_slots(true);

        LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
            "found %" PRIu64 " slot(s) with token.",
            slots.size());

        if (!slots.empty())
        {
            for (auto &slot : slots)
            {
                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "selected slot #%" PRIu64 ".",
                    slot.get_id());
                
                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "retrieving slot/token information...");

                // describe slot
                module.describe_slot(slot);

                // describe token
                module.describe_token(slot);

                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "Slot [ID=%" PRIu64 " description='%s']",
                    slot.get_id(), slot.get_description());
            }

            for (auto &slot : slots)
            {
                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "opening session against slot #%" PRIu64 "...",
                    slot.get_id());

                // open session against the current slot
                const auto &session = module.open_session(slot);

                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "session opened.");

                if (slot.requires_login())
                {
                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "slot requires login.");
                    
                    uint8_t pin[255];

                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "requesting user PIN...");

                    // check whether PIN can be requested
                    if (!_handler_ptr)
                    {
                        LOG(LOG_ERR, "scard_pkcs11_enumerator::enumerate: "
                            "no PIN provider available.");
                        
                        continue;
                    }

                    // request PIN
                    const auto pin_size = _handler_ptr->provide_pkcs11_security_code(
                        writable_bytes_view(pin, sizeof(pin)));
                    
                    // ensure a valid PIN was provided
                    if (!pin_size)
                    {
                        LOG(LOG_ERR, "scard_pkcs11_enumerator::enumerate: "
                            "no PIN provided.");
                        
                        continue;
                    }

                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "logging in as user with PIN...");

                    // login as user with PIN
                    const auto &authenticated = module.login(session,
                        writable_bytes_view(pin, pin_size));
                    if (!authenticated)
                    {
                        LOG(LOG_ERR, "scard_pkcs11_enumerator::enumerate: "
                            "login failed.");
                        
                        continue;
                    }

                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "logged in.");
                }
                else
                {
                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "slot does not require login.");
                }

                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "searching for X.509 signing certificates on-card...");

                // find certificates
                const auto &certificates = module.find_signing_certificates(session);

                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "found %" PRIu64 " suitable certificate(s).",
                    certificates.size());

                for (const auto &certificate : certificates)
                {
                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "Certificate [ID='%s' label='%s' type='%s' category='%s'].",
                        certificate.get_id_as_string(":"), certificate.get_label(),
                        certificate.get_type_as_string(), certificate.get_category_as_string());
                    
                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "searching for X.509 names in certificate...");
                    
                    // extract names from the X.509 subjectAltName certificate extension
                    const auto names = extract_subject_alt_names(certificate.get_data());

                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "found %" PRIu64 " X.509 name(s) in certificate.",
                        names.size());

                    for (const auto &name : names)
                    {
                        LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                            "X.509 name [type='%s' value='%s'].",
                            name_type_to_string(name.type), name.value);

                        // save identity
                        if (name.type == scard_x509_name_type::ms_upn)
                        {
                            scard_pkcs11_identity identity(_module_path,
                                std::to_string(slot.get_id()), slot.get_token_label(),
                                certificate.get_id_as_string(), name.value);

                            // store identity
                            identities.push_back(std::move(identity));
                        }
                    }
                }

                LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                    "found %" PRIu64 " X.509 identities on-card.",
                    identities.size());

                for (const auto &identity : identities)
                {
                    LOG(LOG_INFO, "scard_pkcs11_enumerator::enumerate: "
                        "X.509 identity [value='%s'].",
                        identity.string());
                }
            }
        }
    }
    catch (const scard_pkcs11_exception &e)
    {
        LOG(LOG_ERR, "scard_pkcs11_enumerator::enumerate: "
            "PKCS#11 error: %s (0x%08lu)", return_value_to_string(e.get_return_value()),
                e.get_return_value());
        
        throw e;
    }
    
    // notify handler
    if (_handler_ptr)
    {
        _handler_ptr->handle_pkcs11_enumeration_end(identities);
    }

    return identities;
}