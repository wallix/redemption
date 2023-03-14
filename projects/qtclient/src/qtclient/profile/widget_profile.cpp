/*
SPDX-FileCopyrightText: 2023 Wallix Proxies Team

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtGui/QIntValidator>

#include "qtclient/profile/widget_profile.hpp"
#include "qtclient/profile/profile_as_cli_options.hpp"
#include "utils/ascii.hpp"
#include "utils/sugar/chars_to_int.hpp"
#include "utils/sugar/int_to_chars.hpp"
#include "utils/config_enum_description_to_array.hpp"
#include "utils/static_string.hpp"
#include "keyboard/keylayouts.hpp"
#include "debug_verbose_description.hpp"

namespace
{

QString to_qstring(chars_view str)
{
    return QString::fromUtf8(str.data(), checked_int(str.size()));
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

protected:
    std::string* value = nullptr;
    std::string original_value {};
};

struct DirectoryWidget : StringEdit
{
    DirectoryWidget()
    : choose("Choose...")
    {
        widget.addWidget(&choose);
        widget.addWidget(&this->StringEdit::widget);

        choose.connect(&choose, &QPushButton::released, [this]{
            auto dirname = QFileDialog::getExistingDirectory(
                &choose, QString(), to_qstring(*value),
                QFileDialog::ReadOnly |
                QFileDialog::ShowDirsOnly |
                QFileDialog::DontResolveSymlinks |
                QFileDialog::DontUseCustomDirectoryIcons
            );
            if (!dirname.isEmpty()) {
                StringEdit::widget.setText(dirname);
                *value = dirname.toStdString();
            }
        });
    }

    QHBoxLayout widget;

private:
    QPushButton choose;
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

    QComboBox widget;

private:
    qtclient::ProtocolMod* value = nullptr;
    qtclient::ProtocolMod original_value {};
};

struct KbdWidget
{
    KbdWidget()
    {
        widget.addWidget(&select);
        widget.addWidget(&edit);

        select.addItem(QStringLiteral("Unknown"), QVariant(uint32_t(0)));
        for (auto&& layout : keylayouts()) {
            auto text = static_str_concat<128>(
                truncated_bounded_array_view<64>(layout.name),
                " (0x"_sized_av, to_hex(layout.kbdid), ')');
            select.addItem(to_qstring(text), QVariant(safe_cast<uint32_t>(layout.kbdid)));
        }

        select.connect(&select, &QComboBox::currentIndexChanged, [this](int idx){
            if (!updatable) {
                return ;
            }
            if (idx > 0) {
                last_value = keylayouts()[checked_int(idx - 1)].kbdid;
                setText(last_value);
            }
            else {
                last_value = KeyLayout::KbdId();
                edit.setText(QStringLiteral("0x0"));
            }
        });

        edit.connect(&edit, &QLineEdit::textChanged, [this](QStringView text){
            auto cpy = [](QStringView text, char* out) -> chars_view {
                return {out, std::transform(text.begin(), text.end(), out,
                    [](QChar c){ return static_cast<char>(c.unicode()); }
                )};
            };

            updatable = false;

            using Int = std::underlying_type_t<KeyLayout::KbdId>;
            if (text.size() > 2 && text[0] == '0') {
                constexpr qsizetype max_string_len = 16;
                if (text.size() <= max_string_len && (text[1] == 'x' || text[1] == 'X')) {
                    char buf[max_string_len];
                    auto av = cpy(text.mid(2), buf);
                    auto r = hexadecimal_chars_to_int<Int>(av);
                    if (r.ec == std::errc() && r.ptr == av.end()) {
                        last_value = KeyLayout::KbdId(r.val);
                        select.setCurrentIndex(find_layout(last_value).idx);
                    }
                }
            }
            else {
                constexpr qsizetype max_string_len = 32;
                if (text.size() <= max_string_len) {
                    char buf[max_string_len];
                    auto [layout, idx] = find_layout(cpy(text, buf).as<std::string_view>());
                    select.setCurrentIndex(idx);
                    if (layout) {
                        last_value = layout->kbdid;
                    }
                }
            }

            updatable = true;
        });
    }

    void reset()
    {
        *value = original_value;
        update_widgets();
    }

    void attach(KeyLayout::KbdId* value_ref)
    {
        value = value_ref;
        original_value = *value_ref;
        update_widgets();
    }

    void synchronize()
    {
        *value = last_value;
    }

    void update_widgets()
    {
        auto [layout, idx] = find_layout(original_value);
        select.setCurrentIndex(idx);
        last_value = original_value;
        setText(original_value);
    }

    QHBoxLayout widget;

private:
    static array_view<KeyLayout> keylayouts()
    {
        return keylayouts_sorted_by_name();
    }

    static int_to_chars_result to_hex(KeyLayout::KbdId kbdid)
    {
        return int_to_hexadecimal_upper_chars(underlying_cast(kbdid));
    }

    struct FindLayoutResult
    {
        KeyLayout const* layout;
        int idx;
    };

    static FindLayoutResult find_layout(KeyLayout::KbdId kbdid)
    {
        for (auto&& layout : keylayouts()) {
            if (layout.kbdid == kbdid) {
                return {&layout, checked_int(&layout - keylayouts().begin() + 1)};
            }
        }
        return {};
    }

    static FindLayoutResult find_layout(std::string_view name)
    {
        for (auto&& layout : keylayouts()) {
            if (layout.name.to_sv() == name) {
                return {&layout, checked_int(&layout - keylayouts().begin() + 1)};
            }
        }
        return {};
    }

    void setText(KeyLayout::KbdId kbdid)
    {
        auto text = static_str_concat<64>("0x"_sized_av, to_hex(kbdid));
        edit.setText(to_qstring(text));
    }

    KeyLayout::KbdId* value = nullptr;
    KeyLayout::KbdId original_value {};
    KeyLayout::KbdId last_value;
    bool updatable = true;
    QComboBox select;
    QLineEdit edit;
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

        widget.addWidget(&width);
        widget.addWidget(&label1);
        widget.addWidget(&height);
        widget.addWidget(&label2);
        widget.addWidget(&bpp);

        int nbChar = 5;

        width.setMaxLength(nbChar);
        height.setMaxLength(nbChar);

        width.setPlaceholderText("Width");
        height.setPlaceholderText("Height");

        widget.addStretch();

        int fontCharWidth = QFontMetrics(width.font()).horizontalAdvance(QChar('0')) * (nbChar + 2);
        width.setFixedWidth(fontCharWidth);
        height.setFixedWidth(fontCharWidth);
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

    QHBoxLayout widget;

private:
    ScreenInfo* value = nullptr;
    ScreenInfo original_value {};
    QLineEdit width;
    QLineEdit height;
    QComboBox bpp;
    QLabel label1;
    QLabel label2;
};

struct RDPVerboseWidget
{
    RDPVerboseWidget()
    : RDPVerboseWidget(std::make_index_sequence<rdp_verbose_name_positions.size>())
    {}

    template<std::size_t... Ints>
    RDPVerboseWidget(std::index_sequence<Ints...>)
    : widgets{QCheckBox(to_qstring(rdp_verbose_name_positions.names[Ints]))...}
    {
        for (auto& w : widgets) {
            widget.addWidget(&w);
        }
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

    QVBoxLayout widget;

private:
    RDPVerbose* value = nullptr;
    RDPVerbose original_value {};
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
    using Widget = KbdWidget;
};

template<>
struct cli_parser_to_widget<cli::parsers::arg_location<qtclient::DirectoryStringPath>>
{
    using Widget = DirectoryWidget;
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

template<std::size_t NHelper, class... Widgets>
struct ProfileWidget : QScrollArea
{
    ProfileWidget(QWidget* parent)
    : QScrollArea(parent)
    {
        setWidgetResizable(true);
        setWidget(&inner);
        inner.setLayout(&layout);
    }

    QWidget inner;
    QVBoxLayout layout;
    QGroupBox groups[NHelper];
    QFormLayout forms[NHelper];
    cli::Options<Widgets...> widgets;
};


QString qstring_label(chars_view str)
{
    auto s = to_qstring(str);
    s.replace(QLatin1Char('-'), QLatin1Char(' '));
    s.data()[0] = QLatin1Char(ascii_to_upper(str[0]));
    // dir -> directory
    if (str.as<std::string_view>().ends_with(std::string_view("-dir"))) {
        s += QStringLiteral("ectory");
    }
    return s;
}

QString qstring_helper(std::string_view str)
{
    // skip '=' in "==== label ===="
    auto* first = std::find(str.begin(), str.end(), ' ');
    auto* last = std::find(first, str.end(), '=');
    assert(first != str.end());
    assert(last != str.end());
    return to_qstring({first+1, last-1});
}

}

QWidget* qtclient::create_widget_profile(Profile& profile, QWidget* parent)
{
    return profile_as_cli_options(profile)([&](auto&... options){
        auto* w = new ProfileWidget<
            (std::is_same_v<std::decay_t<decltype(options)>, cli::Helper> + ...),
            typename cli_option_to_widget<std::decay_t<decltype(options)>>::Widget...
        >(parent);

        std::ptrdiff_t ihelper = 0;
        QGroupBox* group;
        QFormLayout* form;
        w->widgets([&](auto&... widgets){
            ([&]{
                // cli::Option -> Widget.attach(value)
                if constexpr (is_serializable_option<std::decay_t<decltype(options)>>::value) {
                    widgets.attach(options._parser.value);
                    form->addRow(qstring_label(options._long_name), &widgets.widget);
                }
                // cli::Helper -> QFormLayout
                else if constexpr (std::is_same_v<cli::Helper, std::decay_t<decltype(options)>>) {
                    group = &w->groups[ihelper];
                    form = &w->forms[ihelper];
                    group->setTitle(qstring_helper(options.s));
                    group->setLayout(form);
                    w->layout.addWidget(group);
                    ++ihelper;
                }
                // cli::trigger is ignored
            }(), ...);
        });

        return w;
    });
}
