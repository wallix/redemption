rdpproxy communicates with a server to know which module when a user connects. The most common module is RDP which represents a connection to an RDP server, but there are others like VNC, selector, interactive_target, etc. `passthrough.py` is a minimal implementation of this server and offers interactive authentication to a remote target.

Feel free to modify it to fit your needs. For example, to enable recording, uncomment the line `kv['is_rec'] = '1'`.


# General structure

`passthrough.py` contains essentially 2 classes:

- `AuthentifierSharedData` which is used to receive and send data to rdpproxy. The `shared` member represents a dictionary of exchanged values. It is updated when `receive_data()` or `send_data()` is called. A direct modification of the `shared` member does not send any data to the proxy.
- The `ACLPassthrough` class is the main class that takes care of selecting and configuring the module used by rdpproxy. The `start()` function is the one you should modify.


# Parameters that can be exchanged

The values that can be exchanged between the proxy and `passthrough.py` are visible in `projects/redemption_configs/autogen/doc/sesman_dialog.txt`:

```ini
...
cfg::mod_rdp::disabled_orders ⇐ mod_rdp:disabled_orders [std::string]
...
```

This means that `passthrough.py` can send the key `mod_rdp:disabled_orders` to the proxy. `cfg::mod_rdp::disabled_orders` corresponds to the internal name, which usually corresponds to the name found in the ini file: `mod_rdp` is a section and `disabled_orders` is a parameter. The direction of the arrow indicates who can receive the parameter.

For various reasons, not all values have a perfect match and you have to refer to the `projects/redemption_configs/configs_specs/configs/specs/config_spec.hpp` file to get them.

The important parts are `W.section()` which defines a section and `W.member()` which describes a value in the section.

The parameters to remember are:

- `no_sesman` / `sesman_rw` / `sesman_to_proxy` / `proxy_to_sesman`: indicates whether parameter is sent and/or received by `passthrough.py`. The name sesman refers here to passthrough.
- `names{...}`: The name of the parameter. If this is of the form `names{..., .sesman="login"}`, then the name exchanged between passthrough and the proxy is `login`. Otherwise it is of the form `${section}:${param_name}`.


# Modules

List of modules in `src/acl/module_manager/enums.hpp` (see `ModuleName::*`).

To know what modules and `passthrough.py` can communicate, you can look at the beginning of `src/mod/internal/*.hpp` files:

```cpp
// src/mod/internal/flat_login_mod.hpp
// vcfg::var<field_name_in_the_proxy>
// vcfg::accessmode: what does the module do with the field
// - ::set = module can write the value
// - ::get = module can read the value
// - ::ask = module can request the value (verified with AuthentifierSharedData.is_asked() in passthrough.py)
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

## Selector module

The `selector` module (i.e. `kv[u'module'] = 'selector'`) allows the proxy to display a list of targets. At this time, `target_login`, `target_device` and `proto_dest` fields are value lists separated by a `\x01` character (i.e. `kv[u'target_device'] = 'target 1\x01target 2\x01target 3...'`).

A commented example is available in the `start()` function.
