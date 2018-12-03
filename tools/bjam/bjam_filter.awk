#!/usr/bin/awk -f
BEGINFILE {
    if (!columns) {
        columns = int(ENVIRON["COLUMNS"])
        if (!columns) {
            columns = 65536
        }
    }
    len = 0
    state = 0
    nb_rep=split(replacements, rep, ";")
}
{
  if (/^Running [0-9]+ test cases?\.\.\.$/)
  {
    getline
    if (!$0) {
      next
    }
  }

  if (/^\*\*\* No errors detected$|^\.\.\.skip|^Jamfile|^  *"ccache|^  *"clang|^ *"g\+\+d|^ *em\+\+|\.o\.\.\.$|^In|^  "scan-build/)
  {
    next
  }
  # else if (/[a-z]+\.(compile\.c\+\+|link)|^testing\.unit-test|qt.?\.moc /)
  else if (/^testing\.unit-test |^emscripten|[a-z]+\.(compile\.c\+\+|link)/)
  {
    for (k=1; k<nb_rep; k+=2)
    {
      idx=index($0, rep[k])
      if (idx) {
        if (idx == 1) {
          $0 = rep[k+1] substr($0, idx+length(rep[k]))
        }
        else {
          $0 = substr($0, 0, idx-1) rep[k+1] substr($0, idx+length(rep[k]))
        }
      }
    }

    ++i

    s = sprintf("\x1b[32m[%d%%] [%d/%d]\x1b[0m %s", int(i*100/n), i, n, $0)
    if (state)
    {
      k = "\x1b[K\x1b[A"
      while (len) {
          k = k "\x1b[K\x1b[A"
          --len
      }
      print k "\x1b[K" s
    }
    else
    {
      print
      state = 1
    }
    len = int((length(s) - 10) / columns)

    next
  }
  else if (/^\.\.\.on /)
  {
    i=int($2)
    next
  }

  if (/^\.\.\.updating/)
  {
    n=int($2)
    i=0;
  }

  print
  state=0
}
