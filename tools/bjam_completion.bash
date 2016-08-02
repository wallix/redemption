_bjam_completion () {
  local cur prev words cword
  _init_completion || return
  COMPREPLY=($(compgen -W '$(sed -n -E '\''/^ *(unit-test|exe|install|alias|lib|explicit)/{
    s/^ *[^ ]+\s+([-_a-zA-Z0-9]+).*/\1/
    H
    /_src$/!p
  }'\'' Jamroot)' -- "$cur" ))
}

complete -F _bjam_completion bjam
