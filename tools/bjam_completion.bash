_bjam_completion () {
  local cur prev words cword
  _init_completion || return
  COMPREPLY=($(compgen -W '$(sed -n -E '\''
  /^ *(test-run|unit-test|exe|install|alias|lib|explicit) /{
    s/^ *[^ ]+\s+([-_a-zA-Z0-9]+).*/\1/
    H
    /_src$/!p
  }
  /^ *test-canonical /{
    s#^.+/([^.]+)\.h.*#test_\1#p
  }'\'' Jamroot)' -- "$cur" ))
}

complete -F _bjam_completion bjam
