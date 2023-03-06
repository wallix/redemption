/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtGui/QIntValidator>

#include "qtclient/profile/widget_profile.hpp"
#include "qtclient/profile/profile_as_cli_options.hpp"
#include "utils/ascii.hpp"
#include "utils/sugar/chars_to_int.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "debug_verbose_description.hpp"

namespace
{

constexpr std::size_t config_description_count_values(char const* s)
{
    std::size_t len = 0;
    for (; *s; ++s) {
        if (*s == '=') {
            ++len;
        }
    }
    return len;
}

template<std::size_t N>
struct ConfigNamePos
{
    static const std::size_t size = N;

    std::array<std::string_view, N> names;
    std::array<uint8_t, N> bit_positions;
};

template<class Box>
constexpr auto config_enum_description_to_array()
{
    // format:
    // - name    = value\n
    // ...
    // - name    = value
    ConfigNamePos<config_description_count_values(Box::string)> result {};
    std::string_view* name_ptr = result.names.data();
    uint8_t* bit_pos_ptr = result.bit_positions.data();
    for (char const* s = Box::string;;) {
        // skip "- "
        s += 2;

        auto end = s + 1;
        while (*end != ' ') {
            ++end;
        }
        *name_ptr++ = {s, end};

        s = end + 1;
        while (*s != '=') {
            ++s;
        }
        // skip "= 0x"
        s += 4;

        auto r = hexadecimal_chars_to_int<uint64_t>(s);
        s = r.ptr;

        uint8_t bitpos = 0;
        while (r.val >>= 1) {
            ++bitpos;
        }
        *bit_pos_ptr++ = bitpos;

        if (!*s) {
            break;
        }
    }

    return result;
}

QString to_qstring(chars_view str)
{
    return QString::fromUtf8(str.data(), checked_int(str.size()));
}

QString printableStr(chars_view str)
{
    auto s = to_qstring(str);
    s.replace(QLatin1Char('-'), QLatin1Char(' '));
    s.data()[0] = QLatin1Char(ascii_to_upper(str[0]));
    return s;
}

struct CheckBox
{
    void reset()
    {
        *value = original_value;
        widget.setChecked(original_value);
    }

    void attach(bool* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        widget.setChecked(original_value);
    }

    void synchronize()
    {
        *value = widget.isChecked();
    }

    QCheckBox widget;

private:
    bool* value = nullptr;
    bool original_value {};
};

template<class T>
struct NumberEdit
{
    NumberEdit()
    {
        widget.setValidator(new QIntValidator(&widget));
    }

    void reset()
    {
        *value = original_value;
        widget.setText(to_qstring(int_to_decimal_chars(original_value)));
    }

    void attach(T* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        widget.setText(to_qstring(int_to_decimal_chars(original_value)));
    }

    void synchronize()
    {
        if constexpr (std::is_unsigned_v<T>) {
            *value = widget.text().toUInt();
        }
        else {
            *value = widget.text().toInt();
        }
    }

    QLineEdit widget;

private:
    T* value = nullptr;
    T original_value {};
};

struct StringEdit
{
    StringEdit()
    {}

    void reset()
    {
        *value = original_value;
        widget.setText(to_qstring(original_value));
    }

    void attach(std::string* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        widget.setText(to_qstring(original_value));
    }

    void synchronize()
    {
        *value = widget.text().toStdString();
    }

    QLineEdit widget;

private:
    std::string* value = nullptr;
    std::string original_value {};
};

struct ProtocolWidget
{
    ProtocolWidget()
    {
        widget.addItem("RDP");
        widget.addItem("VNC");
    }

    void reset()
    {
        *value = original_value;
        widget.setCurrentIndex(int(original_value));
    }

    void attach(qtclient::ProtocolMod* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        widget.setCurrentIndex(int(original_value));
    }

    void synchronize()
    {
        *value = checked_int(widget.currentIndex());
    }

    qtclient::ProtocolMod* value = nullptr;
    qtclient::ProtocolMod original_value {};
    QComboBox widget;
};

template<class T>
struct KbdWidget
{
    KbdWidget()
    {}

    void reset()
    {
        *value = original_value;
        // widget.setText(to_qstring(int_to_decimal_chars(original_value)));
    }

    void attach(KeyLayout::KbdId* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        // widget.setText(to_qstring(int_to_decimal_chars(original_value)));
    }

    void synchronize()
    {
    }

    QLineEdit widget;

private:
    KeyLayout::KbdId* value = nullptr;
    KeyLayout::KbdId original_value {};
};

#ifdef IN_IDE_PARSER
namespace cfg_specs {
    constexpr char const* CONFIG_DESC_RDP = "- name = 0x01";
}
#endif
struct RDPVerboseConfigString
{
    constexpr static const char* string = cfg_specs::CONFIG_DESC_RDP;
};

constexpr auto rdp_verbose_name_positions = config_enum_description_to_array<RDPVerboseConfigString>();

struct ScreenInfoWidget
{
    ScreenInfoWidget()
    : label1("x")
    , label2("x")
    {
        bpp.addItem("8 bits");
        bpp.addItem("15 bits");
        bpp.addItem("16 bits");
        bpp.addItem("24 bits");
        // bpp_widget.addItem("32 bits", int(32));

        layout.addWidget(&width);
        layout.addWidget(&label1);
        layout.addWidget(&height);
        layout.addWidget(&label2);
        layout.addWidget(&bpp);

        widget.setLayout(&layout);
    }

    void reset()
    {
        *value = original_value;
        update_widgets();
    }

    void attach(ScreenInfo* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        update_widgets();
    }

    void synchronize()
    {
        value->width = checked_int(std::min(4096u, width.text().toUInt()));
        value->height = checked_int(std::min(4096u, height.text().toUInt()));
        switch (bpp.currentIndex()) {
            case 0: value->bpp = BitsPerPixel::BitsPP8; break;
            case 1: value->bpp = BitsPerPixel::BitsPP15; break;
            case 2: value->bpp = BitsPerPixel::BitsPP16; break;
            case 3: value->bpp = BitsPerPixel::BitsPP24; break;
        }
    }

    void update_widgets()
    {
        width.setText(to_qstring(int_to_decimal_chars(original_value.width)));
        height.setText(to_qstring(int_to_decimal_chars(original_value.height)));
        switch (underlying_cast(original_value.bpp)) {
            case  8: bpp.setCurrentIndex(0); break;
            case 15: bpp.setCurrentIndex(1); break;
            case 16: bpp.setCurrentIndex(2); break;
            case 24: bpp.setCurrentIndex(3); break;
        }
    }

    QWidget widget;

private:
    ScreenInfo* value = nullptr;
    ScreenInfo original_value {};
    QLineEdit width;
    QLineEdit height;
    QComboBox bpp;
    QLabel label1;
    QLabel label2;
    QHBoxLayout layout;
};

struct RDPVerboseWidget
{
    RDPVerboseWidget()
    : RDPVerboseWidget(std::make_index_sequence<rdp_verbose_name_positions.size>())
    {}

    template<std::size_t... Ints>
    RDPVerboseWidget(std::index_sequence<Ints...>)
    : layout()
    , widgets{QCheckBox(to_qstring(rdp_verbose_name_positions.names[Ints]))...}
    {
        for (auto& w : widgets) {
            layout.addWidget(&w);
        }
        widget.setLayout(&layout);
    }

    void reset()
    {
        *value = original_value;
        update_widgets();
    }

    void attach(RDPVerbose* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        update_widgets();
    }

    void synchronize()
    {
        *value = {};
        auto wp = widgets;
        for (auto bitpos : rdp_verbose_name_positions.bit_positions) {
            auto bit = wp->isChecked() ? 1u : 0u;
            *value = safe_int(underlying_cast(*value) | (bit << bitpos));
        }
    }

    void update_widgets()
    {
        auto wp = widgets;
        for (auto bitpos : rdp_verbose_name_positions.bit_positions) {
            wp->setChecked(underlying_cast(*value) & (1u << bitpos));
        }
    }

    QWidget widget;

private:
    RDPVerbose* value = nullptr;
    RDPVerbose original_value {};
    QVBoxLayout layout;
    QCheckBox widgets[rdp_verbose_name_positions.size];
};


template<class T>
struct cli_parser_to_widget
{};

template<>
struct cli_parser_to_widget<cli::parsers::on_off_location>
{
    using Widget = CheckBox;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<qtclient::ProtocolMod>>
{
    using Widget = ProtocolWidget;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<RDPVerbose>>
{
    using Widget = RDPVerboseWidget;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<uint32_t>>
{
    using Widget = NumberEdit<uint32_t>;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<int>>
{
    using Widget = NumberEdit<int>;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<std::string>>
{
    using Widget = StringEdit;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<ScreenInfo>>
{
    using Widget = ScreenInfoWidget;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<KeyLayout::KbdId>>
{
    using Widget = KbdWidget<KeyLayout::KbdId>;
};


struct NoWidget
{
};

template<class Act>
struct cli_parser_to_widget<cli::parsers::trigger<Act>>
{
    using Widget = NoWidget;
};

template<class>
struct cli_option_to_widget
{
    using Widget = NoWidget;
};

template<class Short, class Long, class Parser, class Name>
struct cli_option_to_widget<cli::Option<Short, Long, Parser, Name>>
{
    using Widget = typename cli_parser_to_widget<Parser>::Widget;
};

template<std::size_t NHelper, std::size_t NSerializableOption, class... Widgets>
struct ProfileWidget : QWidget
{
    ProfileWidget(QWidget* parent)
    : QWidget(parent)
    {
        setLayout(&layout);
    }

    QFormLayout form_layouts[NHelper];
    QLabel helpers[NHelper];
    QLabel label[NSerializableOption];
    QVBoxLayout layout;
    cli::Options<Widgets...> widgets;
};


template<bool cond, class F>
void if_call(F f)
{
    if constexpr (cond) {
        f([](auto& x) -> decltype(x) { return x; });
    }
}

}

QWidget* qtclient::create_widget_profile(Profile& profile, QWidget* parent)
{
    // bool
    // string
    // TODO dirname
    // number
    // select (kbdid / bpp)

    // name = name.replace('-', ' ').replace(' dir$', 'directory').upperFirstLetter()

    auto* w = profile_as_cli_options(profile)([&](auto&... options){
        auto* w = new ProfileWidget<
            (0 + ... + std::is_same_v<std::decay_t<decltype(options)>, cli::Helper>),
            (0 + ... + is_serializable_option<std::decay_t<decltype(options)>>::value),
            typename cli_option_to_widget<std::decay_t<decltype(options)>>::Widget...
        >(parent);

        int ihelper = -1;
        QFormLayout* form_layout;
        QLabel* label;
        w->widgets([&](auto&... widgets){
            // cli::Option -> Widget.attach(value)
            (..., if_call<is_serializable_option<std::decay_t<decltype(options)>>::value>([&](auto get){
                get(widgets).attach(get(options)._parser.value);
                form_layout->addWidget(&get(widgets).widget);
            }));

            // cli::Helper -> QFormLayout
            (..., if_call<std::is_same_v<cli::Helper, std::decay_t<decltype(options)>>>([&](auto get){
                ++ihelper;
                auto& label = w->helpers[ihelper];
                label.setText(to_qstring(get(options).s));
                form_layout = &w->form_layouts[ihelper];
                w->layout.addWidget(&label);
                w->layout.addLayout(form_layout);
            }));
        });

        return w;
    });

    return w;
}
