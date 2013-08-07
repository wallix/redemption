#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <stdint.h>
#include <boost/lexical_cast.hpp>
#include <boost/range/iterator_range.hpp>

struct Definition {
    struct utils{
        static const char * ignorespace(const char * s){
            while (*s && std::isspace(*s)) {
                ++s;
            }
            return s;
        }

        static const char * ignoretoken(const char * s){
            while (*s && (
                ('a' <= *s && *s <= 'z')
                || ('A' <= *s && *s <= 'Z')
                || ('0' <= *s && *s <= '9')
                || *s == '_' || *s == '-')
            ) {
                ++s;
            }
            return s;
        }

        static int chex2i(char c, bool& err) {
            if ('0' <= c && c <= '9') {
                return c - '0';
            }
            if ('A' <= c && c <= 'Z') {
                return c - 'A' + 10;
            }
            if ('a' <= c && c <= 'z') {
                return c - 'a' + 10;
            }
            err = 1;
            return 0;
        }
    };

    static int parse_color(const char * first, const char * last, bool& err)
    {
        int color = 0;

        if (*first == '#') {
            if (last - first == 4) {
                int r = utils::chex2i(first[1], err);
                int g = utils::chex2i(first[2], err);
                int b = utils::chex2i(first[3], err);
                color = (r << 20) + (r << 16) + (g << 12) + (g << 8) + (b << 4) + b;
            }
            else if (last - first == 7) {
                color = (utils::chex2i(first[1], err) << 20)
                      + (utils::chex2i(first[2], err) << 16)
                      + (utils::chex2i(first[3], err) << 12)
                      + (utils::chex2i(first[4], err) << 8)
                      + (utils::chex2i(first[5], err) << 4)
                      + utils::chex2i(first[6], err);
            }
            else {
                err = 1;
            }
        }
        else if (last - first >= 10
            && first[0] == 'r'
            && first[1] == 'g'
            && first[2] == 'b'
            && last[-1] == ')')
        {
            first = utils::ignorespace(first + 3);
            if (*first != '(') {
                err = 1;
                return color;
            }

            struct next_component {
                static int next(const char *& p, bool& err){
                    p = utils::ignorespace(p);
                    const char * s = p;
                    while ('0' <= *p && *p <= '9') {
                        ++p;
                    }

                    int component = 0;
                    switch (p - s) {
                        case 3: component = s[0] * 100 + s[1] * 10 + s[2];
                            break;
                        case 2: component = s[0] * 10 + s[1];
                            break;
                        case 1: component = s[0];
                            break;
                        default:
                            err = 1;
                    }
                    p = utils::ignorespace(p);
                    return component;
                }
            };

            int component = next_component::next(++first, err);
            std::cout << "*first: " << (*first) << std::endl;
            if (*first != ',' && component > 255) {
                err = 1;
                return 0;
            }
            color = component << 16;
            component = next_component::next(++first, err);
            std::cout << "*first: " << (*first) << std::endl;
            if (*first != ',' && component > 255) {
                err = 1;
                return 0;
            }
            color += component << 8;
            component = next_component::next(++first, err);
            std::cout << "*first: " << (*first) << std::endl;
            if (first + 1 != last && component > 255) {
                err = 1;
                return 0;
            }
            color += component;
        }
        else {
            std::cout << "else *first: " << (*first) << std::endl;
            err = 1;
        }

        return color;
    }

    struct Property {
        std::string name;

        union Value {
            char * s;
            int i;
            unsigned u;
            long l;
            unsigned long ul;
        } value;
        int type;

        Property()
        : name()
        , type(0)
        {}

        Property(const Property& other)
        : name(other.name)
        , type(other.type)
        {
            if ('i' == other.type) {
                this->value.i = other.value.i;
            }
            if ('s' == other.type) {
                this->value.s = strdup(other.value.s);
            }
        }

        bool set_value(const char * first, const char * last) {
            if (this->name == "bgcolor" || this->name == "color") {
                bool err = false;
                this->value.i = parse_color(first, last, err);
                this->type = 'i';
                return !err;
            }
            else {
                this->value.s = static_cast<char*>(std::malloc(last - first + 1));
                std::memcpy(this->value.s, first, last - first + 1);
                this->value.s[last - first] = 0;
                this->type = 's';
                return true;
            }
        }

        ~Property()
        {
            if (this->type == 's') {
                free(this->value.s);
            }
        }
    };

    struct Target {
        std::string name;
        std::vector<Property> properties;
        bool is_name() const{
            return this->name[0] == '#';
        }
        bool operator < (const Target& other) const {
            return this->name < other.name;
        }
    };

    std::vector<Target> targets;
    size_t boundary;

    Definition()
    : targets()
    , boundary(0)
    {}

    size_t parse_add(const char * s)
    {
        size_t before_size = this->targets.size();
        const char * begin_s = s;
        s = utils::ignorespace(s);
        const char * first = s;
        while (*s) {
            if (*s == '#') {
                ++s;
            }
            s = utils::ignoretoken(s);
            if (!*s) {
                return s - begin_s - 1;
            }
            Target target;
            target.name.assign(first, s);
            std::cout << "target: " << (target.name) << std::endl;
            s = utils::ignorespace(s);
            if (*s != '{') {
                return s - begin_s;
            }
            while (1) {
                first = utils::ignorespace(s+1);
                s = utils::ignoretoken(first);
                if (!*s) {
                    return s - begin_s - 1;
                }
                if (*s == '}') {
                    break ;
                }
                Property property;
                property.name.assign(first, s);
                std::cout << "property: " << (property.name) << std::endl;
                first = utils::ignorespace(s);
                if (*first != ':') {
                    return first - begin_s;
                }
                first = utils::ignorespace(first+1);
                s = first;
                ///TODO check value
                while (*s && *s != '\n' && *s != ';' && *s != '}') {
                    ++s;
                }
                if (!*s) {
                    return s - begin_s - 1;
                }
                const char * tmps = s;
                if (*s == '\n' || *s == ';' || *s == '}') {
                    while (std::isspace(s[-1])) {
                        --s;
                    }
                }

                if (!property.set_value(first, s)){
                    return first - begin_s;
                }

                if (property.type == 'i')
                    std::cout << "value: " << (property.value.i) << std::endl;
                else
                    std::cout << "value: " << (property.value.s) << std::endl;

                s = tmps;
                target.properties.push_back(property);
                if (*s == '}') {
                    break ;
                }
            }
            s = utils::ignorespace(s+1);
            first = s;
            if (!target.properties.empty()){
                this->targets.push_back(target);
            }
        }

        std::sort(this->targets.begin() + before_size, this->targets.end());
        if (before_size) {
            std::inplace_merge(this->targets.begin(),
                               this->targets.begin() + before_size,
                               this->targets.end());
        }
        size_t last = this->targets.size();
        while (this->boundary < last && this->targets[this->boundary].is_name()) {
            ++this->boundary;
        }
        return size_t(-1);
    }
};

namespace rwl {

template<int(*F)(int)>
const char * ignore(const char * s)
{
    while (F(*s)) {
        ++s;
    }
    return s;
}

struct cstring_range
{
    const char * first;
    const char * last;

    cstring_range(const char * first, const char * last)
    : first(first)
    , last(last)
    {}

    bool operator==(const char * s) const
    {
        for (const char * ss = this->first; ss != this->last; ++ss, ++s) {
            if (*ss != *s) {
                return false;
            }
        }
        return true;
    }

    char * copy() const
    {
        char * s = (char*)std::malloc(this->last - this->first + 1);
        std::memcpy(s, this->first, this->last - this->first);
        s[this->last - this->first] = 0;
        return s;
    }
};

struct property_t
{
    const char * name;
    const int type;

    property_t(int type)
    : type(type)
    {}
};

struct properties_t
{
    const char * id;
    const char * style;
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
    int color;

    std::vector<property_t*> children;

    properties_t()
    : id(0)
    , style(0)
    , children()
    {}
};

template<typename To>
To range_cast(const char * first, const char * last)
{
    return boost::lexical_cast<To>(boost::make_iterator_range(first, last));
}

struct string_property_t
: property_t
{
    const char * s;

    string_property_t(const char * first, const char * last)
    : property_t('s')
    , s(cstring_range(first, last).copy())
    {}
};

struct unsigned_property_t
: property_t
{
    unsigned u;

    unsigned_property_t(const char * first, const char * last)
    : property_t('u')
    , u(range_cast<unsigned>(first, last))
    {}

    unsigned_property_t(unsigned u)
    : property_t('u')
    , u(u)
    {}
};

struct int_property_t
: property_t
{
    int i;

    int_property_t(const char * first, const char * last)
    : property_t('i')
    , i(range_cast<int>(first, last))
    {}

    int_property_t(int i)
    : property_t('i')
    , i(i)
    {}
};

struct properties_property_t
: property_t
{
    properties_t p;

    properties_property_t()
    : property_t('p')
    , p()
    {}
};

const char * parser_value(const cstring_range& srange, const char * first,
                          properties_t& properties)
{
    first = ignore<isspace>(first);
    if (srange == "id" || srange == "style") {
        if (std::isalpha(*first)) {
            const char * s = ignore<isalnum>(first);
            if (s == first) {
                std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
            }
            else {
                //properties.children.push_back(new string_property_t(first, s));
                ('i' == *srange.first ? properties.id : properties.style)
                    = cstring_range(first, s).copy();
                first = s;
            }
        }
        else {
            std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
        }
    }
    else if (srange == "x" || srange == "y") {
        const char * s = ignore<isalnum>(first);
        if (s == first) {
            std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
        }
        else {
            //properties.children.push_back(new int_property_t(first, s));
            ('x' == *srange.first ? properties.x : properties.y)
                = (int16_t)range_cast<int>(first, s);
            first = s;
        }
    }
    else if (srange == "width" || srange == "height") {
        const char * s = ignore<isalnum>(first);
        if (s == first) {
            std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
        }
        else {
            //properties.children.push_back(new unsigned_property_t(first, s));
            ('w' == *srange.first ? properties.width : properties.height)
                = (uint16_t)range_cast<unsigned>(first, s);
            first = s;
        }
    }
    else if (srange == "color") {

    }
    else if (srange == "border") { ///TODO etc

    }
    else {
        std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
    }

    return first;
}


const char * parser_block(const char * name_first, const char * name_last,
                          properties_t& properties);

const char * parser_property(const char * first, properties_t& properties)
{
    first = ignore<isspace>(first);

    const char * s = ignore<isalpha>(first+1); ///TODO recursive property border.left

    const char * sep = ignore<isspace>(s);

    if (*sep == ':') {
        first = parser_value(cstring_range(first, s), sep+1, properties);
    }
    else {
        first = parser_block(first, s, properties);
    }

    return first;
}

const char * parser_block(const char * name_first, const char * name_last,
                          properties_t& properties)
{
    const char * first = ignore<isspace>(name_last);

    if ('{' == *first) {
        first = ignore<isspace>(first + 1);
        properties_property_t * prop_child = new properties_property_t; ///TODO if type
        while (*first != '}' && *first) {
            first = parser_property(first, prop_child->p);
            first = ignore<isblank>(first);
            if (*first == ';' || *first == '\n') {
                first = ignore<isspace>(first+1);
            }
        }
        if (*first == '}') {
            prop_child->name = cstring_range(name_first, name_last).copy();
            properties.children.push_back(prop_child);
            ++first;
        }
        else {
            delete prop_child;
            std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
        }
    }
    else {
        std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
    }
    return first;
}

void parser(const char * first, properties_t& properties)
{
    while (*(first = ignore<isspace>(first))) {
        if (std::isalpha(*first)) {
            first = parser_block(first, ignore<isalnum>(first + 1), properties);
        }
        else {
            std::cerr << "line: " << __LINE__ << std::endl; throw __LINE__;
        }
    }
}

}



// void first_imp() {
//     std::string s;
//     {
//         std::ifstream ifs("/tmp/login_mod.rwl");
//         std::getline(ifs, s, '\0');
//         if (ifs.eof() == false) {
//             return 2;
//         }
//     }
//     std::cout << "style:\n" << (s) << std::endl;
//
//     Definition definition;
//     int perr = definition.parse_add(s.c_str());
//     if (perr != -1){
//         std::cout << "error " << perr << std::endl;
//         std::cout << '"' << s.substr(perr-5, 10) << '"' << std::endl;
//         return 3;
//     }
//
//     for (auto& target: definition.targets) {
//         std::cout << "target: '" << target.name << "'\n";
//         for (auto& property: target.properties) {
//             std::cout << "\t'" << property.name << "': '";
//             if (property.type == 'i')
//                 std::cout << (property.value.i) << std::endl;
//             else
//                 std::cout << (property.value.s) << std::endl;
//             std::cout << "'\n";
//         }
//     }
//
//     std::cout << "boundary: " << definition.boundary << std::endl;
// }

void display_properties(const rwl::properties_t& properties, std::string tab = "")
{
    std::cout
    << tab << "id: " << (properties.id ? properties.id : "nil") << std::endl
    << tab << "style: " << (properties.style ? properties.style : "nil") << std::endl
    << tab << "x: " << properties.x << std::endl
    << tab << "y: " << properties.y << std::endl
    << tab << "width: " << properties.width << std::endl
    << tab << "height: " << properties.height << std::endl
    << tab << "color: " << properties.color << std::endl
    ;
    for (size_t n = 0, max = properties.children.size(); n != max; ++n) {
        rwl::property_t& property = *properties.children[n];
        switch (property.type) {
            case 'i':
                std::cout << tab << property.name << ": "
                    << ((rwl::int_property_t&)property).i;
                break;
            case 'u':
                std::cout << tab << property.name << ": "
                    << ((rwl::unsigned_property_t&)property).u;
                break;
            case 's':
                std::cout << tab << property.name << ": "
                    << ((rwl::string_property_t&)property).s;
                break;
            case 'p':
                std::cout << std::endl;
                display_properties(((rwl::properties_property_t&)property).p, tab+"  ");
                break;
        }
    }
}

int main(/*int argc, char **argv*/) {
    std::string s;
    {
        std::ifstream ifs("/tmp/mod.rwl");
        std::getline(ifs, s, '\0');
        if (ifs.eof() == false) {
            return 2;
        }
    }
    s += "\n";

    std::cout.sync_with_stdio(false);
    std::cerr.sync_with_stdio(false);

    std::cout << "style:\n" << (s) << std::endl;

    rwl::properties_t properties;
    try {
        rwl::parser(s.c_str(), properties);
    } catch (int c) {
        std::cout << char(c) << "\n";
    }

    std::cout << "\nresultat:\n" << std::endl;
    display_properties(properties);

    return 0;
}
