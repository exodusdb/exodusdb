#!/bin/bash
set -e

export SWIG_SYNTAX="Syntax is ./make.sh"
export SWIG_TARGET=$1

export EXO_EXODUS_INCLUDE_FLAGS="-I../../exodus/exodus"
export EXO_WRAPPER_FLAGS="-fPIC -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes"
export EXO_EXODUS_LDFLAGS="-lexodus"

#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULENAME="exodus"

export SWIG_LOCAL_LIBDIR=/usr/local/lib
export SWIG_OPTIONS="-w503,314,389,361,362,370,383,384"

export SWIG_MODE=make

source config.sh
if [ "$SWIG_TARGET" == "all" ]; then
	for  SWIG_TARGET in $SWIG_ALL_TARGETS; do
		./make.sh $SWIG_TARGET
	done
	echo "all done"
	exit 0
fi

#------------
#--- "cd" ---
#------------
test -d $SWIG_TARGET || mkdir $SWIG_TARGET
cd $SWIG_TARGET

echo
echo ------------
echo $SWIG_TARGET $SWIG_MODE
echo ------------
echo Entering: `pwd`


#if java, create jar class directory
if [ "$SWIG_PACKAGE_SUBDIR" != "" ]; then
test -d $SWIG_PACKAGE_SUBDIR || mkdir -pv $SWIG_PACKAGE_SUBDIR
fi

#--------------------
#--- "SWIG MAGIC" ---
#--------------------
echo
echo Generating: \
swig -c++ $SWIG_OPTIONS -$SWIG_TARGET -module $SWIG_MODULENAME $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i
swig -c++ $SWIG_OPTIONS -$SWIG_TARGET -module $SWIG_MODULENAME $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i

if [ "$SWIG_POSTGENERATE_CMD" != "" ]; then
echo
echo Post Generation: \
$SWIG_POSTGENERATE_CMD
$SWIG_POSTGENERATE_CMD
fi

#-----------------
#--- "Compile" ---
#-----------------
if [ "$SWIG_MODULE_COMPILE" == "" ]; then

#g++ -I/usr/include/python2.6 -fPIC -c exodus_wrap.cxx
#g++ -shared exodus_wrap.o -o exodus.so -L.libs -lexodus -lpython2.6 -Wl,-no-undefined
echo
echo Compiling: \
gcc -c exodus_wrap.$SWIG_WRAPPER_EXT $SWIG_TARGET_INCLUDE_FLAGS $EXO_EXODUS_INCLUDE_FLAGS $EXO_WRAPPER_FLAGS
gcc -c exodus_wrap.$SWIG_WRAPPER_EXT $SWIG_TARGET_INCLUDE_FLAGS $EXO_EXODUS_INCLUDE_FLAGS $EXO_WRAPPER_FLAGS

echo
echo Linking: \
g++ -shared exodus_wrap.o -o $SWIG_TARGET_LIBFILE $EXO_EXODUS_LDFLAGS $SWIG_TARGET_LDFLAGS
g++ -shared exodus_wrap.o -o $SWIG_TARGET_LIBFILE $EXO_EXODUS_LDFLAGS $SWIG_TARGET_LDFLAGS

else

echo
echo Compiling: \
$SWIG_MODULE_COMPILE
$SWIG_MODULE_COMPILE

fi

if [ "$SWIG_MODULE_BUILD" != "" ]; then
echo
echo Building: \
$SWIG_MODULE_BUILD
$SWIG_MODULE_BUILD
fi

