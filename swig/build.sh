#!/bin/bash
set -e

export SWIG_SYNTAX="Syntax is ./build.sh python|php|java|perl|all build/install/both"

export SWIG_TARGET=$1
export SWIG_MODE=$2

if [ "$2" == "both" ]; then
	./build.sh $1 build
	./build.sh $1 install
	exit 0
fi

export EXO_EXODUS_INCLUDE_FLAGS="-I../../exodus/exodus"
export EXO_WRAPPER_FLAGS="-fPIC -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes"
export EXO_EXODUS_LDFLAGS="-lexodus"
#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULE_FILEBASE="exodus"

export SWIG_LOCAL_LIBDIR=/usr/local/lib
export SWIG_OPTIONS="-w503,314,389,361,362,370,383,384"

#php    exodus.so in php extension dir
#python _exodus.si in local lib
#java   libexodus.so in local lib? but this conficts with main exodus library file name
#perl   exodus.so in local lib (also exodus.pm in usr lib perl5

#something like python2.6
export SWIG_PYTHON_LIBCODE="`python --version 2>&1|cut -d'.' -f 1,2|sed -e 's/ //;y/P/p/'`"

#----------------
#--- "Target" ---
#----------------
case $SWIG_TARGET in

   all )

	./build.sh php $2
	./build.sh python $2
	./build.sh php $2
	./build.sh java $2
	echo "all done"
	exit 0

;; php )
        export SWIG_TARGET_INCLUDE_FLAGS="`php-config --includes`"
        export SWIG_WRAPPER_EXT=cpp

        export SWIG_TARGET_LIBDIR="`php-config --extension-dir`"
	export SWIG_TARGET_LIBFILE="$SWIG_MODULE_FILEBASE.so"

;; python )
        export SWIG_TARGET_INCLUDE_FLAGS="`python-config --includes`"
	export SWIG_TARGET_LDFLAGS="-l$SWIG_PYTHON_LIBCODE"

	export SWIG_TARGET_LIBFILE="_$SWIG_MODULE_FILEBASE.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

        export SWIG_TARGET_MODFILE="$SWIG_MODULE_FILEBASE.py*"
        export SWIG_TARGET_MODDIR="$SWIG_LOCAL_LIBDIR/$SWIG_PYTHON_LIBCODE/site-packages"

;; perl )
	export SWIG_TARGET_INCLUDE_FLAGS="`perl -MConfig -e 'print join(\" \", @Config{qw(ccflags optimize cccdlflags)}, \"-I$Config{archlib}/CORE\")'`"
	export SWIG_TARGET_LDFLAGS="`perl -MConfig -e 'print $Config{lddlflags}'`"

	export SWIG_TARGET_LIBFILE="$SWIG_MODULE_FILEBASE.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

        export SWIG_TARGET_MODDIR="/usr/lib/perl5"
        export SWIG_TARGET_MODFILE="$SWIG_MODULE_FILEBASE.pm"

;; java )
        export SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java-6-openjdk/include -I/usr/lib/jvm/java-6-openjdk/include/linux"
        export SWIG_TARGET_LIBFILE="lib$SWIG_MODULE_FILEBASE.so"

	#nb dont copy to local lib otherwise main libexodus.so will be lost

;;*)
        echo "Invalid or Missing SWIG target: $SWIG_TARGET $SWIG_SYNTAX"
        exit 1
;;
esac

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

if [ "$SWIG_MODE" == "build" ]; then

#--------------------
#--- "SWIG MAGIC" ---
#--------------------
echo
echo Generating: \
swig -c++ $SWIG_OPTIONS -$SWIG_TARGET $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i
swig -c++ $SWIG_OPTIONS -$SWIG_TARGET $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i

#-----------------
#--- "Compile" ---
#-----------------
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

test "$SWIG_MODE" != "install" && echo "$SWIG_MODE is invalid. $SWIG_SYNTAX" && exit 1

#-----------------
#--- "Install" ---
#-----------------
if [ "$SWIG_TARGET_LIBDIR" != "" ]; then
	echo
	echo -ne "Installing $SWIG_TARGET library: "
	echo sudo cp -f $SWIG_TARGET_LIBFILE $SWIG_TARGET_LIBDIR/
	     sudo cp -f $SWIG_TARGET_LIBFILE $SWIG_TARGET_LIBDIR/
fi
if [ "$SWIG_TARGET_MODDIR" != "" ]; then
	echo
	echo -ne "Installing $SWIG_TARGET module: "
	echo sudo cp -f $SWIG_TARGET_MODFILE $SWIG_TARGET_MODDIR/
	     sudo cp -f $SWIG_TARGET_MODFILE $SWIG_TARGET_MODDIR/
fi
sleep 1

fi
