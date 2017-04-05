#!/usr/bin/awk -f
/^\.\.\.skip|^Jamfile|^ *"ccache"|\.o\.\.\.$|^In/ {}
{
  if (/[^.]+\.(compile\.c\+\+|link)|^testing\.unit-test /)
  {
    ++i
    print "\x1b[32m[" int(i*100/n) "%] [" i "/" n "]\x1b[0m " $0
    fflush()
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
    fflush()
  }
  else
  {
    print
  }
}
