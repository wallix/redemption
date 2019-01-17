#include <mln/image/image2d.hh>
#include <mln/io/ppm/load.hh>

#include <iostream>
#include <chrono>
#include <cstring>
#include <cerrno>

#include "capture/ocr/rgb8.hpp"
#include "capture/ocr/extract_bars.hh"
#include "capture/ocr/image2dview.hpp"
#include "capture/rdp_ppocr/extract_text.hpp"
#include "capture/rdp_ppocr/get_ocr_constants.hpp"
#include "capture/ocr/locale/latin_to_cyrillic.hpp"


using ImageView = ocr::Image2dView<ocr::rgb8>;

using resolution_clock = std::chrono::high_resolution_clock;

struct Classification
{
    rdp_ppocr::OcrDatasConstant const & ocr_constant;
    rdp_ppocr::OcrContext ocr_context;
    ocr::fonts::LocaleId locale_id;

    Classification(std::string const & directory, ocr::fonts::LocaleId locale_id)
    : ocr_constant(rdp_ppocr::get_ocr_constants(directory))
    , ocr_context{this->ocr_constant.glyphs.size()}
    , locale_id(locale_id)
    {}

    void operator()(ImageView const & input, unsigned tid, mln::box2d const & box, unsigned button_col)
    {
        struct PtrImageView {
            unsigned char * drawable;
            unsigned w;
            unsigned h;

            unsigned width() const
            { return this->w; }

            unsigned height() const
            { return this->h; }

            struct Color
            {
                const uint8_t * c;

                uint8_t blue()  const { return c[0]; }
                uint8_t green() const { return c[1]; }
                uint8_t red()   const { return c[2]; }
            };

            Color operator()(unsigned row, unsigned col) const
            { return {this->drawable + (this->w * row + col) * 3}; }

            Color operator[](::mln::point2d p) const
            { return this->operator()(p.row(), p.col()); }
        };
        std::unique_ptr<unsigned char[]> ptr_data(new unsigned char[input.width() * input.height() * 3]);
        auto t1 = resolution_clock::now();
        {
            unsigned char * p = ptr_data.get();
            for (unsigned row = 0; row < input.height(); ++row) {
                for (unsigned col = 0; col < input.width(); ++col) {
                    auto const & color = input(row, col);
                    *p++ = color.blue();
                    *p++ = color.green();
                    *p++ = color.red();
                }
            }
        }
        auto t2 = resolution_clock::now();
        std::cerr << "\n" << std::chrono::duration<double>(t2-t1).count() << "s (copy)\n";

        unsigned const unrecognized_count = rdp_ppocr::extract_text(
            this->ocr_constant,
            this->ocr_context,
            PtrImageView{ptr_data.get(), input.width(), input.height()}, tid, box, button_col
        );
        unsigned const unrecognized_rate =
            this->ocr_context.ambiguous.empty()
          ? 0
          : unrecognized_count * 100 / this->ocr_context.ambiguous.size();
        auto & result = this->ocr_context.result;
        if (!result.empty()) {
            if (this->locale_id == ::ocr::locale::LocaleId::cyrillic) {
                ocr::locale::latin_to_cyrillic_context().latin_to_cyrillic(result);
            }
        }

        const bool is_title_bar = ocr::is_title_bar(
            input, tid, box.min_row(), box.max_row(), button_col
          , ::ocr::bbox_max_height
        );
        std::cout
            << (&"other\0title"[is_title_bar*6])
            << ": " << result
            << "\nlocale: " << &"latin\0cyrillic"[this->locale_id * 6]
            << "\n   length: " << this->ocr_context.ambiguous.size()
            << "  unrecognized: " << unrecognized_count
            << "  unrecognized rate: " << unrecognized_rate
            << "\n"
        ;
        //for (ppocr::ocr2::view_ref_list const & view_list : this->ocr_context.ambiguous) {
        //    for (ppocr::ocr2::View const & view : view_list) {
        //        std::cout << "  " << this->ocr_constant.glyphs.get_word(view);
        //    }
        //    std::cout << "\n";
        //}
    }
};

struct ReferenceClassification
{
    Classification & ref;

    ReferenceClassification(Classification & ref_)
    : ref(ref_)
    {}

    void operator()(ImageView const & input, unsigned tid, mln::box2d const & box, unsigned button_col)
    {
        this->ref(input, tid, box, button_col);
    }
};


inline void usage(char** argv)
{
    std::cerr
      << "Usage: " << argv[0] << " input.ppm/pnm directory_ressources [latin|cyrillic]\n"
      "  d - display characters" << std::endl
    ;
}

int main(int argc, char** argv)
{
    if (argc < 3 || argc > 4) {
        usage(argv);
        return 1;
    }

    ocr::fonts::LocaleId locale_id = ocr::fonts::LocaleId::latin;
    char const * dir = argv[2];

    if (argc == 4) {
        if (strcmp(argv[3], "latin") == 0) {
            locale_id = ocr::fonts::LocaleId::latin;
        }
        else if (strcmp(argv[3], "cyrillic") == 0) {
            locale_id = ocr::fonts::LocaleId::cyrillic;
        }
        else {
            usage(argv);
            return 3;
        }
    }

    mln::image2d<ocr::rgb8> input;
    if (!mln::io::ppm::load(input, argv[1])) {
        if (errno != 0) {
            std::cerr << argv[0] << ": " << strerror(errno) << std::endl;
        }
        return 4;
    }

    ocr::ExtractTitles extract_titles;

    auto t1 = resolution_clock::now();
    Classification classification(dir, locale_id);
    auto t2 = resolution_clock::now();
    std::cerr << "load: " << std::chrono::duration<double>(t2-t1).count() << "s\n\n";
    t1 = resolution_clock::now();
    extract_titles.extract_titles(ImageView(input), ReferenceClassification(classification));
    t2 = resolution_clock::now();
    std::cerr << std::chrono::duration<double>(t2-t1).count() << "s\n";

    return 0;
}
