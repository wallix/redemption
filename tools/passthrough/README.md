# Shared table (ACLPassthrough.shared / kv)

- Only with module selector (i.e. `kv[u'module'] = 'selector'`): `target_login`, `target_device` and `proto_dest` fields are value lists separated by a `\x01` character (i.e. `kv[u'target_device'] = 'target 1\x01target 2\x01target 3...'`).

List of modules in `src/acl/module_manager/enums.hpp` (see `STRMODULE_*`).


- To know what modules and `passthrough.py` can communicate, you can look at the beginning of `src/mod/internal/*.hpp` files:

```cpp
// src/mod/internal/flat_login_mod.hpp
// vcfg::var<field_name_in_the_proxy>
// vcfg::accessmode: what does the module do with the field
// - ::set = send the value
// - ::get = read the value
// - ::ask = ask the value
using FlatLoginModVariables = vcfg::variables<
    vcfg::var<cfg::globals::auth_user, vcfg::accessmode::set>,
    // ...
>;
```

The correspondence between the names in the proxy and `passthrough.py` are in `projects/redemption_configs/autogen/doc/sesman_dialog.txt`

```ini
...
cfg::globals::auth_user <-> login   [std::string]
...
```

This line means that the `login` field of `passthrough.py` matches the name `cfg::globals::auth_user` in the proxy.
