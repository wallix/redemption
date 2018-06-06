#!/usr/bin/awk -f
{
  if (/^\.\.\.skip|^Jamfile|^  *"ccache|^  *"clang|^ *"g\+\+d|\.o\.\.\.$|^In|^  "scan-build/)
  {}
  else if (/[^.]+\.(compile\.c\+\+|link)|^testing\.unit-test|qt.?\.moc /)
  {
    ++i
    print "\x1b[32m[" int(i*100/n) "%] [" i "/" n "]\x1b[0m " $0
  }
  else if (/^testing\.unit-test /)
  {
    ++i
  }
  else if (/^\.\.\.on /)
  {
    i=int($2)
  }
  else if (/^\.\.\.updating/)
  {
    n=int($2)
    i=0;
    print
  }
  else
  {
    print
  }
}
