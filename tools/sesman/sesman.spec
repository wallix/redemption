## Config file for sesman.

[sesman]
# Display a warning when the session is recorded.
record_warning=boolean(default=True)

# Case-sensitivity on selector filters.
selector_filters_case_sensitive=boolean(default=False)

# Mode passthrough.
auth_mode_passthrough=boolean(default=False)

# Default login (for passthrough mode, disabled if empty).
default_login=string(default='')

# Debug Logs
#_advanced
debug=boolean(default=False)
