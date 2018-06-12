_bjam_completion () {
  reply=($(
    sed -n -E '
    /^ *(test-run|unit-test|exe|install|alias|lib|make) /{
      s/^ *[^ ]+\s+([-_.a-zA-Z0-9\/.]+).*/\1/
      H
      /_src$/!p
    }
    /^ *test-canonical /{
        s#^.+/([^.]+)\.h.*#test_\1#p
    }' Jamroot
    
    sed -n -E '/^(exe|alias|lib) /{
        /\.coverage /d
        s/^[^ ]+\s+([-_a-zA-Z0-9\/.]+) .*/\1/p
    }' targets.jam 2>/dev/null
  ))
}

compctl -K _bjam_completion -M 'r:|[_/]=** r:|=*' bjam
