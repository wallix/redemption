#include <iostream>
#include <stdexcept>

#include <boost/program_options/parsers.hpp>

#include "wrm_recorder_option.hpp"
#include "get_type.hpp"
#include "validate.hpp"

namespace po = boost::program_options;

void validate(boost::any& v,
              const std::vector<std::string>& values,
              range_time_point* /*range*/, int)
{
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    v = boost::any(range_time_point(s));
}

void validate(boost::any& v,
              const std::vector<std::string>& values,
              time_point* /*time*/, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    v = boost::any(time_point(s));
}

template<std::size_t _N>
void validate(boost::any& v,
              const std::vector<std::string>& values,
              HexadecimalOption<_N>* /*binary_string*/, int)
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

void validate(boost::any& v,
              const std::vector<std::string>& values,
              CipherMode::enum_t* /*mode*/, int)
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

    v = boost::any(mode);
}

WrmRecorderOption::WrmRecorderOption()
: desc("Options")
, options()
, range()
, frame(std::numeric_limits<uint>::max())
, time(60*2)
, in_filename()
, idx_start(0)
, base_path()
, metaname()
, ignore_dir_for_meta_in_wrm(false)
, input_type()
, times_in_meta_are_false(false)
, in_crypt_mode(CipherMode::NO_MODE)
, in_crypt_key()
, in_crypt_iv()
{
    this->add_default_options();
}

void WrmRecorderOption::add_default_options()
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
    ("input-file,i", po::value(&this->in_filename), "input filename (see --input-type)")
    ("in,i", po::value(&this->in_filename), "alias for --input-file")
    ("index-start,x", po::value(&this->idx_start), "index file in the meta")
    ("path,p", po::value(&this->base_path), "base path for the files presents in the meta")
    ("ignore-dir,N", "ignore directory for meta in the wrm file")
    ("deduce-dir,d", "use --ignore-dir and set --path with the directory of --input-file")
    ("times-in-meta-file-are-false", "")
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

void WrmRecorderOption::parse_command_line(int argc, char** argv)
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

int WrmRecorderOption::notify_options()
{
    po::notify(this->options);

    if (this->in_filename.empty()){
        return IN_FILENAME_IS_EMPTY;
    }

    if (this->in_crypt_mode && !this->in_crypt_key.size){
        return UNSPECIFIED_DECRIPT_KEY;
    }

    return SUCCESS;
}

int WrmRecorderOption::normalize_options()
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

    return SUCCESS;
}

int WrmRecorderOption::prepare(InputType::enum_t& itype)
{
    int error = this->notify_options();
    if (error){
        std::cerr << this->get_cerror(error) << '\n'<< this->desc;
        return error;
    }

    itype = get_input_type(*this);
    if (itype == InputType::NOT_FOUND){
        std::cerr
        << "Incorrect input-type, "
        << this->desc.find("input-type", false).description() << '\n';
        return 1000;
    }

    error = this->normalize_options();
    if (error){
        std::cerr << this->get_cerror(error) << std::endl;
        return error;
    }

    return 0;
}
