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
} // anonymous namespace


struct ValidatorApi
{
    ValidatorApi(unique_fd ufd) noexcept
    : transport(std::move(ufd))
    {}

    ValidatorApi(char const* path)
    : ValidatorApi(addr_connect(path))
    {}

    ValidatorTransport transport;
    FileValidatorService file_validator{transport};
    bool wating_data = false;
    FileValidatorService::ResponseType response_type = FileValidatorService::ResponseType::WaitingData;
};


#define CHECK_ERRNUM(validator)              \
    if (validator->transport.errnum)         \
    {                                        \
        return -validator->transport.errnum; \
    }


extern "C"
{

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

ValidatorFileId validator_open_text(ValidatorApi* validator, uint32_t locale_identifier, char const* target_name) noexcept
{
    SCOPED_TRACE;
    CHECK_HANDLE(validator);
    auto file_id = validator->file_validator.open_text(locale_identifier, target_name);
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
