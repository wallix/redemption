#include <iostream>
#include <stdexcept>

#include <boost/program_options/parsers.hpp>

#include "wrm_recorder_option.hpp"

namespace po = boost::program_options;

WrmRecorderOption::WrmRecorderOption()
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
                        if ('+' == first->symbol)
                            first->point += prev->point;
                        else
                            first->point = prev->point - first->point;
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

int WrmRecorderOption::prepare(InputType::enum_t& itype)
{
    int error = this->notify_options();
    if (error){
        std::cerr << this->get_cerror(error) << std::endl;
        return error;
    }

    itype = get_input_type(*this);
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

namespace po = boost::program_options;

RecorderOption::RecorderOption()
: WrmRecorderOption()
, out_filename()
, output_type()
, screenshot_wrm(false)
, screenshot_start(false)
, no_screenshot_stop(false)
, screenshot_all(false)
, cat_wrm(false)
, png_scale_width(-1)
, png_scale_height(-1)
, out_crypt_mode(CipherMode::NO_MODE)
, out_crypt_key()
, out_crypt_iv()
, out_cipher_info()
{
    this->add_default_options();
}

void RecorderOption::add_default_options()
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

void RecorderOption::add_output_type(const std::string& desc)
{
    this->desc.add_options()
    ("output-type,O", po::value(&this->output_type), desc.c_str())
    ;
}

int RecorderOption::notify_options()
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

int RecorderOption::normalize_options()
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
}
