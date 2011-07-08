#!/bin/bash
set -ex

export SWIG_SYNTAX="Syntax is ./make.sh"
export SWIG_TARGET=$1

export SWIG_MODE=make

swig -version

#----------------------
#--- install swig 2 ---
#----------------------
#if [ "`swig -version | grep 1.3`" != "" ]; then
# #yum -y install pcre-devel || :
# #apt-get -y install libpcre3-dev || :
# cd ~
# export SWIG_VER=2.0.4
# wget http://downloads.sourceforge.net/project/swig/swig/swig-$SWIG_VER/swig-$SWIG_VER.tar.gz
# tar xf swig-$SWIG_VER.tar.gz
# cd swig-$SWIG_VER
# ./configure --prefix=$HOME/local
# make && sudo make install
#fi

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
echo "could use swig's -outcurrentdir option in later versions of swig but not the one in centos5/rh5"
echo \
cp ../exodus.i .
cp ../exodus.i .

swig -version

echo Using: ${SWIG_CMD}
echo Generating Source: \
${SWIG_CMD} -c++ $SWIG_OPTIONS -$SWIG_TARGET -module $SWIG_MODULENAME $EXO_EXODUS_INCLUDE_FLAGS ../exodus.i
#${SWIG_CMD} -c++ $SWIG_OPTIONS -$SWIG_TARGET -module $SWIG_MODULENAME $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i
${SWIG_CMD} -c++ $SWIG_OPTIONS -$SWIG_TARGET -module $SWIG_MODULENAME $EXO_EXODUS_INCLUDE_FLAGS exodus.i

if [ "$SWIG_PATCH_CMD" != "" ]; then
echo
echo Patching: \
     $SWIG_PATCH_CMD
eval $SWIG_PATCH_CMD
fi

if [ "$SWIG_POSTGENERATE_CMD" != "" ]; then
echo
echo Module Generation: \
     $SWIG_POSTGENERATE_CMD
sleep 10
eval $SWIG_POSTGENERATE_CMD
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
sleep 10
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

