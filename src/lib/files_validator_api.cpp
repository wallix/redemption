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
   Copyright (C) Wallix 2019
   Author(s): Clément Moroldo
*/

#include "lib/files_validator_api.hpp"
#include "mod/file_validator_service.hpp"
#include "utils/c_interface.hpp"
#include "utils/netutils.hpp"


namespace
{
    struct ValidatorTransport : Transport
    {
        unique_fd file;
        int errnum = 0;

        ValidatorTransport(unique_fd&& ufd) noexcept
        : file(std::move(ufd))
        {}

        std::size_t do_partial_read(uint8_t * buffer, std::size_t len) override
        {
            ssize_t res;
            do
            {
                res = ::read(this->file.fd(), buffer, len);
            } while (res == 0 && errno == EINTR);

            if (res <= 0)
            {
                if (res < 0 || len != 0) {
                    this->errnum = errno;
                }
            }

            return static_cast<size_t>(res);
        }

        void do_send(const uint8_t * data, std::size_t len) override
        {
            size_t total_sent = 0;
            while (len > total_sent)
            {
                ssize_t const ret = ::write(this->file.fd(), data + total_sent, len - total_sent);
                if (ret <= 0)
                {
                    if (errno == EINTR)
                    {
                        continue;
                    }
                    this->errnum = errno;
                    return;
                }
                total_sent += ret;
            }
        }
    };

    using DataMap = FileValidatorService::DataMap;

    template<class... Ts>
    auto to_data_map(Ts... xs)
    {
        return std::array{bytes_view(std::string_view(xs))...};
    }
} // anonymous namespace


struct ValidatorApi
{
    ValidatorApi(unique_fd ufd) noexcept
    : transport(std::move(ufd))
    {}

    ValidatorApi(char const* path)
    : ValidatorApi(addr_connect(path, false /*no_log_for_unix_socket*/))
    {}

    ValidatorTransport transport;
    FileValidatorService file_validator{transport};
    bool wating_data = false;
    FileValidatorService::ResponseType response_type = FileValidatorService::ResponseType::WaitingData;
};

struct ValidatorKeyValueList
{
    static constexpr std::size_t str_max_length = ~uint16_t{};

    void push(char const* key, char const* value)
    {
        auto key_len = strnlen(key, str_max_length);
        auto value_len = strnlen(value, str_max_length);

        this->chars.insert(this->chars.end(), key, key + key_len);
        this->chars.insert(this->chars.end(), value, value + value_len);

        this->lengths.emplace_back(key_len);
        this->lengths.emplace_back(value_len);
    }

    void clear()
    {
        this->chars.clear();
        this->lengths.clear();
    }

    DataMap data_map()
    {
        this->views.clear();
        auto p = this->chars.data();
        for (auto len : lengths) {
            this->views.emplace_back(p, p+len);
            p += len;
        }
        return this->views;
    }

private:
    std::vector<char> chars;
    std::vector<uint16_t> lengths;
    std::vector<bytes_view> views;
};


#define CHECK_ERRNUM(validator) do {         \
    if (validator->transport.errnum)         \
    {                                        \
        return -validator->transport.errnum; \
    }                                        \
} while(0)


extern "C"
{

ValidatorKeyValueList* validator_key_value_list_alloc() noexcept
{
    SCOPED_TRACE;
    return new (std::nothrow) ValidatorKeyValueList; /*NOLINT*/
}

void validator_key_value_list_free(ValidatorKeyValueList* key_value_list) noexcept
{
    SCOPED_TRACE;
    delete key_value_list; /*NOLINT*/
}

int validator_key_value_list_clear(ValidatorKeyValueList* key_value_list) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(key_value_list);
    key_value_list->clear();
    return 0;
}

int validator_key_value_list_push_value(ValidatorKeyValueList* key_value_list, char const* key, char const* value) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(key_value_list);
    key_value_list->push(key, value);
    return 0;
}


ValidatorApi* validator_open_session(char const* socket_path) noexcept
{
    SCOPED_TRACE;
    return new (std::nothrow) ValidatorApi(socket_path); /*NOLINT*/
}

ValidatorApi* validator_open_fd_session(int fd) noexcept
{
    SCOPED_TRACE;
    return new (std::nothrow) ValidatorApi(unique_fd(fd)); /*NOLINT*/
}

int validator_close_session(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    validator->file_validator.send_close_session();
    CHECK_ERRNUM(validator);
    delete validator; /*NOLINT*/
    return 0;
}

int validator_get_fd(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    return validator->transport.file.fd();
}

int validator_get_errno(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    return validator->transport.errnum;
}

ValidatorFileId validator_open_file(ValidatorApi* validator, char const* file_name, char const* target_name) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_file(file_name, target_name);
    CHECK_ERRNUM(validator);
    return safe_int(file_id);
}

ValidatorFileId validator_open_data(ValidatorApi* validator, char const* target_name, ValidatorKeyValueList* key_value_list) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_raw_data(
        target_name, key_value_list ? key_value_list->data_map() : DataMap{});
    CHECK_ERRNUM(validator);
    return safe_int(file_id);
}

ValidatorFileId validator_open_data0(ValidatorApi* validator, char const* target_name) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_raw_data(target_name, {});
    CHECK_ERRNUM(validator);
    return safe_int(file_id);
}

ValidatorFileId validator_open_data1(
    ValidatorApi* validator, char const* target_name,
    char const* key1, char const* value1) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_raw_data(
        target_name, to_data_map(key1, value1));
    CHECK_ERRNUM(validator);
    return safe_int(file_id);
}

ValidatorFileId validator_open_data2(
    ValidatorApi* validator, char const* target_name,
    char const* key1, char const* value1,
    char const* key2, char const* value2) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_raw_data(
        target_name, to_data_map(key1, value1, key2, value2));
    CHECK_ERRNUM(validator);
    return safe_int(file_id);
}

ValidatorFileId validator_open_data3(
    ValidatorApi* validator, char const* target_name,
    char const* key1, char const* value1,
    char const* key2, char const* value2,
    char const* key3, char const* value3) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_raw_data(
        target_name, to_data_map(key1, value1, key2, value2, key3, value3));
    CHECK_ERRNUM(validator);
    return safe_int(file_id);
}

int validator_send_data(ValidatorApi* validator, ValidatorFileId id, char const* data, const unsigned size) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    validator->file_validator.send_data(FileValidatorId(id), {data, size});
    CHECK_ERRNUM(validator);
    return 0;
}

int validator_end_of_file(ValidatorApi* validator, ValidatorFileId id) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    validator->file_validator.send_eof(FileValidatorId(id));
    CHECK_ERRNUM(validator);
    return 0;
}

int validator_abort_file(ValidatorApi* validator, ValidatorFileId id) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    validator->file_validator.send_abort(FileValidatorId(id));
    CHECK_ERRNUM(validator);
    return 0;
}

int validator_receive_response(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    validator->response_type = validator->file_validator.receive_response();
    CHECK_ERRNUM(validator);
    return safe_int(validator->response_type);
}

int validator_get_response_type(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    return safe_int(validator->response_type);
}

int validator_get_result_flag(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    return safe_int(validator->file_validator.last_result_flag());
}

char const* validator_get_content(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    return validator ? validator->file_validator.get_content().c_str() : nullptr;
}

ValidatorFileId validator_get_result_file_id(ValidatorApi* validator) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    return safe_int(validator->file_validator.last_file_id());
}

}
