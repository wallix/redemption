#include <boost/program_options/parsers.hpp>

#include "wrm_recorder_option.hpp"

namespace po = boost::program_options;

void validate(boost::any& v,
              const std::vector<std::string>& values,
              range_time_point* range, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = boost::program_options::validators::get_single_string(values);
    v = boost::any(range_time_point(s));
}

void validate(boost::any& v,
              const std::vector<std::string>& values,
              time_point* time, int)
{
    // Make sure no previous assignment to 'a' was made.
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    v = boost::any(time_point(s));
}

WrmRecorderOption::WrmRecorderOption()
: desc("Options")
, options()
, range()
, frame(std::numeric_limits<uint>::max())
, time(60*2)
, out_filename()
, in_filename()
, idx_start("0")
, base_path()
, screenshot_wrm(false)
, screenshot_start(false)
, no_screenshot_stop(false)
, ignore_dir_for_meta_in_wrm(false)
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
     "\n[[[+|-]time[h|m|s][...]][,][[+]time[h|m|s][[+|-]time[h|m|s][...]]]]"
     "\n\nexamples:"
     "\n1h30,+10m -> from 1h30 to 1h40"
     "\n20m+2h-50s,3h -> from 2h19m10s to 3h")
    ("frame,f", po::value(&this->frame), "maximum number of frames in the interval")
    ("time,t", po::value(&this->time), "duration between each capture"
    "\nformat: [+|-]time[h|m|s][...]")
    ("input-file,i", po::value(&this->in_filename), "wrm filename")
    ("output-file,o", po::value(&this->out_filename), "png or wrm filename")
    ("index-start,x", po::value(&this->idx_start), "index file in the meta")
    ("screenshot-wrm,s", "capture the screen when a file wrm is create")
    ("screenshot-start,0", "")
    ("no-screenshot-stop,N", "")
    ("path,p", po::value(&this->base_path), "base path for the files presents in the meta")
    ("ignore-dir,n", "ignore directory for meta in the file wrm")
    ("deduce-dir,d", "use --ignore-dir and set --path with the directory of --input-file")
    ("output-meta-name,m", po::value(&this->metaname), "specified name of meta file")
    ("input-type", po::value(&this->input_type), "accept 'mwrm' or 'wrm'")
    ("output-type", po::value(&this->output_type), "accept 'png' or 'wrm'")
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

    if (this->out_filename.empty()){
        return OUT_FILENAME_IS_EMPTY;
    }
    if (this->in_filename.empty()){
        return IN_FILENAME_IS_EMPTY;
    }

    if (!this->range.valid()){
        std::swap<>(this->range.left, this->range.right);
    }

    return SUCCESS;
}

int WrmRecorderOption::normalize_options()
{
    po::variables_map::iterator end = this->options.end();

    {
        typedef std::pair<const char *, bool&> pair_type;
        pair_type p[] = {
            pair_type("screenshot-wrm", this->screenshot_wrm),
            pair_type("screenshot-start", this->screenshot_start),
            pair_type("no-screenshot-stop", this->no_screenshot_stop),
            pair_type("ignore-dir", this->ignore_dir_for_meta_in_wrm),
        };
        for (std::size_t n = 0; n < sizeof(p)/sizeof(p[0]); ++n) {
            if (this->options.find(p[n].first) != end)
                p[n].second = true;
        }
    }

    if (this->options.find("deduce-dir") != end)
    {
        this->ignore_dir_for_meta_in_wrm = true;
        std::size_t pos = this->in_filename.find_last_of('/');
        if (std::string::npos != pos)
            this->base_path = this->in_filename.substr(0, pos+1);
    }

    return SUCCESS;
}
