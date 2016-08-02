_bjam_completion () {
  reply=($(
    sed -n -E '/^ *(unit-test|exe|install|alias|lib|explicit)/{
      s/^ *[^ ]+\s+([-_a-zA-Z0-9]+).*/\1/
      H
      /_src$/!p
    }' Jamroot
  ))
}

compctl -K _bjam_completion bjam
