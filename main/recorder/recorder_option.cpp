#include <boost/program_options/parsers.hpp>

#include "recorder_option.hpp"

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
    int err = WrmRecorderOption::notify_options();
    if (err)
        return err;
    if (this->out_filename.empty()){
        return OUT_FILENAME_IS_EMPTY;
    }
    return SUCCESS;
}

int RecorderOption::normalize_options()
{
    int err = WrmRecorderOption::normalize_options();
    if (err)
        return err;

    po::variables_map::iterator end = this->options.end();

    {
        typedef std::pair<const char *, bool&> pair_type;
        pair_type p[] = {
            pair_type("screenshot-wrm", this->screenshot_wrm),
            pair_type("screenshot-start", this->screenshot_start),
            pair_type("no-screenshot-stop", this->no_screenshot_stop),
            pair_type("screenshot-all", this->screenshot_all),
            pair_type("concat-wrm", this->cat_wrm),
        };
        for (std::size_t n = 0; n < sizeof(p)/sizeof(p[0]); ++n) {
            if (this->options.find(p[n].first) != end)
                p[n].second = true;
        }
    }

    return SUCCESS;
}
