#!/bin/bash
#http://www.swig.org/Doc1.3/SWIGPlus.html
set -e

export SWIG_TARGET=$1


export EXO_EXODUS_INCLUDE_FLAGS="-I../../exodus/exodus"
export EXO_EXODUS_LDFLAGS="-L../../exodus/exodus/exodus/.libs -l exodus"

#defaults
export SWIG_WRAPPER_EXT=cpp
export SWIG_MODULE_FILENAME="exodus.so"

#----------------
#--- "Target" ---
#----------------
case $SWIG_TARGET in
php )
	export SWIG_TARGET_INCLUDE_FLAGS="`php-config --includes`"
	export SWIG_TARGET_LIBDIR="`php-config --extension-dir`"
;;
python )
	export SWIG_TARGET_INCLUDE_FLAGS="`python-config --includes`"
	export SWIG_WRAPPER_EXT=cxx
	export SWIG_MODULE_FILENAME="_exodus.so"
;;
*)
	echo "Invalid or Missing SWIG target, $SWIG_TARGET"
	exit 1
;;
esac

test -d $SWIG_TARGET || mkdir $SWIG_TARGET
cd $SWIG_TARGET

#--------------
#--- "Swig" ---
#--------------
echo swig -c++ -$SWIG_TARGET $EXO_EXODUS_INCLUDE_FLAGS ../exodus.i
swig -c++ -$SWIG_TARGET -outdir $SWIG_TARGET $EXO_EXODUS_INCLUDE_FLAGS ../exodus.i

#-----------------
#--- "Compile" ---
#-----------------
echo Compiling
#g++ -I/usr/include/python2.6 -fPIC -c exodus_wrap.cxx
#g++ -shared exodus_wrap.o -o exodus.so -L.libs -lexodus -lpython2.6 -Wl,-no-undefined

gcc -pthread -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -fPIC \
 $SWIG_TARGET_INCLUDE_FLAGS \
 $SWIG_EXODUS_INCLUDE_FLAGS \
 -c exodus_wrap.$SWIG_WRAPPER_EXT \
 -o exodus_wrap.o

echo Linking
# `php-config --libs` -lpython2.6 -Wl,-no-undefined \
g++ -pthread -shared -Wl,-O1 -Wl,-Bsymbolic-functions \
 $EXO_EXODUS_LDFLAGS \
 exodus_wrap.o \
 -o  $SWIG_MODULE_FILENAME

#-----------------
#--- "Install" ---
#-----------------
echo Installing
test "$SWIG_TARGET_LIBDIR" != "" && sudo cp $SWIG_MODULE_FILENAME $SWIG_TARGET_LIBDIR

