_bjam_completion () {
  local cur prev words cword
  _init_completion || return
  COMPREPLY=($(compgen -W '$(sed -n -E '\''
  /^ *(test-run|unit-test|exe(js)?|install|alias|lib|make|app) /{
    s/^ *[^ ]+\s+([-_.a-zA-Z0-9\/.]+).*/\1/
    H
    /_src$/!p
  }'\'' Jamroot;
  sed -n -E '\''/^(exe(-js)?|alias|lib|test-run) /{
    /\.coverage /d
    s/^[^ ]+\s+([-_a-zA-Z0-9\/.]+) .*/\1/p
  }'\'' targets.jam 2>/dev/null)' -- "$cur" ))
}

complete -F _bjam_completion bjam
