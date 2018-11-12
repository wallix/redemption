#!/usr/bin/awk -f
BEGINFILE {
    if (!columns) {
        columns = int(ENVIRON["COLUMNS"])
        if (!columns) {
            columns = 65536
        }
    }
    len = 0
}
{
  if (/^\x1b\[32m\[/)
  {
    # 10 for \x1b[32m[ \x1b[0m
    len = int((length($0) - 10) / columns)
    printf "%s", $0
    if (getline) {
      while (/^\x1b\[32m\[/)
      {
        k = "\r"
        while (len) {
          k = k "\x1b[K\x1b[A"
          --len
        }
        len = int((length($0) - 10) / columns)
        printf "%s%s\x1b[K", k, $0

        if (!getline) {
          print ""
          next
        }
      }
      print "\n\n" $0
    }
    else
    {
      print ""
    }
  }
  else
  {
    print
  }
}
