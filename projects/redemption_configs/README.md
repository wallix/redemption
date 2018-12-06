# Add/Modify variable

Edit `configs_specs/configs/specs/config_spec.hpp`

- `CONFIG_DEFINE_TYPE` macro: declare a redemption type (forward declaration and more)
- `W.section(section_name, [&]{ /* members... */ })`
- `W.section(W.names(section_name, *::name{section_name}), [&]{ /* members... */ })`
- `W.member(spec_attr, sesman_io | connpolicy, log_policy, type, name[, desc][, default_value][, ...])`. Ordering value is not significant.
- `W.sep()`: empty line (human readable)



## Ini (spec::*)

- `spec::name{"variable name"}`
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

- `sesman::name{"variable name"}`
- `sesman::type_<cpp_type>{}`
- `sesman::connection_policy{name[, connpolicy_attr]}` enable connpolicy. Combinable with `connpolicy_attr`
- `sesman::deprecated_names{"name1", ...}` (sesman -> proxy for connpolicy only)


### sesman_io (sesman::constants::*)

- `no_sesman`
- `proxy_to_sesman`
- `sesman_to_proxy`
- `sesman_rw` (`proxy_to_sesman + sesman_to_proxy`)



## Connection Policy (connpolicy::*)

- `connpolicy::name{"variable name"}`
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
- `sesman::type_<cpp_type>()`: sesman and connpolicy type
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
- `types::static_string`
- `types::fixed_string<n>`: size without zero-terminal.
- `types::fixed_binary<n>`: size without zero-terminal.



## name

- `char *`: ini, sesman, connpolicy and cpp name
- `spec::name`: ini name
- `sesman::name`: sesman name
- `cpp::name`: cpp name
- `connpolicy::name`: connection policy name

Note: `W.member("fish", sesman::name{"netfish"}, ini::name{"superfish"}, ...)` is ok.



## desc

- `desc{"desc..."}`



## default_value

- `set(CPP_EXPR(MACRO_NAME))`: instead of `MACRO_NAME` (cf: `set(CPP_MACRO(HASH_PATH)))`)
- `set(any_value)`
- `connpolicy::set(any_value)`

By default, initialized with `{}` (cf: `type value = {}`).



# Add/Modify enumeration type

Edit `configs_specs/configs/specs/config_type.hpp`

```cpp
// enum { a = 1, b = 2, c = 4, ... }
e.enumeration_flags(enum_name[, enum_desc][, enum_info])
    [.set_string_parser()]
    .value(value_name[, value_desc])[.alias(alias_name)...]
    ...

// enum { a, b, c, ... }
e.enumeration_list(enum_name[, enum_desc][, enum_info])
    [.set_string_parser()]
    .value(value_name[, value_desc])[.alias(alias_name)...]
    ...

// enum { a = v1, b = v2, c = v3, ... }
e.enumeration_set(enum_name[, enum_desc][, enum_info])
    [.set_string_parser()]
    .value(value_name, integer_val[, value_desc])[.alias(alias_name)...]
    ...

// enum_info: description after values
// set_string_parser(): use a name parser instead of a value parser
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
