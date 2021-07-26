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

#include <dlfcn.h>
#include <memory>
#include <string>

#include <pkcs11-helper-1.0/pkcs11.h>

#include "scard/scard_pkcs11_common.hpp"


///////////////////////////////////////////////////////////////////////////////


class scard_pkcs11_module
{
public:
    ///
    static scard_pkcs11_module load(const std::string &library_path);

    ///
    scard_pkcs11_module() = delete;

    ///
    ~scard_pkcs11_module();

    ///
    scard_pkcs11_slot_list list_slots(bool with_token);

    ///
    void describe_slot(scard_pkcs11_slot &slot);

    ///
    void describe_token(scard_pkcs11_slot &slot);

    ///
    scard_pkcs11_session open_session(const scard_pkcs11_slot &slot);

    ///
    bool login(const scard_pkcs11_session &session, writable_bytes_view pin,
        scard_pkcs11_user_type user_type = CKU_USER);

    ///
    scard_pkcs11_object_list list_objects(const scard_pkcs11_session &session,
        std::size_t max_count = 128);

    ///
    scard_pkcs11_object_list find_objects_by_attributes(const scard_pkcs11_session &session,
        scard_pkcs11_attribute_list &attributes, std::size_t max_count = 128);
    
    ///
    void describe_object(const scard_pkcs11_session &session,
        const scard_pkcs11_attribute_type_list &attribute_types, scard_pkcs11_object &object);

    ///
    scard_pkcs11_certificate_list find_signing_certificates(const scard_pkcs11_session &session);

private:
    ///
    typedef std::unique_ptr<void, void(*)(void *)> handle_ptr;

    ///
    typedef CK_FUNCTION_LIST function_list;

    ///
    typedef CK_RV return_value;

    ///
    scard_pkcs11_module(handle_ptr library_handle,
        function_list functions);


    //////// PKCS#11 API //////////////////////////////////////////////////////


    ///
    return_value C_GetFunctionList(CK_FUNCTION_LIST_PTR_PTR function_list);

    ///
    return_value C_Initialize(CK_VOID_PTR init_args);

    ///
    return_value C_Finalize(CK_VOID_PTR reserved);

    ///
    return_value C_GetInfo(CK_INFO_PTR info);

    ///
    return_value C_GetSlotList(CK_BBOOL token_present, CK_SLOT_ID_PTR slot_list,
        CK_ULONG_PTR count);

    ///
    return_value C_GetSlotInfo(CK_SLOT_ID slot_id, CK_SLOT_INFO_PTR info);

    ///
    return_value C_GetTokenInfo(CK_SLOT_ID slot_id, CK_TOKEN_INFO_PTR info);

    ///
    return_value C_GetMechanismList(CK_SLOT_ID  slot_id, CK_MECHANISM_TYPE_PTR mechanism_list,
        CK_ULONG_PTR count);

    ///
    return_value C_GetMechanismInfo(CK_SLOT_ID  slot_id, CK_MECHANISM_TYPE type,
        CK_MECHANISM_INFO_PTR info);

    ///
    return_value C_InitToken(CK_SLOT_ID slot_id, CK_UTF8CHAR_PTR pin, CK_ULONG pin_length,
        CK_UTF8CHAR_PTR label);

    ///
    return_value C_InitPIN(CK_SESSION_HANDLE session, CK_UTF8CHAR_PTR pin, CK_ULONG pin_length);

    ///
    return_value C_SetPIN(CK_SESSION_HANDLE session, CK_UTF8CHAR_PTR old_pin, CK_ULONG old_pin_length,
        CK_UTF8CHAR_PTR new_pin, CK_ULONG new_pin_length);

    ///
    return_value C_OpenSession(CK_SLOT_ID slot_id, CK_FLAGS flags, CK_VOID_PTR application,
        CK_NOTIFY notify, CK_SESSION_HANDLE_PTR session);

    ///
    return_value C_CloseSession(CK_SESSION_HANDLE session);

    ///
    return_value C_CloseAllSessions(CK_SLOT_ID slot_id);

    ///
    return_value C_GetSessionInfo(CK_SESSION_HANDLE session, CK_SESSION_INFO_PTR info);

    ///
    return_value C_GetOperationState(CK_SESSION_HANDLE session, CK_BYTE_PTR operation_state,
        CK_ULONG_PTR operation_state_length);

    ///
    return_value C_SetOperationState(CK_SESSION_HANDLE session, CK_BYTE_PTR operation_state,
        CK_ULONG operation_state_length, CK_OBJECT_HANDLE encryption_key,
        CK_OBJECT_HANDLE authentication_key);

    ///
    return_value C_Login(CK_SESSION_HANDLE session, CK_USER_TYPE user_type,
        CK_UTF8CHAR_PTR pin, CK_ULONG pin_length);

    ///
    return_value C_Logout(CK_SESSION_HANDLE session);

    ///
    return_value C_CreateObject(CK_SESSION_HANDLE session, CK_ATTRIBUTE_PTR template_, CK_ULONG count,
        CK_OBJECT_HANDLE_PTR object);

    ///
    return_value C_CopyObject(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
        CK_ATTRIBUTE_PTR template_, CK_ULONG count, CK_OBJECT_HANDLE_PTR new_object);

    ///
    return_value C_DestroyObject(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object);

    ///
    return_value C_GetObjectSize(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object, CK_ULONG_PTR size);

    ///
    return_value C_GetAttributeValue(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
        CK_ATTRIBUTE_PTR template_, CK_ULONG count);

    ///
    return_value C_SetAttributeValue(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE object,
            CK_ATTRIBUTE_PTR template_, CK_ULONG count);

    ///
    return_value C_FindObjectsInit(CK_SESSION_HANDLE session, CK_ATTRIBUTE_PTR template_,
        CK_ULONG count);

    ///
    return_value C_FindObjects(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE_PTR object,
        CK_ULONG max_object_count, CK_ULONG_PTR object_count);

    ///
    return_value C_FindObjectsFinal(CK_SESSION_HANDLE session);

    ///
    return_value C_EncryptInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_OBJECT_HANDLE key);

    ///
    return_value C_Encrypt(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
        CK_BYTE_PTR encrypted_data, CK_ULONG_PTR encrypted_data_length);

    ///
    return_value C_EncryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length,
        CK_BYTE_PTR encrypted_part, CK_ULONG_PTR encrypted_part_length);

    ///
    return_value C_EncryptFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR last_encrypted_part,
        CK_ULONG_PTR last_encrypted_part_length);

    ///
    return_value C_DecryptInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key);

    ///
    return_value C_Decrypt(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_data, CK_ULONG encrypted_data_length,
        CK_BYTE_PTR data, CK_ULONG_PTR data_length);

    ///
    return_value C_DecryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_part,
        CK_ULONG encrypted_part_length, CK_BYTE_PTR part, CK_ULONG_PTR part_length);

    ///
    return_value C_DecryptFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR last_part,
        CK_ULONG_PTR last_part_length);

    ///
    return_value C_DigestInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism);

    ///
    return_value C_Digest(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
        CK_BYTE_PTR digest, CK_ULONG_PTR digest_length);

    ///
    return_value C_DigestUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length);

    ///
    return_value C_DigestKey(CK_SESSION_HANDLE session, CK_OBJECT_HANDLE key);

    ///
    return_value C_DigestFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR digest, CK_ULONG_PTR digest_length);

    ///
    return_value C_SignInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key);

    ///
    return_value C_Sign(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
        CK_BYTE_PTR signature, CK_ULONG_PTR signature_length);

    ///
    return_value C_SignUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length);

    ///
    return_value C_SignFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR signature, CK_ULONG_PTR signature_length);

    ///
    return_value C_SignRecoverInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key);

    ///
    return_value C_SignRecover(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
        CK_BYTE_PTR signature, CK_ULONG_PTR signature_length);

    ///
    return_value C_VerifyInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism, CK_OBJECT_HANDLE key);

    ///
    return_value C_Verify(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG data_length,
        CK_BYTE_PTR signature, CK_ULONG signature_length);

    ///
    return_value C_VerifyUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length);

    ///
    return_value C_VerifyFinal(CK_SESSION_HANDLE session, CK_BYTE_PTR signature, CK_ULONG signature_length);

    ///
    return_value C_VerifyRecoverInit(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_OBJECT_HANDLE key);

    ///
    return_value C_VerifyRecover(CK_SESSION_HANDLE session, CK_BYTE_PTR signature, CK_ULONG signature_length,
        CK_BYTE_PTR data, CK_ULONG_PTR data_length);

    ///
    return_value C_DigestEncryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length,
        CK_BYTE_PTR encrypted_part, CK_ULONG_PTR encrypted_part_length);

    ///
    return_value C_DecryptDigestUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_part,
        CK_ULONG encrypted_part_length, CK_BYTE_PTR part, CK_ULONG_PTR part_length);

    ///
    return_value C_SignEncryptUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR part, CK_ULONG part_length,
        CK_BYTE_PTR encrypted_part, CK_ULONG_PTR encrypted_part_length);

    ///
    return_value C_DecryptVerifyUpdate(CK_SESSION_HANDLE session, CK_BYTE_PTR encrypted_part,
        CK_ULONG encrypted_part_length, CK_BYTE_PTR part, CK_ULONG_PTR part_length);

    ///
    return_value C_GenerateKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_ATTRIBUTE_PTR template_, CK_ULONG count, CK_OBJECT_HANDLE_PTR key);

    ///
    return_value C_GenerateKeyPair(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_ATTRIBUTE_PTR public_key_template, CK_ULONG public_key_attribute_count,
        CK_ATTRIBUTE_PTR private_key_template, CK_ULONG private_key_attribute_count,
        CK_OBJECT_HANDLE_PTR public_key, CK_OBJECT_HANDLE_PTR private_key);

    ///
    return_value C_WrapKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_OBJECT_HANDLE wrapping_key, CK_OBJECT_HANDLE key,
        CK_BYTE_PTR wrapped_key, CK_ULONG_PTR wrapped_key_length);

    ///
    return_value C_UnwrapKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_OBJECT_HANDLE unwrapping_key, CK_BYTE_PTR wrapped_key, CK_ULONG wrapped_key_length,
        CK_ATTRIBUTE_PTR template_, CK_ULONG attribute_count, CK_OBJECT_HANDLE_PTR key);

    ///
    return_value C_DeriveKey(CK_SESSION_HANDLE session, CK_MECHANISM_PTR mechanism,
        CK_OBJECT_HANDLE base_key, CK_ATTRIBUTE_PTR template_, CK_ULONG attribute_count,
        CK_OBJECT_HANDLE_PTR key);

    ///
    return_value C_SeedRandom(CK_SESSION_HANDLE session, CK_BYTE_PTR seed, CK_ULONG seed_length);

    ///
    return_value C_GenerateRandom(CK_SESSION_HANDLE session, CK_BYTE_PTR random_data,
        CK_ULONG random_data_length);

    ///
    return_value C_GetFunctionStatus(CK_SESSION_HANDLE session);

    ///
    return_value C_CancelFunction(CK_SESSION_HANDLE session);

    ///
    return_value C_WaitForSlotEvent(CK_FLAGS flags, CK_SLOT_ID_PTR slot, CK_VOID_PTR reserved);

private:
    ///
    handle_ptr _handle;

    ///
    function_list _functions;
};

