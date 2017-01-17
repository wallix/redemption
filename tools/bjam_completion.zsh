_bjam_completion () {
  reply=($(
    sed -n -E '
    /^ *(test-run|unit-test|exe|install|alias|lib|explicit) /{
      s/^ *[^ ]+\s+([-_a-zA-Z0-9]+).*/\1/
      H
      /_src$/!p
    }
    /^ *test-canonical /{
        s#^.+/([^.]+)\.h.*#test_\1#p
    }' Jamroot
  ))
}

compctl -K _bjam_completion -M 'r:|[_[:lower:]]=** r:|=*' bjam
