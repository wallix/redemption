#include <cassert>
#include <dlfcn.h>
#include <memory>
#include <string>
#include <vector>

#include <pkcs11-helper-1.0/pkcs11.h>

#include "scard/scard_pkcs11_common.hpp"
#include "scard/scard_pkcs11_module.hpp"


///////////////////////////////////////////////////////////////////////////////


scard_pkcs11_module scard_pkcs11_module::load(const std::string &library_path)
{
    // check input parameters
	if (library_path.empty())
	{
		throw scard_pkcs11_exception(
            "Failed to load PKCS#11 module: "
            "library path is empty.");
	}

    // load dynamic library
    void *handle = dlopen(library_path.c_str(), RTLD_LAZY);
    if (!handle)
	{
		throw scard_pkcs11_exception(
            "Failed to load PKCS#11 module: "
            "could not load dynamic library.");
	}

    handle_ptr handle_ptr(handle, [](void *h) { dlclose(h); });

	// find function listing function
	auto c_get_function_list = (CK_RV(*)(CK_FUNCTION_LIST_PTR_PTR))
        dlsym(handle, "C_GetFunctionList");
	if (!c_get_function_list)
	{
		throw scard_pkcs11_exception(
            "Failed to load PKCS#11 module: "
            "could not find symbol 'C_GetFunctionList'.");
	}

    function_list *functions_ptr(nullptr);

    // list module functions
	auto rv = c_get_function_list(&functions_ptr);
	if (rv != CKR_OK)
	{
		throw scard_pkcs11_exception(
            "Failed to load PKCS#11 module: "
            "could not list module functions.");
	}

    assert(functions_ptr);

    return scard_pkcs11_module(std::move(handle_ptr),
        std::move(*functions_ptr));
}

scard_pkcs11_module::scard_pkcs11_module(handle_ptr library_handle,
    function_list library_functions)
    :
    _handle(std::move(library_handle)),
    _functions(std::move(library_functions))
{
    CK_C_INITIALIZE_ARGS init_args;
    {
        std::memset(&init_args, 0, sizeof(CK_C_INITIALIZE_ARGS));
        init_args.flags |= CKF_LIBRARY_CANT_CREATE_OS_THREADS;
    }

    // initialize module
    const auto rv = C_Initialize(&init_args);
    if (rv != CKR_OK && rv != CKR_CRYPTOKI_ALREADY_INITIALIZED)
    {
        throw scard_pkcs11_exception("Failed to initialize module.", rv);
    }  
}

scard_pkcs11_module::~scard_pkcs11_module()
{
    C_Finalize(nullptr);
}

scard_pkcs11_slot_list scard_pkcs11_module::list_slots(bool with_token)
{
    CK_RV rv;
    CK_ULONG count;

    // first get the number of slots
    rv = C_GetSlotList((with_token ? CK_TRUE : CK_FALSE), nullptr, &count);
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to retrieve slot count.", rv);
    }

    std::vector<CK_SLOT_ID> slot_ids(count);

    // now get the slot list
    rv = C_GetSlotList((with_token ? CK_TRUE : CK_FALSE), slot_ids.data(), &count);
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to retrieve slot list.", rv);
    }

    scard_pkcs11_slot_list slots;

    for (const auto &slot_id : slot_ids)
    {
        slots.push_back(
            scard_pkcs11_slot(slot_id)
        );
    }

    return slots;
}

void scard_pkcs11_module::describe_slot(scard_pkcs11_slot &slot)
{
    CK_SLOT_INFO slot_info;

    // get slot info for the provided slot
    const auto rv = C_GetSlotInfo(slot.get_id(), &slot_info);
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to retrieve slot info.", rv);
    }

    // update slot information
    slot.set_description(bytes_view(slot_info.slotDescription,
        (sizeof(slot_info.slotDescription) / sizeof(slot_info.slotDescription[0]))
    ));
}

void scard_pkcs11_module::describe_token(scard_pkcs11_slot &slot)
{
    CK_TOKEN_INFO token_info;

    // get token info for the provided slot
    const auto rv = C_GetTokenInfo(slot.get_id(), &token_info);
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to retrieve token info.", rv);
    }

    // update slot information
    slot.set_token_serial(bytes_view(token_info.serialNumber,
        (sizeof(token_info.serialNumber) / sizeof(token_info.serialNumber[0]))
    ));
    slot.set_token_label(bytes_view(token_info.label,
        (sizeof(token_info.label) / sizeof(token_info.label[0]))
    ));
    slot.set_flags(
        (token_info.flags & CKF_LOGIN_REQUIRED)
    );
}

scard_pkcs11_session scard_pkcs11_module::open_session(const scard_pkcs11_slot &slot)
{
    CK_SESSION_HANDLE session_handle(CK_INVALID_HANDLE);

    // open session against the provided slot
    const auto rv = C_OpenSession(slot.get_id(), CKF_SERIAL_SESSION,
        nullptr, nullptr, &session_handle);
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to open session.", rv);
    }

    return scard_pkcs11_session(session_handle);
}

bool scard_pkcs11_module::login(const scard_pkcs11_session &session, writable_bytes_view pin,
    scard_pkcs11_user_type user_type)
{
    bool authenticated(false);

    // login
    auto rv = C_Login(session.get_handle(), user_type, pin.data(), pin.size());
    switch (rv)
    {
        case CKR_OK:
            authenticated = true;
            break;
        case CKR_PIN_INCORRECT:
            authenticated = false;
            break;
        default:
            throw scard_pkcs11_exception("Failed to login.", rv);
    }

    return authenticated;
}

scard_pkcs11_object_list scard_pkcs11_module::list_objects(
    const scard_pkcs11_session &session, std::size_t max_count)
{
    // initiate object search
    auto rv = C_FindObjectsInit(session.get_handle(), nullptr, 0);
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to initiate object search.", rv);
    }

    scard_pkcs11_object_list objects;
    std::vector<CK_OBJECT_HANDLE> object_handles(max_count);
    unsigned long count;

    do
    {
        // reset object count
        count = 0;

        // search objects
        rv = C_FindObjects(session.get_handle(), object_handles.data(), max_count, &count);
        if (rv != CKR_OK)
        {
            // terminate object search
            rv = C_FindObjectsFinal(session.get_handle());
            if (rv != CKR_OK)
            {
                throw scard_pkcs11_exception("Failed to terminate object search while handling object search failure.", rv);
            }

            throw scard_pkcs11_exception("Failed to search objects.", rv);
        }

        // fill in the output object list
        for (unsigned long i = 0; i < count; ++i)
        {
            objects.push_back(
                scard_pkcs11_object(object_handles[i])
            );
        }
    }
    while (count);

    // terminate object search
    rv = C_FindObjectsFinal(session.get_handle());
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to terminate object search.", rv);
    }

    return objects;
}

scard_pkcs11_object_list scard_pkcs11_module::find_objects_by_attributes(
    const scard_pkcs11_session &session, scard_pkcs11_attribute_list &attributes,
    std::size_t max_count)
{
    std::vector<CK_ATTRIBUTE> raw_attributes;

    // form the raw attribute search template
    for (auto &attribute : attributes)
    {
        CK_ATTRIBUTE raw_attribute;

        raw_attribute.type = attribute.get_type();
        raw_attribute.pValue = attribute.data();
        raw_attribute.ulValueLen = attribute.get_size();

        raw_attributes.push_back(
            std::move(raw_attribute)
        );
    }

    // initiate object search
    auto rv = C_FindObjectsInit(session.get_handle(), raw_attributes.data(), raw_attributes.size());
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to initiate object search.", rv);
    }

    scard_pkcs11_object_list objects;
    std::vector<CK_OBJECT_HANDLE> object_handles(max_count);
    unsigned long count;

    do
    {
        // reset object count
        count = 0;

        // search objects
        rv = C_FindObjects(session.get_handle(), object_handles.data(), max_count, &count);
        if (rv != CKR_OK)
        {
            // terminate object search
            rv = C_FindObjectsFinal(session.get_handle());
            if (rv != CKR_OK)
            {
                throw scard_pkcs11_exception("Failed to terminate object search while handling object search failure.", rv);
            }

            throw scard_pkcs11_exception("Failed to search objects.", rv);
        }

        // fill in the output object list
        for (unsigned long i = 0; i < count; ++i)
        {
            scard_pkcs11_object object(object_handles[i]);

            for (const auto &raw_attribute : raw_attributes)
            {
                object.set_attribute(raw_attribute.type, raw_attribute.pValue,
                    raw_attribute.ulValueLen);
            }

            objects.push_back(
                std::move(object)
            );
        }
    }
    while (count);

    // terminate object search
    rv = C_FindObjectsFinal(session.get_handle());
    if (rv != CKR_OK)
    {
        throw scard_pkcs11_exception("Failed to terminate object search.", rv);
    }

    return objects;
}

void scard_pkcs11_module::describe_object(const scard_pkcs11_session &session,
    const scard_pkcs11_attribute_type_list &attribute_types, scard_pkcs11_object &object)
{
    std::vector<CK_ATTRIBUTE> raw_attributes;

    // prepare the raw attribute search template
    for (const auto &attribute_type : attribute_types)
    {
        CK_ATTRIBUTE raw_attribute;

        raw_attribute.type = attribute_type;
        raw_attribute.pValue = nullptr;
        raw_attribute.ulValueLen = 0;

        raw_attributes.push_back(
            std::move(raw_attribute)
        );
    }

    // perform a first API call in order to get attribute sizes
    auto rv = C_GetAttributeValue(session.get_handle(), object.get_handle(),
        raw_attributes.data(), raw_attributes.size());
    switch (rv)
	{
        case CKR_ATTRIBUTE_TYPE_INVALID:
        {
            /* size inquiry failed: at least one attribute cannot be accessed */

            std::vector<CK_ATTRIBUTE> corrected_raw_attributes;

            // prepare a new raw attribute search template, excluding the inaccessible attributes,
            // and initialize those raw attributes with a default value
            // note: erasing them may be worse because of vector data relocation
            for (auto &raw_attribute : raw_attributes)
            {
                if (raw_attribute.ulValueLen != CK_UNAVAILABLE_INFORMATION)
                {
                    // move raw attribute into the new search template
                    corrected_raw_attributes.push_back(
                        std::move(raw_attribute)
                    );
                }
                else
                {
                    // initialize object attribute to defaults
                    object.set_attribute(raw_attribute.type, 0);
                }
            }

            // replace raw attribute search template by the corrected one
            raw_attributes = std::move(corrected_raw_attributes);
        } // FALL-THROUGH

		case CKR_OK:
		case CKR_BUFFER_TOO_SMALL:
        {
            /* size inquiry succeeded */

            for (auto &raw_attribute : raw_attributes)
            {
                assert(!raw_attribute.pValue && raw_attribute.ulValueLen);

                // allocate or resize object attribute
                auto &attribute = object.set_attribute(raw_attribute.type,
                    raw_attribute.ulValueLen);

                // update the raw attribute search template
                raw_attribute.pValue = attribute.data();
                raw_attribute.ulValueLen = attribute.get_size();
            }

            // perform a final API call in order to get attribute values
            auto rv = C_GetAttributeValue(session.get_handle(), object.get_handle(),
                raw_attributes.data(), raw_attributes.size());
            if (rv == CKR_OK)
            {
                break;
            }
        } // FALL-THROUGH

        default:
            throw scard_pkcs11_exception("Failed to get object attribute(s).", rv);
    }
}

scard_pkcs11_certificate_list scard_pkcs11_module::find_signing_certificates(
    const scard_pkcs11_session &session)
{
    scard_pkcs11_attribute_list search_template;

    // form the search template for finding private keys with signing capability
    {
        CK_OBJECT_CLASS object_class(CKO_PRIVATE_KEY);
        CK_KEY_TYPE key_type(CKK_RSA);
        CK_BBOOL can_sign(CK_TRUE);

        search_template.push_back(
            scard_pkcs11_attribute(CKA_CLASS, &object_class, sizeof(object_class))
        );
        search_template.push_back(
            scard_pkcs11_attribute(CKA_SIGN, &can_sign, sizeof(can_sign))
        );
        search_template.push_back(
            scard_pkcs11_attribute(CKA_KEY_TYPE, &key_type, sizeof(key_type))
        );
    }

    // find private key objects
    auto private_key_objects = find_objects_by_attributes(session, search_template);

    scard_pkcs11_certificate_list certificates;

    // find related certificates
    for (auto &private_key_object : private_key_objects)
    {
        // retrieve object ID
        describe_object(session, { CKA_ID }, private_key_object);

        const auto &id = private_key_object.get_attribute(CKA_ID);

        // prepare for new search
        search_template.clear();

        // form the search template for finding certificates
        // associated with the current signing private key
        {
            CK_OBJECT_CLASS object_class(CKO_CERTIFICATE);
            CK_CERTIFICATE_TYPE certificate_type(CKC_X_509);

            // 
            search_template.push_back(
                scard_pkcs11_attribute(CKA_CLASS, &object_class, sizeof(object_class))
            );
            search_template.push_back(
                scard_pkcs11_attribute(CKA_CERTIFICATE_TYPE, &certificate_type, sizeof(certificate_type))
            );
            search_template.push_back(
                scard_pkcs11_attribute(CKA_ID, id.get_value())
            );
        }

        // find certificate objects
        auto certificate_objects = find_objects_by_attributes(session, search_template);
        if (certificate_objects.size() != 1)
        {
            continue;
        }

        auto &certificate_object = certificate_objects.front();

        // retrieve certificate attributes
        describe_object(session, { CKA_CLASS, CKA_ID, CKA_OBJECT_ID, CKA_LABEL,
            CKA_CERTIFICATE_TYPE, CKA_CERTIFICATE_CATEGORY, CKA_ISSUER,
            CKA_SUBJECT, CKA_VALUE, CKA_KEY_TYPE }, certificate_object);
        
        const auto &label_attribute = certificate_object.get_attribute(CKA_LABEL);
        const auto &type_attribute = certificate_object.get_attribute(CKA_CERTIFICATE_TYPE);
        const auto &key_type_attribute = certificate_object.get_attribute(CKA_KEY_TYPE);
        const auto &category_attribute = certificate_object.get_attribute(CKA_CERTIFICATE_CATEGORY);
        const auto &issuer_attribute = certificate_object.get_attribute(CKA_ISSUER);
        const auto &subject_attribute = certificate_object.get_attribute(CKA_SUBJECT);
        const auto &value_attribute = certificate_object.get_attribute(CKA_VALUE);
        
        certificates.push_back(
            scard_pkcs11_certificate(
                id.get_value(),
                label_attribute.get_value(),
                type_attribute.get_value(),
                key_type_attribute.get_value(),
                category_attribute.get_value(),
                issuer_attribute.get_value(),
                subject_attribute.get_value(),
                value_attribute.get_value()
            )
        );
    }

    return certificates;
}


//////// PKCS#11 API //////////////////////////////////////////////////////


template<typename Function, typename... Arguments>
scard_pkcs11_module::return_value
call_pkcs11_api(Function function, Arguments... arguments)
{
    return function(arguments...);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR function_list)
{
	assert(_functions.C_GetFunctionList);

    return call_pkcs11_api(_functions.C_GetFunctionList,
        function_list);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Initialize(CK_VOID_PTR init_args)
{
	assert(_functions.C_Initialize);

    return call_pkcs11_api(_functions.C_Initialize,
        init_args);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Finalize(CK_VOID_PTR reserved)
{
	assert(_functions.C_Finalize);

    return call_pkcs11_api(_functions.C_Finalize,
        reserved);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetInfo(CK_INFO_PTR info)
{
	assert(_functions.C_GetInfo);

    return call_pkcs11_api(_functions.C_GetInfo,
        info);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetSlotList(CK_BBOOL token_present, CK_SLOT_ID_PTR slot_list,
    CK_ULONG_PTR count)
{
	assert(_functions.C_GetSlotList);

    return call_pkcs11_api(_functions.C_GetSlotList,
        token_present, slot_list, count);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetSlotInfo(CK_SLOT_ID slot_id, CK_SLOT_INFO_PTR info)
{
	assert(_functions.C_GetSlotInfo);

    return call_pkcs11_api(_functions.C_GetSlotInfo,
        slot_id, info);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetTokenInfo(CK_SLOT_ID slot_id, CK_TOKEN_INFO_PTR info)
{
	assert(_functions.C_GetTokenInfo);

    return call_pkcs11_api(_functions.C_GetTokenInfo,
        slot_id, info);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetMechanismList(CK_SLOT_ID  slot_id, CK_MECHANISM_TYPE_PTR mechanism_list,
    CK_ULONG_PTR count)
{
	assert(_functions.C_GetMechanismList);

    return call_pkcs11_api(_functions.C_GetMechanismList,
        slot_id, mechanism_list, count);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetMechanismInfo(CK_SLOT_ID  slot_id, CK_MECHANISM_TYPE type,
    CK_MECHANISM_INFO_PTR info)
{
	assert(_functions.C_GetMechanismInfo);

    return call_pkcs11_api(_functions.C_GetMechanismInfo,
        slot_id, type, info);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_InitToken(CK_SLOT_ID slot_id, CK_UTF8CHAR_PTR pin, CK_ULONG pin_length,
    CK_UTF8CHAR_PTR label)
{
	assert(_functions.C_InitToken);

    return call_pkcs11_api(_functions.C_InitToken,
        slot_id, pin, pin_length, label);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_InitPIN(CK_SESSION_HANDLE session, CK_UTF8CHAR_PTR pin, CK_ULONG pin_length)
{
	assert(_functions.C_InitPIN);

    return call_pkcs11_api(_functions.C_InitPIN,
        session, pin, pin_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SetPIN(CK_SESSION_HANDLE session, CK_UTF8CHAR_PTR old_pin, CK_ULONG old_pin_length,
    CK_UTF8CHAR_PTR new_pin, CK_ULONG new_pin_length)
{
	assert(_functions.C_SetPIN);

    return call_pkcs11_api(_functions.C_SetPIN,
        session, old_pin, old_pin_length, new_pin, new_pin_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_OpenSession(CK_SLOT_ID slot_id, CK_FLAGS flags, CK_VOID_PTR application,
    CK_NOTIFY notify, CK_SESSION_HANDLE_PTR session)
{
	assert(_functions.C_OpenSession);

    return call_pkcs11_api(_functions.C_OpenSession,
        slot_id, flags, application, notify, session);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_CloseSession(CK_SESSION_HANDLE session)
{
	assert(_functions.C_CloseSession);

    return call_pkcs11_api(_functions.C_CloseSession,
        session);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_CloseAllSessions(CK_SLOT_ID slot_id)
{
	assert(_functions.C_CloseAllSessions);

    return call_pkcs11_api(_functions.C_CloseAllSessions,
        slot_id);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetSessionInfo(CK_SESSION_HANDLE session, CK_SESSION_INFO_PTR info)
{
	assert(_functions.C_GetSessionInfo);

    return call_pkcs11_api(_functions.C_GetSessionInfo,
        session, info);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetOperationState(CK_SESSION_HANDLE session, CK_BYTE_PTR operation_state,
    CK_ULONG_PTR operation_state_length)
{
	assert(_functions.C_GetOperationState);

    return call_pkcs11_api(_functions.C_GetOperationState,
        session, operation_state, operation_state_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SetOperationState(CK_SESSION_HANDLE session, CK_BYTE_PTR operation_state,
    CK_ULONG operation_state_length, CK_OBJECT_HANDLE encryption_key,
    CK_OBJECT_HANDLE authentication_key)
{
	assert(_functions.C_SetOperationState);

    return call_pkcs11_api(_functions.C_SetOperationState,
        session, operation_state, operation_state_length, encryption_key, authentication_key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Login(CK_SESSION_HANDLE session, CK_USER_TYPE user_type,
    CK_UTF8CHAR_PTR pin, CK_ULONG pin_length)
{
	assert(_functions.C_Login);

    return call_pkcs11_api(_functions.C_Login,
        session, user_type, pin, pin_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Logout(CK_SESSION_HANDLE session)
{
	assert(_functions.C_Logout);

    return call_pkcs11_api(_functions.C_Logout,
        session);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_CreateObject(CK_SESSION_HANDLE session, CK_ATTRIBUTE_PTR template_, CK_ULONG count,
    CK_OBJECT_HANDLE_PTR object)
{
	assert(_functions.C_CreateObject);

    return call_pkcs11_api(_functions.C_CreateObject,
        session, template_, count, object);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_CopyObject(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
    CK_ATTRIBUTE_PTR template_, CK_ULONG count, CK_OBJECT_HANDLE_PTR new_object)
{
	assert(_functions.C_CopyObject);

    return call_pkcs11_api(_functions.C_CopyObject,
        session, object, template_, count, new_object);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DestroyObject(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object)
{
	assert(_functions.C_DestroyObject);

    return call_pkcs11_api(_functions.C_DestroyObject,
        session, object);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetObjectSize(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object, CK_ULONG_PTR size)
{
	assert(_functions.C_GetObjectSize);

    return call_pkcs11_api(_functions.C_GetObjectSize,
        session, object, size);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetAttributeValue(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
    CK_ATTRIBUTE_PTR template_, CK_ULONG count)
{
	assert(_functions.C_GetAttributeValue);

    return call_pkcs11_api(_functions.C_GetAttributeValue,
        session, object, template_, count);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SetAttributeValue(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
		CK_ATTRIBUTE_PTR template_, CK_ULONG count)
{
	assert(_functions.C_SetAttributeValue);

    return call_pkcs11_api(_functions.C_SetAttributeValue,
        session, object, template_, count);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_FindObjectsInit(CK_SESSION_HANDLE session, CK_ATTRIBUTE_PTR template_,
    CK_ULONG count)
{
	assert(_functions.C_FindObjectsInit);

    return call_pkcs11_api(_functions.C_FindObjectsInit,
        session, template_, count);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_FindObjects(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE_PTR object,
    CK_ULONG max_object_count, CK_ULONG_PTR object_count)
{
	assert(_functions.C_FindObjects);

    return call_pkcs11_api(_functions.C_FindObjects,
        session, object, max_object_count, object_count);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_FindObjectsFinal(CK_SESSION_HANDLE session)
{
	assert(_functions.C_FindObjectsFinal);

    return call_pkcs11_api(_functions.C_FindObjectsFinal,
        session);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_EncryptInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_OBJECT_HANDLE key)
{
	assert(_functions.C_EncryptInit);

    return call_pkcs11_api(_functions.C_EncryptInit,
        session, mechanism, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Encrypt(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
    CK_BYTE_PTR encrypted_data, CK_ULONG_PTR encrypted_data_length)
{
	assert(_functions.C_Encrypt);

    return call_pkcs11_api(_functions.C_Encrypt,
        session, data, data_length, encrypted_data, encrypted_data_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_EncryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length,
    CK_BYTE_PTR encrypted_part, CK_ULONG_PTR encrypted_part_length)
{
	assert(_functions.C_EncryptUpdate);

    return call_pkcs11_api(_functions.C_EncryptUpdate,
        session, part, part_length, encrypted_part, encrypted_part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_EncryptFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR last_encrypted_part,
    CK_ULONG_PTR last_encrypted_part_length)
{
	assert(_functions.C_EncryptFinal);

    return call_pkcs11_api(_functions.C_EncryptFinal,
        session, last_encrypted_part, last_encrypted_part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DecryptInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key)
{
	assert(_functions.C_DecryptInit);

    return call_pkcs11_api(_functions.C_DecryptInit,
        session, mechanism, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Decrypt(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_data, CK_ULONG encrypted_data_length,
    CK_BYTE_PTR data, CK_ULONG_PTR data_length)
{
	assert(_functions.C_Decrypt);

    return call_pkcs11_api(_functions.C_Decrypt,
        session, encrypted_data, encrypted_data_length, data, data_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DecryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_part,
    CK_ULONG encrypted_part_length, CK_BYTE_PTR part, CK_ULONG_PTR part_length)
{
	assert(_functions.C_DecryptUpdate);

    return call_pkcs11_api(_functions.C_DecryptUpdate,
        session, encrypted_part, encrypted_part_length, part, part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DecryptFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR last_part,
    CK_ULONG_PTR last_part_length)
{
	assert(_functions.C_DecryptFinal);

    return call_pkcs11_api(_functions.C_DecryptFinal,
        session, last_part, last_part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DigestInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism)
{
	assert(_functions.C_DigestInit);

    return call_pkcs11_api(_functions.C_DigestInit,
        session, mechanism);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Digest(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
    CK_BYTE_PTR digest, CK_ULONG_PTR digest_length)
{
	assert(_functions.C_Digest);

    return call_pkcs11_api(_functions.C_Digest,
        session, data, data_length, digest, digest_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DigestUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length)
{
	assert(_functions.C_DigestUpdate);

    return call_pkcs11_api(_functions.C_DigestUpdate,
        session, part, part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DigestKey(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE key)
{
	assert(_functions.C_DigestKey);

    return call_pkcs11_api(_functions.C_DigestKey,
        session, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DigestFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR digest, CK_ULONG_PTR digest_length)
{
	assert(_functions.C_DigestFinal);

    return call_pkcs11_api(_functions.C_DigestFinal,
        session, digest, digest_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SignInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key)
{
	assert(_functions.C_SignInit);

    return call_pkcs11_api(_functions.C_SignInit,
        session, mechanism, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Sign(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
    CK_BYTE_PTR signature, CK_ULONG_PTR signature_length)
{
	assert(_functions.C_Sign);

    return call_pkcs11_api(_functions.C_Sign,
        session, data, data_length, signature, signature_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SignUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length)
{
	assert(_functions.C_SignUpdate);

    return call_pkcs11_api(_functions.C_SignUpdate,
        session, part, part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SignFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR signature, CK_ULONG_PTR signature_length)
{
	assert(_functions.C_SignFinal);

    return call_pkcs11_api(_functions.C_SignFinal,
        session, signature, signature_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SignRecoverInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key)
{
	assert(_functions.C_SignRecoverInit);

    return call_pkcs11_api(_functions.C_SignRecoverInit,
        session, mechanism, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SignRecover(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
    CK_BYTE_PTR signature, CK_ULONG_PTR signature_length)
{
	assert(_functions.C_SignRecover);

    return call_pkcs11_api(_functions.C_SignRecover,
        session, data, data_length, signature, signature_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_VerifyInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key)
{
	assert(_functions.C_VerifyInit);

    return call_pkcs11_api(_functions.C_VerifyInit,
        session, mechanism, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_Verify(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
    CK_BYTE_PTR signature, CK_ULONG signature_length)
{
	assert(_functions.C_Verify);

    return call_pkcs11_api(_functions.C_Verify,
        session, data, data_length, signature, signature_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_VerifyUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length)
{
	assert(_functions.C_VerifyUpdate);

    return call_pkcs11_api(_functions.C_VerifyUpdate,
        session, part, part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_VerifyFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR signature, CK_ULONG signature_length)
{
	assert(_functions.C_VerifyFinal);

    return call_pkcs11_api(_functions.C_VerifyFinal,
        session, signature, signature_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_VerifyRecoverInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_OBJECT_HANDLE key)
{
	assert(_functions.C_VerifyRecoverInit);

    return call_pkcs11_api(_functions.C_VerifyRecoverInit,
        session, mechanism, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_VerifyRecover(CK_SESSION_HANDLE session, CK_BYTE_PTR signature, CK_ULONG signature_length,
    CK_BYTE_PTR data, CK_ULONG_PTR data_length)
{
	assert(_functions.C_VerifyRecover);

    return call_pkcs11_api(_functions.C_VerifyRecover,
        session, signature, signature_length, data, data_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DigestEncryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length,
    CK_BYTE_PTR encrypted_part, CK_ULONG_PTR encrypted_part_length)
{
	assert(_functions.C_DigestEncryptUpdate);

    return call_pkcs11_api(_functions.C_DigestEncryptUpdate,
        session, part, part_length, encrypted_part, encrypted_part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DecryptDigestUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_part,
    CK_ULONG encrypted_part_length, CK_BYTE_PTR part, CK_ULONG_PTR part_length)
{
	assert(_functions.C_DecryptDigestUpdate);

    return call_pkcs11_api(_functions.C_DecryptDigestUpdate,
        session, encrypted_part, encrypted_part_length, part, part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SignEncryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length,
    CK_BYTE_PTR encrypted_part, CK_ULONG_PTR encrypted_part_length)
{
	assert(_functions.C_SignEncryptUpdate);

    return call_pkcs11_api(_functions.C_SignEncryptUpdate,
        session, part, part_length, encrypted_part, encrypted_part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DecryptVerifyUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_part,
    CK_ULONG encrypted_part_length, CK_BYTE_PTR part, CK_ULONG_PTR part_length)
{
	assert(_functions.C_DecryptVerifyUpdate);

    return call_pkcs11_api(_functions.C_DecryptVerifyUpdate,
        session, encrypted_part, encrypted_part_length, part, part_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GenerateKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_ATTRIBUTE_PTR template_, CK_ULONG count, CK_OBJECT_HANDLE_PTR key)
{
	assert(_functions.C_GenerateKey);

    return call_pkcs11_api(_functions.C_GenerateKey,
        session, mechanism, template_, count, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GenerateKeyPair(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_ATTRIBUTE_PTR public_key_template, CK_ULONG public_key_attribute_count,
    CK_ATTRIBUTE_PTR private_key_template, CK_ULONG private_key_attribute_count,
    CK_OBJECT_HANDLE_PTR public_key, CK_OBJECT_HANDLE_PTR private_key)
{
	assert(_functions.C_GenerateKeyPair);

    return call_pkcs11_api(_functions.C_GenerateKeyPair,
        session, mechanism, public_key_template, public_key_attribute_count,
        private_key_template, private_key_attribute_count, public_key, private_key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_WrapKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_OBJECT_HANDLE wrapping_key, CK_OBJECT_HANDLE key,
    CK_BYTE_PTR wrapped_key, CK_ULONG_PTR wrapped_key_length)
{
	assert(_functions.C_WrapKey);

    return call_pkcs11_api(_functions.C_WrapKey,
        session, mechanism, wrapping_key, key, wrapped_key, wrapped_key_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_UnwrapKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_OBJECT_HANDLE unwrapping_key, CK_BYTE_PTR wrapped_key, CK_ULONG wrapped_key_length,
    CK_ATTRIBUTE_PTR template_, CK_ULONG attribute_count, CK_OBJECT_HANDLE_PTR key)
{
	assert(_functions.C_UnwrapKey);

    return call_pkcs11_api(_functions.C_UnwrapKey,
        session, mechanism, unwrapping_key, wrapped_key, wrapped_key_length,
        template_, attribute_count, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_DeriveKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
    CK_OBJECT_HANDLE base_key, CK_ATTRIBUTE_PTR template_, CK_ULONG attribute_count,
    CK_OBJECT_HANDLE_PTR key)
{
	assert(_functions.C_DeriveKey);

    return call_pkcs11_api(_functions.C_DeriveKey,
        session, mechanism, base_key, template_, attribute_count, key);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_SeedRandom(CK_SESSION_HANDLE session, CK_BYTE_PTR seed, CK_ULONG seed_length)
{
	assert(_functions.C_SeedRandom);

    return call_pkcs11_api(_functions.C_SeedRandom,
        session, seed, seed_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GenerateRandom(CK_SESSION_HANDLE session, CK_BYTE_PTR random_data,
    CK_ULONG random_data_length)
{
	assert(_functions.C_GenerateRandom);

    return call_pkcs11_api(_functions.C_GenerateRandom,
        session, random_data, random_data_length);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_GetFunctionStatus(CK_SESSION_HANDLE session)
{
	assert(_functions.C_GetFunctionStatus);

    return call_pkcs11_api(_functions.C_GetFunctionStatus, session);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_CancelFunction(CK_SESSION_HANDLE session)
{
	assert(_functions.C_CancelFunction);

    return call_pkcs11_api(_functions.C_CancelFunction, session);
}

scard_pkcs11_module::return_value
scard_pkcs11_module::C_WaitForSlotEvent(CK_FLAGS flags, CK_SLOT_ID_PTR slot, CK_VOID_PTR reserved)
{
	assert(_functions.C_WaitForSlotEvent);

    return call_pkcs11_api(_functions.C_WaitForSlotEvent, flags, slot, reserved);
}