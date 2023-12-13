#!/bin/bash

bjam -aj2 cxxflags='-gdwarf-2 -g3 -O0 -fno-builtin' linkflags='-Wl,--version-script=veracode.expmap' rdpproxy
read EXE_PATH < <(ls -1t bin/*/*/rdpproxy)
tar -cvf vera-redemption.tar "$EXE_PATH"
