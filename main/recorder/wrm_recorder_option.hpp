/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__MAIN_RECORDER_WRM_RECORDER_OPTION__)
#define __MAIN_RECORDER_WRM_RECORDER_OPTION__
#include "version.hpp"

#include <vector>
#include <string>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "relative_time_point.hpp"
#include "range_time_point.hpp"
#include "cipher.hpp"
#include "validate.hpp"

struct InputType {
    enum enum_t {
        NOT_FOUND,
        META_TYPE,
        WRM_TYPE
    };

    typedef enum_t format_type;

    static InputType::enum_t string_to_type(const std::string& s)
    {
        if (s == "mwrm")
            return META_TYPE;
        if (s == "wrm")
            return WRM_TYPE;
        return NOT_FOUND;
    }
};


struct WrmRecorderOption
{
    boost::program_options::options_description desc;
    boost::program_options::variables_map options;

public:
    range_time_point range;
    uint frame;
    time_point time;
    std::vector<relative_time_point> time_list;
    std::string in_filename;
    uint idx_start;
    std::string base_path;
    std::string metaname;
    bool ignore_dir_for_meta_in_wrm;
    std::string input_type;
    bool times_in_meta_are_false;
    const EVP_CIPHER* in_crypt_mode;
    HexadecimalKeyOption in_crypt_key;
    HexadecimalIVOption in_crypt_iv;
    CipherInfo in_cipher_info;
    bool force_interpret_breakpoint;

    WrmRecorderOption();

    /*virtual*/ void parse_command_line(int argc, char** argv);

    /**
     * Return 0 if success.
     * @{
     */
    virtual int notify_options();
    virtual int normalize_options();
    //@}

    virtual const char * version() const
    {
        return VERSION;
    };

    enum Error {
        SUCCESS,
        IN_FILENAME_IS_EMPTY,
        UNSPECIFIED_DECRIPT_KEY,
        INPUT_KEY_OVERLOAD,
        INPUT_IV_OVERLOAD
    };

    virtual const char * get_cerror(int error)
    {
        if (error == IN_FILENAME_IS_EMPTY)
            return "Not input-file";
        if (error == UNSPECIFIED_DECRIPT_KEY)
            return "Unspecified decript key";
        if (error == INPUT_KEY_OVERLOAD)
            return "Overload --in-crypt-key";
        if (error == INPUT_IV_OVERLOAD)
            return "Overload --in-crypt-iv";
        if (error == SUCCESS)
            return "Success";
        return "Unknow";
    }

    /**
     * Normalize option and display error
     * @param[out] itype
     * Return 0 if success
     */
    int prepare(InputType::enum_t& itype);

private:
    void add_default_options();
};

static inline InputType::enum_t get_input_type(const WrmRecorderOption& opt)
{
    if (!opt.input_type.empty()){
        return InputType::string_to_type(opt.input_type);
    }
    const std::size_t pos = opt.in_filename.find_last_of('.');
    return InputType::string_to_type(opt.in_filename.substr(pos + 1));
}

struct RecorderOption
: WrmRecorderOption
{
    std::string out_filename;
    std::string output_type;
    bool screenshot_wrm;
    bool screenshot_start;
    bool no_screenshot_stop;
    bool screenshot_all;
    bool cat_wrm;
    unsigned png_scale_width;
    unsigned png_scale_height;
    CipherMode::enum_t out_crypt_mode;
    HexadecimalKeyOption out_crypt_key;
    HexadecimalIVOption out_crypt_iv;
    CipherInfo out_cipher_info;

    RecorderOption();

    template<typename _ForwardIterator>
    void accept_output_type(_ForwardIterator first, _ForwardIterator last)
    {
        if (first == last){
            throw std::runtime_error("output type is empty");
        }
        std::string output_type_desc = "accept ";
        for (; first != last; ++first){
            output_type_desc += '\'';
            output_type_desc += *first;
            output_type_desc += "', ";
        }
        output_type_desc.erase(output_type_desc.size() - 2);
        std::size_t pos = output_type_desc.find_last_of(',');
        if (pos != std::string::npos){
            output_type_desc[pos] = ' ';
            output_type_desc.insert(pos + 1, "or");
        }
        this->add_output_type(output_type_desc);
    }

    /**
     * Return 0 if success.
     * @{
     */
    virtual int notify_options();
    virtual int normalize_options();
    //@}

    virtual const char * version() const
    {
        return VERSION;
    };

    enum Error {
        SUCCESS                 = WrmRecorderOption::SUCCESS,
        IN_FILENAME_IS_EMPTY    = WrmRecorderOption::IN_FILENAME_IS_EMPTY,
        UNSPECIFIED_DECRIPT_KEY = WrmRecorderOption::UNSPECIFIED_DECRIPT_KEY,
        INPUT_KEY_OVERLOAD      = WrmRecorderOption::INPUT_KEY_OVERLOAD,
        INPUT_IV_OVERLOAD       = WrmRecorderOption::INPUT_IV_OVERLOAD,
        UNSPECIFIED_ENCRIPT_KEY,
        ENCRIPT_KEY_OR_IV_WITHOUT_MODE,
        OUT_FILENAME_IS_EMPTY,
        OUTPUT_KEY_OVERLOAD,
        OUTPUT_IV_OVERLOAD
    };

    virtual const char * get_cerror(int error)
    {
        if (error == OUT_FILENAME_IS_EMPTY)
            return "Not output-file";
        if (error == ENCRIPT_KEY_OR_IV_WITHOUT_MODE)
            return "Set --out-crypt-key or --out-crypt-iv without --out-crypt-mode";
        if (error == OUTPUT_KEY_OVERLOAD)
            return "Overload --out-crypt-key";
        if (error == OUTPUT_IV_OVERLOAD)
            return "Overload --out-crypt-iv";
        return WrmRecorderOption::get_cerror(error);
    }

private:
    void add_default_options();
    void add_output_type(const std::string& desc);
};

#endif
