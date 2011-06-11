#!/bin/bash
#http://www.swig.org/Doc1.3/SWIGPlus.html
set -e

export EXO_PYTHON_INCLUDE="/usr/include/python2.6"
export EXO_EXODUS_INCLUDE="../exodus/exodus"
export EXO_EXODUS_LDFLAGS="-L../exodus/exodus/exodus/.libs -l exodus"

swig -c++ -python -I$EXO_EXODUS_INCLUDE exodus.i
#g++ -I/usr/include/python2.6 -fPIC -c exodus_wrap.cxx
#g++ -shared exodus_wrap.o -o exodus.so -L.libs -lexodus -lpython2.6 -Wl,-no-undefined

gcc -pthread -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -fPIC \
 -I$EXO_PYTHON_INCLUDE \
 -I$EXO_EXODUS_INCLUDE \
 -c exodus_wrap.cxx \
 -o exodus_wrap.o

g++ -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions \
 $EXO_EXODUS_LDFLAGS \
 exodus_wrap.o \
 -o _exodus.so
