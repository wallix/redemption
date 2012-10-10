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
#include <openssl/evp.h>

#include <iostream>
#include <stdexcept>
#include <vector>
#include <string>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>

#include <error.hpp>

#include "wrm_recorder.hpp"
#include "relative_time_point.hpp"
#include "range_time_point.hpp"
#include "cipher.hpp"

template<std::size_t N>
struct HexadecimalOption
{
    unsigned char data[N];
    std::size_t size;

    HexadecimalOption()
    : size(0)
    {}

    /**
     * \param s value in hexadecimal base
     */
    bool parse(const std::string& s)
    {
        std::size_t n = s.size() / 2 + (s.size() & 1);
        if (n > N || !transform_string_hex_to_data(s, this->data))
            return false;
        this->size = n;
        while (n != N)
            this->data[n++] = 0;
        return true;
    }

private:
    static bool transform_string_hex_to_data(const std::string& s,
                                             unsigned char * pdata)
    {
        std::string::const_iterator first = s.begin();
        std::string::const_iterator last = s.end();
        char c;
        if (s.size() & 1)
            --last;
        for (; first != last; ++first, ++pdata)
        {
            if (0xf == (*pdata = transform_c_hex_to_c_data(*first)))
                return false;
            if (0xf == (c = transform_c_hex_to_c_data(*++first)))
                return false;
            *pdata = (*pdata << 4) + c;
        }
        if (s.size() & 1)
        {
            if (0xf == (*pdata = transform_c_hex_to_c_data(*first)))
                return false;
            *pdata <<= 4;
        }
        return true;
    }

    static unsigned char transform_c_hex_to_c_data(char c)
    {
        if ('a' <= c && c <= 'f')
            return c - 'a' + 0xa;
        if ('A' <= c && c <= 'F')
            return c - 'A' + 0xa;
        if ('0' > c || c > '9')
            return 0xf;
        return c - '0';
    }
};

typedef HexadecimalOption<EVP_MAX_KEY_LENGTH> HexadecimalKeyOption;
typedef HexadecimalOption<EVP_MAX_IV_LENGTH> HexadecimalIVOption;

namespace po = boost::program_options;

template<typename _T>
void validate_time_or_throw_invalid_option_value(boost::any& v,
                                                 const std::vector<std::string>& values,
                                                 _T*)
{
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    try
    {
        v = boost::any(_T(s));
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

template<typename _T>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              basic_relative_point<_T>* target_type, int)
{
    validate_time_or_throw_invalid_option_value(v, values, target_type);
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     range_time_point* target_type, int)
{
    validate_time_or_throw_invalid_option_value(v, values, target_type);
}

inline void validate(boost::any& v,
                     const std::vector<std::string>& values,
                     time_point* target_type, int)
{
    validate_time_or_throw_invalid_option_value(v, values, target_type);
}

template<std::size_t _N>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              HexadecimalOption<_N>*, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    HexadecimalOption<_N> bs;
    if (!bs.parse(s))
        throw po::validation_error(po::validation_error::invalid_option_value);
    v = boost::any(bs);
}


inline CipherMode::enum_t get_enum_mode(boost::any& v,
                                        const std::vector<std::string>& values)
{
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);

    CipherMode::enum_t mode = CipherMode::NO_MODE;

    if (s == "bf")
        mode = CipherMode::BLOWFISH_CBC;
    else if (s == "bf-cbc")
        mode = CipherMode::BLOWFISH_CBC;
    else if (s == "bf-ecb")
        mode = CipherMode::BLOWFISH_ECB;
    else if (s == "bf-ofb")
        mode = CipherMode::BLOWFISH_OFB;

    else if (s == "cast5")
        mode = CipherMode::CAST5_CBC;
    else if (s == "cast5-cbc")
        mode = CipherMode::CAST5_CBC;
    else if (s == "cast5-cfb")
        mode = CipherMode::CAST5_CFB;
    else if (s == "cast5-ofb")
        mode = CipherMode::CAST5_OFB;
    else if (s == "cast5-ecb")
        mode = CipherMode::CAST5_ECB;

    else if (s == "des")
        mode = CipherMode::DES_CBC;
    else if (s == "des-cbc")
        mode = CipherMode::DES_CBC;
    else if (s == "des-cfb")
        mode = CipherMode::DES_CFB;
    else if (s == "des-ofb")
        mode = CipherMode::DES_OFB;
    else if (s == "des-ecb")
        mode = CipherMode::DES_ECB;

    else if (s == "des3")
        mode = CipherMode::DES_EDE3_CBC;
    else if (s == "des3-ecb")
        mode = CipherMode::DES_EDE3_ECB;
    else if (s == "des-ede3-cbc")
        mode = CipherMode::DES_EDE3_CBC;
    else if (s == "des-ede3-ecb")
        mode = CipherMode::DES_EDE3_ECB;
    else if (s == "des-ede3-cfb-1")
        mode = CipherMode::DES_EDE3_CFB_1;
    else if (s == "des-ede3-cfb-8")
        mode = CipherMode::DES_EDE3_CFB_8;
    else if (s == "des-ede3-cfb-64")
        mode = CipherMode::DES_EDE3_CFB_64;
    else if (s == "des-ede3-ofb")
        mode = CipherMode::DES_EDE3_OFB;

    else if (s == "rc2")
        mode = CipherMode::RC2_CBC;
    else if (s == "rc2-cbc")
        mode = CipherMode::RC2_CBC;
    else if (s == "rc2-cfb")
        mode = CipherMode::RC2_CFB;
    else if (s == "rc2-ecb")
        mode = CipherMode::RC2_ECB;
    else if (s == "rc2-ofb")
        mode = CipherMode::RC2_OFB;
    else if (s == "rc2-64-cbc")
        mode = CipherMode::RC2_64_CBC;
    else if (s == "rc2-40-cbc")
        mode = CipherMode::RC2_40_CBC;

    else if (s == "rc4")
        mode = CipherMode::RC4;
    else if (s == "rc4-40")
        mode = CipherMode::RC4_40;

    else if (s == "aes-128")
        mode = CipherMode::AES_128_CBC;
    else if (s == "aes-128-cbc")
        mode = CipherMode::AES_128_CBC;
    else if (s == "aes-128-cfb")
        mode = CipherMode::AES_128_CFB;
    else if (s == "aes-128-cfb1")
        mode = CipherMode::AES_128_CFB1;
    else if (s == "aes-128-cfb8")
        mode = CipherMode::AES_128_CFB8;
    else if (s == "aes-128-ecb")
        mode = CipherMode::AES_128_ECB;
    else if (s == "aes-128-ofb")
        mode = CipherMode::AES_128_OFB;

    else if (s == "aes-192")
        mode = CipherMode::AES_192_CBC;
    else if (s == "aes-192-cbc")
        mode = CipherMode::AES_192_CBC;
    else if (s == "aes-192-cfb")
        mode = CipherMode::AES_192_CFB;
    else if (s == "aes-192-cfb1")
        mode = CipherMode::AES_192_CFB1;
    else if (s == "aes-192-cfb8")
        mode = CipherMode::AES_192_CFB8;
    else if (s == "aes-192-ecb")
        mode = CipherMode::AES_192_ECB;
    else if (s == "aes-192-ofb")
        mode = CipherMode::AES_192_OFB;

    else if (s == "aes-256")
        mode = CipherMode::AES_256_CBC;
    else if (s == "aes-256-cbc")
        mode = CipherMode::AES_256_CBC;
    else if (s == "aes-256-cfb")
        mode = CipherMode::AES_256_CFB;
    else if (s == "aes-256-cfb1")
        mode = CipherMode::AES_256_CFB1;
    else if (s == "aes-256-cfb8")
        mode = CipherMode::AES_256_CFB8;
    else if (s == "aes-256-ecb")
        mode = CipherMode::AES_256_ECB;
    else if (s == "aes-256-ofb")
        mode = CipherMode::AES_256_OFB;

    if (!mode)
        throw po::validation_error(po::validation_error::invalid_option_value);

    return mode;
}

inline void validate(boost::any& v,
              const std::vector<std::string>& values,
              CipherMode::enum_t* /*mode*/, int)
{
    v = boost::any(get_enum_mode(v, values));
}

inline void validate(boost::any& v,
              const std::vector<std::string>& values,
              const EVP_CIPHER** /*mode*/, int)
{
    v = boost::any(CipherMode::to_evp_cipher(get_enum_mode(v, values)));
}

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

    WrmRecorderOption()
    : desc("Options")
    , options()
    , range()
    , frame(std::numeric_limits<uint>::max())
    , time(60*2)
    , time_list()
    , in_filename()
    , idx_start(0)
    , base_path()
    , metaname()
    , ignore_dir_for_meta_in_wrm(false)
    , input_type()
    , times_in_meta_are_false(false)
    , in_crypt_mode(0)
    , in_crypt_key()
    , in_crypt_iv()
    , in_cipher_info()
    , force_interpret_breakpoint(false)
    {
        this->add_default_options();
    }

    void parse_command_line(int argc, char** argv)
    {
        po::positional_options_description p;
        p.add("input-file", -1);
        po::store(
            po::command_line_parser(argc, argv).options(
                this->desc
            ).positional(p).run(),
            this->options
        );
    }

    /**
     * Return 0 if success.
     * @{
     */
    virtual int notify_options()
    {
        po::notify(this->options);

        if (this->in_filename.empty()){
            return IN_FILENAME_IS_EMPTY;
        }

        if (this->in_crypt_mode)
        {
            if (!this->in_crypt_key.size)
                return UNSPECIFIED_DECRIPT_KEY;
            this->in_cipher_info.set_context(this->in_crypt_mode);
            if (this->in_crypt_key.size > this->in_cipher_info.key_len())
                return INPUT_KEY_OVERLOAD;
            if (this->in_crypt_iv.size > this->in_cipher_info.iv_len())
                return INPUT_IV_OVERLOAD;
        }

        return SUCCESS;
    }

    virtual int normalize_options()
    {
        if (!this->range.valid()){
            std::swap<>(this->range.left, this->range.right);
        }

        po::variables_map::iterator end = this->options.end();

        if (this->options.find("ignore-dir") != end)
            this->ignore_dir_for_meta_in_wrm = true;

        if (this->options.find("times-in-meta-file-are-false") != end)
            this->times_in_meta_are_false = true;

        if (this->options.find("deduce-dir") != end)
        {
            this->ignore_dir_for_meta_in_wrm = true;
            std::size_t pos = this->in_filename.find_last_of('/');
            if (std::string::npos != pos)
                this->base_path = this->in_filename.substr(0, pos+1);
        }

        if (this->options.find("time-list") != end)
        {
            typedef std::vector<relative_time_point>::iterator iterator;
            if (this->time_list.size() >= 1)
            {
                iterator first = this->time_list.begin();
                if ('-' == first->symbol)
                    first->point.time = -first->point.time;
                first->symbol = 0;

                if (this->time_list.size() > 1)
                {
                    for (iterator prev = first++, last = this->time_list.end();
                         first != last; ++first, ++prev)
                    {
                        if (first->symbol)
                        {
                            if ('+' == first->symbol){
                                first->point += prev->point;
                            }
                            else {
                                first->point = prev->point - first->point;
                            }
                            first->symbol = 0;
                        }
                    }
                }
            }
            std::sort<>(this->time_list.begin(), this->time_list.end(),
                        relative_time_point_less_only_point());
        }

        return SUCCESS;
    }

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

    InputType::enum_t get_input_type()
    {
        if (!this->input_type.empty()){
            return InputType::string_to_type(this->input_type);
        }
        const std::size_t pos = this->in_filename.find_last_of('.');
        return InputType::string_to_type(this->in_filename.substr(pos + 1));
    }

    /**
     * Normalize option and display error
     * @param[out] itype
     * Return 0 if success
     */
    int prepare(InputType::enum_t& itype)
    {
        int error = this->notify_options();
        if (error){
            std::cerr << this->get_cerror(error) << std::endl;
            return error;
        }

        itype = this->get_input_type();
        if (itype == InputType::NOT_FOUND){
            std::cerr
            << "Incorrect input-type, "
            << this->desc.find("input-type", false).description() << std::endl;
            return 1000;
        }

        error = this->normalize_options();
        if (error){
            std::cerr << this->get_cerror(error) << std::endl;
            return error;
        }

        return 0;
    }

private:
    void add_default_options()
    {
        this->desc.add_options()
        // --help, -h
        ("help,h", "produce help message")
        // --version, -v
        ("version,v", "show software version")
        ("range,r", po::value(&this->range),
         "interval of capture"
         "\n\nformat:"
         "\n[[+|-]time[h|m|s][...]][,[[+]time[h|m|s][[+|-]time[h|m|s][...]]]]"
         "\n\nexamples:"
         "\n1h30,+10m -> from 1h30 to 1h40"
         "\n20m+2h-50s,3h -> from 2h19m10s to 3h")
        ("frame,f", po::value(&this->frame), "maximum number of frames in the interval")
        ("time,t", po::value(&this->time), "duration between each capture"
        "\nformat: [+|-]time[h|m|s][...]")
        ("time-list,l", po::value(&this->time_list)->multitoken(), "points of capture. Set --output-type with 'png.list' if not done"
        "\nformat: [+|-]time[h|m|s][...] ...")
        ("input-file,i", po::value(&this->in_filename), "input filename (see --input-type)")
        ("in", po::value(&this->in_filename), "alias for --input-file")
        ("index-start,x", po::value(&this->idx_start), "index file in the meta")
        ("path,p", po::value(&this->base_path), "base path for the files presents in the meta")
        ("ignore-dir,N", "ignore directory for meta in the wrm file")
        ("deduce-dir,d", "use --ignore-dir and set --path with the directory of --input-file")
        ("times-in-meta-file-are-false", "")
        ("force-interpret-breakpoint", po::value(&this->force_interpret_breakpoint), "interpret always breakpoint chunk if exists")
        ("output-meta-name,m", po::value(&this->metaname), "specified name of meta file")
        ("input-type,I", po::value(&this->input_type), "accept 'mwrm' or 'wrm'")
        ("in-crypt-key", po::value(&this->in_crypt_key), "key in hexadecimal base")
        ("in-crypt-iv", po::value(&this->in_crypt_iv), "IV in hexadecimal base")
        ("in-crypt-mode", po::value(&this->in_crypt_mode),
         "bf-cbc        Blowfish in CBC mode\n"
         "bf            Alias for bf-cbc\n"
         "bf-ecb        Blowfish in ECB mode\n"
         "bf-ofb        Blowfish in OFB mode\n"
         "\n"
         "cast-cbc      CAST5 in CBC mode\n"
         "cast          Alias for cast5-cbc\n"
         "cast-cbc      CAST5 in CBC mode\n"
         "cast-cfb      CAST5 in CFB mode\n"
         "cast-ecb      CAST5 in ECB mode\n"
         "cast-ofb      CAST5 in OFB mode\n"
         "\n"
         "des-cbc       DES in CBC mode\n"
         "des           Alias for des-cbc\n"
         "des-cfb       DES in CFB mode\n"
         "des-ofb       DES in OFB mode\n"
         "des-ecb       DES in ECB mode\n"
         "\n"
         "des-ede-cbc        Two key triple DES EDE in CBC mode\n"
         "des-ede            Two key triple DES EDE in ECB mode\n"
         "des-ede-cfb        Two key triple DES EDE in CFB mode\n"
         "des-ede-ofb        Two key triple DES EDE in OFB mode\n"
         "des-ede-ecb        Two key triple DES EDE in ECB mode\n"
         "\n"
         "des-ede3-cbc       Three key triple DES EDE in CBC mode\n"
         "des-ede3-ecb       Three key triple DES EDE in ECB mode\n"
         "des3-ede3          Alias for des-ede3-ecb\n"
         "des3               Alias for des-ede3-cbc\n"
         "des-ede3-cfb-1     Three key triple DES EDE in 1 bit CFB mode\n"
         "des-ede3-cfb-8     Three key triple DES EDE in 8 bit CFB mode\n"
         "des-ede3-cfb-64    Three key triple DES EDE in 64 bit CFB mode\n"
         "des-ede3-ofb       Three key triple DES EDE in OFB mode\n"
         "\n"
         "rc2-cbc       128 bit RC2 in CBC mode\n"
         "rc2           Alias for rc2-cbc\n"
         "rc2-cfb       128 bit RC2 in CFB mode\n"
         "rc2-ecb       128 bit RC2 in ECB mode\n"
         "rc2-ofb       128 bit RC2 in OFB mode\n"
         "rc2-64-cbc    64 bit RC2 in CBC mode\n"
         "rc2-40-cbc    40 bit RC2 in CBC mode\n"
         "\n"
         "rc4           128 bit RC4\n"
         "rc4-40        40 bit RC4\n"
         "\n"
         "aes-[128|192|256]-cbc     128/192/256 bit AES in CBC mode\n"
         "aes-[128|192|256]         Alias for aes-[128|192|256]-cbc\n"
         "aes-[128|192|256]-cfb     128/192/256 bit AES in 128 bit CFB mode\n"
         "aes-[128|192|256]-cfb1    128/192/256 bit AES in 1 bit CFB mode\n"
         "aes-[128|192|256]-cfb8    128/192/256 bit AES in 8 bit CFB mode\n"
         "aes-[128|192|256]-ecb     128/192/256 bit AES in ECB mode\n"
         "aes-[128|192|256]-ofb     128/192/256 bit AES in OFB mode\n")
        ;
    }
};

namespace po = boost::program_options;

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

    RecorderOption()
    : WrmRecorderOption()
    , out_filename()
    , output_type()
    , screenshot_wrm(false)
    , screenshot_start(false)
    , no_screenshot_stop(false)
    , screenshot_all(false)
    , cat_wrm(false)
    , png_scale_width(0)
    , png_scale_height(0)
    , out_crypt_mode(CipherMode::NO_MODE)
    , out_crypt_key()
    , out_crypt_iv()
    , out_cipher_info()
    {
        this->add_default_options();
    }

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
    virtual int notify_options()
    {
        if (int err = WrmRecorderOption::notify_options())
            return err;

        if (this->out_filename.empty()){
            return OUT_FILENAME_IS_EMPTY;
        }

        if ((this->out_crypt_iv.size || this->out_crypt_key.size)
            && !this->out_crypt_mode)
            return ENCRIPT_KEY_OR_IV_WITHOUT_MODE;

        if (this->out_crypt_mode && !this->out_crypt_key.size){
            return UNSPECIFIED_ENCRIPT_KEY;
        }

        if (this->out_crypt_mode)
        {
            if (!this->out_crypt_key.size)
                return UNSPECIFIED_DECRIPT_KEY;
            this->out_cipher_info.set_context(CipherMode::to_evp_cipher(this->out_crypt_mode));
            if (this->out_crypt_key.size > this->out_cipher_info.key_len())
                return OUTPUT_KEY_OVERLOAD;
            if (this->out_crypt_iv.size > this->out_cipher_info.iv_len())
                return OUTPUT_IV_OVERLOAD;
        }

        return SUCCESS;
    }

    virtual int normalize_options()
    {
        if (int err = WrmRecorderOption::normalize_options())
            return err;

        po::variables_map::iterator end = this->options.end();

        {
            typedef std::pair<const char *, bool&> pair_type;
            pair_type p[] = {
                pair_type("screenshot-wrm",     this->screenshot_wrm),
                pair_type("screenshot-start",   this->screenshot_start),
                pair_type("no-screenshot-stop", this->no_screenshot_stop),
                pair_type("screenshot-all",     this->screenshot_all),
                pair_type("concat-wrm",         this->cat_wrm),
            };
            for (std::size_t n = 0; n < sizeof(p)/sizeof(p[0]); ++n) {
                if (this->options.find(p[n].first) != end)
                    p[n].second = true;
            }
        }

        if (this->options.find("time-list") != end && this->options.find("output-type") == end)
        {
            this->output_type = "png.list";
        }

        if (!this->time_list.empty() && this->output_type == "png.list")
        {
            if (this->range.left < this->time_list.front().point)
            {
                this->range.left = std::min(this->time_list.front().point, this->range.right);
            }
        }

        return SUCCESS;
    }    //@}
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

    void add_default_options()
    {
        this->desc.add_options()
        ("output-file,o", po::value(&this->out_filename), "output filename (see --output-type)")
        ("out", po::value(&this->out_filename), "alias for --output-file")
        ("screenshot-wrm,s", "capture the screen when a file wrm is create")
        ("screenshot-start,0", "")
        ("no-screenshot-stop,n", "")
        ("screenshot-all,a", "")
        ("png-scale-width,W", po::value(&this->png_scale_width), "")
        ("png-scale-height,H", po::value(&this->png_scale_height), "")
        ("concat-wrm,c", "concat each wrm in a single wrm")
        ("out-crypt-key", po::value(&this->out_crypt_key), "key in hexadecimal base")
        ("out-crypt-iv", po::value(&this->out_crypt_iv), "IV in hexadecimal base")
        ("out-crypt-mode", po::value(&this->out_crypt_mode), "see --in-crypt-mode")
        ;
    }

    void add_output_type(const std::string& desc)
    {
        this->desc.add_options()
        ("output-type,O", po::value(&this->output_type), desc.c_str())
        ;
    }

};


inline static int _wrm_recorder_init_meta_not_found(WRMRecorder& recorder,
                                                    const char * wrm_filename)
{
    std::cerr << recorder.reader.data_meta << '\n'
    << "Chunk META not found in " << wrm_filename
    << "\n. Chunk is " << recorder.reader.chunk_type << std::endl;
    return 2004;
}

inline static int _wrm_recorder_init_idx_not_found(WRMRecorder& recorder,
                                                   WrmRecorderOption& opt)
{
    std::cerr << "idx " << opt.idx_start << " not found" << std::endl;
    return 2002;
}

inline static void _wrm_recorder_init_set_good_idx(WRMRecorder& recorder,
                                                   WrmRecorderOption& opt)
{
    if (opt.times_in_meta_are_false)
        return ;
    const std::vector<DataFile>& files = recorder.reader.data_meta.files;
    if (!files[0].start_sec)
        return ;
    const timeval tm = {files[0].start_sec, files[0].start_usec};
    uint64_t time = 0;
    for (uint idx = opt.idx_start + 1; idx != files.size(); ++idx)
    {
        const DataFile& data_file = files[idx];
        if (data_file.start_sec)
        {
            timeval tm2 = {data_file.start_sec, data_file.start_usec};
            uint64_t elapsed = difftimeval(tm2, tm) / 1000000;
            if (elapsed > opt.range.left.time)
            {
                opt.range.left.time -= time;
                break;
            }
            time = elapsed;
            opt.idx_start = idx;
        }
    }
}

inline static bool _wrm_recorder_init_init_crypt(WRMRecorder& recorder,
                                                 WrmRecorderOption& opt)
{
    if (opt.in_crypt_key.size)
    {
        if (!opt.in_crypt_iv.size)
        {
            opt.in_crypt_iv.size = sizeof(opt.in_crypt_iv.data);
            memcpy(opt.in_crypt_iv.data,
                   recorder.reader.data_meta.crypt_iv,
                   sizeof(opt.in_crypt_iv.data));
        }
        
        LOG(LOG_INFO, "init_cipher");
        recorder.cipher_mode = (opt.in_crypt_mode ? opt.in_crypt_mode 
                            : CipherMode::to_evp_cipher((CipherMode::enum_t)recorder.reader.data_meta.crypt_mode));
        if (!recorder.cipher_mode){
            return false;
        }
        if (!recorder.cipher_trans.start(recorder.cipher_mode, 
                                         opt.in_crypt_key.data,
                                         opt.in_crypt_iv.size ? opt.in_crypt_iv.data : 0,
                                         0)){
            recorder.cipher_mode = 0;
            std::cerr << "Error in the initialization of the encryption" << std::endl;
            return false;
        }

        recorder.cipher_key = opt.in_crypt_key.data;
        recorder.cipher_iv = opt.in_crypt_iv.size ? opt.in_crypt_iv.data : 0;
        recorder.cipher_impl = 0;
        recorder.reader.trans = &recorder.cipher_trans;
        recorder.trans.diff_size_is_error = false;
    }
    return true;
}


static inline int wrm_recorder_init(WRMRecorder& recorder, WrmRecorderOption& opt, InputType::enum_t itype)
{
    recorder.set_basepath(opt.base_path);
    recorder.only_filename = opt.ignore_dir_for_meta_in_wrm;

    try
    {
        switch (itype) {
            case InputType::WRM_TYPE:
            {
                if (!_wrm_recorder_init_init_crypt(recorder, opt))
                    return 3000;
                const char * filename = opt.in_filename.c_str();
                LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
                int fd = ::open(filename, O_RDONLY);
                if (-1 == fd){
                    LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
                   throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
                }
                recorder.trans.fd = fd;
                if (!recorder.reader.selected_next_order())
                {
                    std::cerr << opt.in_filename << " is invalid wrm file" << std::endl;
                    return 2001;
                }
                if (!recorder.reader.chunk_type == WRMChunk::META_FILE)
                    return _wrm_recorder_init_meta_not_found(recorder, filename);
                    
                char tmp_filename[1024];
                size_t len = recorder.reader.stream.in_uint32_le();
                recorder.reader.stream.in_copy_bytes((uint8_t*)tmp_filename, len);
                tmp_filename[len] = 0;
                --recorder.reader.remaining_order_count;
                
                const char * filename2 = tmp_filename;
                if (recorder.only_filename)
                {
                    const char * tmp = strrchr(filename2 + strlen(filename2), '/');
                    if (tmp){
                        filename2 = tmp+1;
                    }
                }
                if (recorder.base_path_len){
                    recorder.path.erase(recorder.base_path_len);
                    recorder.path += filename2;
                    filename2 = recorder.path.c_str();
                }
                
                if (!recorder.reader.load_data(filename2)){
                    std::cerr << "invalid meta chunck in " << opt.in_filename << std::endl;
                    return 2003;
                }
                if (!recorder.reader.data_meta.files.empty())
                {
                    if (opt.idx_start >= recorder.reader.data_meta.files.size())
                        return _wrm_recorder_init_idx_not_found(recorder, opt);
                    _wrm_recorder_init_set_good_idx(recorder, opt);
                }
                else  if (opt.idx_start >= recorder.reader.data_meta.files.size())
                    return _wrm_recorder_init_idx_not_found(recorder, opt);
                if (opt.idx_start != recorder.idx_file)
                {
                    recorder.next_file(recorder.reader.data_meta.files[recorder.idx_file].wrm_filename.c_str());
                }
            }
            break;
            case InputType::META_TYPE:
            {
                if (!recorder.reader.load_data(opt.in_filename.c_str()))
                {
                    std::cerr << "open " << opt.in_filename << ' ' << strerror(errno) << std::endl;
                    return 2005;
                }
                if (opt.idx_start >= recorder.reader.data_meta.files.size())
                    return _wrm_recorder_init_idx_not_found(recorder, opt);
                _wrm_recorder_init_set_good_idx(recorder, opt);
                const char * filename = recorder.reader.data_meta.files[opt.idx_start].wrm_filename.c_str();
                
                if (recorder.only_filename)
                {
                    const char * tmp = strrchr(filename + strlen(filename), '/');
                    if (tmp){
                        filename = tmp+1;
                    }
                }
                if (recorder.base_path_len){
                    recorder.path.erase(recorder.base_path_len);
                    recorder.path += filename;
                    filename = recorder.path.c_str();
                }
                
                if (!_wrm_recorder_init_init_crypt(recorder, opt)){
                    return 3000;
                }

                LOG(LOG_INFO, "WRMRecorder opening file : %s", filename);
                int fd = ::open(filename, O_RDONLY);
                if (-1 == fd){
                    LOG(LOG_ERR, "Error opening wrm reader file : %s", strerror(errno));
                   throw Error(ERR_WRM_RECORDER_OPEN_FAILED);
                }
                recorder.trans.fd = fd;
                if (recorder.reader.selected_next_order() 
                && recorder.reader.chunk_type == WRMChunk::META_FILE){
                    recorder.reader.stream.p = recorder.reader.stream.end;
                    recorder.reader.remaining_order_count = 0;
                }
                if (!recorder.reader.chunk_type == WRMChunk::META_FILE){
                    return _wrm_recorder_init_meta_not_found(recorder, filename);
                }
            }
            break;
            default:
                std::cerr << "Input type not found" << std::endl;
                return 2000;
        }
        recorder.idx_file = opt.idx_start + 1;
        recorder.force_interpret_breakpoint = opt.force_interpret_breakpoint;
    }
    catch (Error& error)
    {
        std::cerr << "Error " << error.id << ": " << strerror(error.errnum) << std::endl;
        return 100000 + error.errnum;
    }

    return 0;
}


#endif
