_bjam_completion () {
  reply=($(
    sed -n -E '
    /^ *(test-run|unit-test|exe|install|alias|lib|make) /{
      s/^ *[^ ]+\s+([-_.a-zA-Z0-9\/.]+).*/\1/
      H
      /_src$/!p
    }' Jamroot
    
    sed -n -E '/^(alias|test-run|exe|lib) /{
        /\.coverage /d
        s/^[^ ]+\s+([-_a-zA-Z0-9\/.]+) .*/\1/p
    }' targets.jam 2>/dev/null
  ))
}

compctl -K _bjam_completion -M 'r:|[_/]=** r:|=*' bjam
