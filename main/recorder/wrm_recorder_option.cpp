#include <iostream>

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

    else if (s == "des")
        mode = CipherMode::DES_CBC;
    else if (s == "des-cbc")
        mode = CipherMode::DES_CBC;
    else if (s == "des-ecb")
        mode = CipherMode::DES_ECB;
    else if (s == "des-ede")
        mode = CipherMode::DES_EDE;

    else if (s == "rc2")
        mode = CipherMode::RC2_CBC;
    else if (s == "rc2-cbc")
        mode = CipherMode::RC2_CBC;
    else if (s == "rc2-ecb")
        mode = CipherMode::RC2_ECB;

    else if (s == "rc4")
        mode = CipherMode::RC4;
    else if (s == "rc4-40")
        mode = CipherMode::RC4_40;

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
, in_cipher_mode(CipherMode::NO_MODE)
, in_cipher_key()
, in_cipher_iv()
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
    ("in-cipher-key", po::value(&this->in_cipher_key), "")
    ("in-cipher-iv", po::value(&this->in_cipher_iv), "")
    ("in-cipher-mode", po::value(&this->in_cipher_mode), "")
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

    if ((!this->in_cipher_iv.empty() || !this->in_cipher_key.empty())
        && !this->in_cipher_mode)
        return KEY_OR_IV_WITHOUT_MODE;

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
