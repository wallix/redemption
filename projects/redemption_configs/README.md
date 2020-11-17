1. [Add/Modify variable](#addmodify-variable)
    1. [Ini (spec::*)](#ini-spec)
        1. [log_policy (spec::constants::*)](#log_policy-specconstants)
        2. [spec_attr (spec::constants::*)](#spec_attr-specconstants)
    2. [Sesman (sesman::*)](#sesman-sesman)
        1. [sesman_io (sesman::constants::*)](#sesman_io-sesmanconstants)
        2. [is_target_context (sesman::constants::*)](#is_target_context-sesmanconstants)
    3. [Connection Policy (connpolicy::*)](#connection-policy-connpolicy)
        1. [connpolicy_attr (connpolicy::constants::*)](#connpolicy_attr-connpolicyconstants)
    4. [Cpp (cpp::*)](#cpp-cpp)
    5. [type](#type)
        1. [special cpp_type](#special-cpp_type)
    6. [names](#names)
    7. [desc](#desc)
    8. [default_value](#default_value)
    9. [prefix](#prefix)
2. [Add/Modify enumeration type](#addmodify-enumeration-type)
3. [Build](#build)


# Add/Modify variable

Edit `configs_specs/configs/specs/config_spec.hpp`

- `CONFIG_DEFINE_TYPE` macro: declare a redemption type (forward declaration)
- `W.section(names{section_name}, [&]{ /* members... */ })`
- `W.section(names{.cpp=name, .ini=name, .sesman=name, .connpolicy=name}, [&]{ /* members... */ })`
- `W.section(names, [&]{ /* members... */ })`
- `W.member(spec_attr, sesman_io | connpolicy[, back_to_selector_policy], log_policy, type, names[, desc][, default_value][, ...])`. Ordering value is not significant.
- `W.sep()`: empty line (human readable)



## Ini (spec::*)

- `spec::type_<cpp_type>{}`


### log_policy (spec::constants::*)

- `loggable` (or `L` in const_spec.hpp)
- `unloggable` (or `NL` in const_spec.hpp)
- `unloggable_if_value_contains_password` (or `VNL` in const_spec.hpp)


### spec_attr (spec::constants::*)

- `no_ini_no_gui` incompatible with other value
- `ini_and_gui`
- `hidden_in_gui` incompatible with hex, advanced, iptable and password
- `hex_in_gui`
- `advanced_in_gui`
- `iptables_in_gui`
- `password_in_gui`



## Sesman (sesman::*)

- `sesman::type_<cpp_type>{}`
- `sesman::connection_policy{filetype[, connpolicy_attr]}` enable connpolicy. Combinable with `connpolicy_attr`


### sesman_io (sesman::constants::*)

- `no_sesman`
- `proxy_to_sesman`
- `sesman_to_proxy`
- `sesman_rw` (`proxy_to_sesman + sesman_to_proxy`)


### is_target_context (sesman::constants::*)

Only with `proxy_to_sesman`, `sesman_to_proxy` and `sesman_rw`. Sesman should send value before the connection to target.

- `is_target_ctx`
- `not_target_ctx`



## Connection Policy (connpolicy::*)

- `connpolicy::section{"name"}` overwrite section name


### connpolicy_attr (connpolicy::constants::*)

- `hex_in_connpolicy`
- `advanced_in_connpolicy`

Combination with `|`: `sesman::connection_policy{"rdp"} | advanced_in_connpolicy | hex_in_connpolicy`.



## Cpp (cpp::*)

- `cpp::name{"variable name"}`
- `cpp::type_<cpp_type>{}`
- `cpp::expr{"expr as string"}`
- `CPP_EXPR(expression)` (for macro: `CPP_EXPR(MACRO_NAME)`). equivalent to cpp::expr{#expression"}



## type

- `type_<cpp_type>()`: ini, sesman, connpolicy and cpp type
- `spec::type_<cpp_type>()`: ini type
- `sesman::type_<cpp_type>()`: connpolicy type
- `connpolicy::type_<cpp_type>()`: sesman type
- `cpp::type_<cpp_type>()`: cpp type

Note: `W.member(type_<int>(), sesman::type_<bool>(), ...)` is ok.

### special cpp_type

- `types::u16` instead of uint16_t
- `types::u32` instead of uint32_t
- `types::u64` instead of uint64_t
- `types::list<cpp_type>`: comma-separated values (cf: `0, 3, 4`)
- `types::ip_string`
- `types::dirpath`: always `/` terminated. Note: use `std::string` for file path type.
- `types::range<cpp_type, min, max>`
- `types::fixed_string<n>`: size without zero-terminal.
- `types::fixed_binary<n>`: size without zero-terminal.
- `types::file_permission`



## names

```cpp
struct names
{
    std::string cpp; // cpp name and default value for ini, sesman and connpolicy
    std::string ini {};
    std::string sesman {};
    std::string connpolicy {};
};
```

```cpp
W.member(names{.cpp="fish", .ini="superfish", .sesman="netfish"}, ...);
```



## desc

- `desc{"desc..."}`



## default_value

- `set(CPP_EXPR(MACRO_NAME))`: instead of `MACRO_NAME` (cf: `set(CPP_MACRO(HASH_PATH)))`)
- `set(any_value)`
- `connpolicy::set(any_value)`

By default, initialized with `{}` (cf: `type value = {}`).


## prefix

- `prefix_value`: `prefix_value disable_prefix_val{"disable"}` in `const_spec.hpp`


# Add/Modify enumeration type

Edit `configs_specs/configs/specs/config_type.hpp`

```cpp
// enum { a = 1, b = 2, c = 4, ... }
e.enumeration_flags(enum_name[, enum_desc[, enum_info]])
    .value(value_name[, value_desc])[.alias(alias_name)...][.exclude()...]
    ...

// enum { a, b, c, ... }
e.enumeration_list(enum_name[, enum_desc[, enum_info]])
    .value(value_name[, value_desc])[.alias(alias_name)...][.exclude()...]
    ...

// enum { a = v1, b = v2, c = v3, ... }
e.enumeration_set(enum_name[, enum_desc[, enum_info]])
    .value(value_name, uint_val[, value_desc])[.alias(alias_name)...]
    ...

// enum_info: description after values
```


# Build

```bash
bjam
```

or

```bash
bjam generate_cpp_enumeration
bjam generate_config_spec
```
