#!/bin/bash
set -e

export SWIG_SYNTAX="Syntax is ./build.sh <action> <target> where action=make/install/all/clean target=all|csharp|java|perl|php|python"

export SWIG_MODE=$1
export SWIG_TARGET=$2

if [ "$SWIG_MODE" == "all" ]; then
	./build.sh make $2
	./build.sh install $2
	exit 0
fi

export EXO_EXODUS_INCLUDE_FLAGS="-I../../exodus/exodus"
export EXO_WRAPPER_FLAGS="-fPIC -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes"
export EXO_EXODUS_LDFLAGS="-lexodus"

#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULENAME="exodus"

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

	./build.sh $1 csharp
	./build.sh $1 java
	./build.sh $1 perl
	./build.sh $1 php
	./build.sh $1 python
	echo "all done"
	exit 0

;; php )
	export SWIG_MODULENAME="exo"
        export SWIG_TARGET_INCLUDE_FLAGS="`php-config --includes`"
        export SWIG_WRAPPER_EXT=cpp

        export SWIG_TARGET_LIBDIR="`php-config --extension-dir`"
	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"

	export SWIG_POSTGENERATE_CMD="patch exo.php ../exo.php.patch"

;; python )
        export SWIG_TARGET_INCLUDE_FLAGS="`python-config --includes`"
	export SWIG_TARGET_LDFLAGS="-l$SWIG_PYTHON_LIBCODE"

	export SWIG_TARGET_LIBFILE="_$SWIG_MODULENAME.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

        export SWIG_TARGET_MODFILE="$SWIG_MODULENAME.py*"
        export SWIG_TARGET_MODDIR="$SWIG_LOCAL_LIBDIR/$SWIG_PYTHON_LIBCODE/site-packages"

;; perl )
	export SWIG_MODULENAME="exo"
	export SWIG_TARGET_INCLUDE_FLAGS="`perl -MConfig -e 'print join(\" \", @Config{qw(ccflags optimize cccdlflags)}, \"-I$Config{archlib}/CORE\")'`"
	export SWIG_TARGET_LDFLAGS="`perl -MConfig -e 'print $Config{lddlflags}'`"

	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

        export SWIG_TARGET_MODDIR="/usr/lib/perl5"
        export SWIG_TARGET_MODFILE="$SWIG_MODULENAME.pm"

;; java )
        export SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java-6-openjdk/include -I/usr/lib/jvm/java-6-openjdk/include/linux"
        export SWIG_TARGET_LIBFILE="lib$SWIG_MODULENAME.so"

	export SWIG_MODULE_BUILD="javac *.java"
	#nb dont copy to local lib otherwise main libexodus.so will be lost

;; csharp )
        export SWIG_TARGET_INCLUDE_FLAGS=""
        export SWIG_TARGET_LIBFILE="lib$SWIG_MODULENAME.so"

	#nb dont copy to local lib otherwise main libexodus.so will be lost

;;*)
        echo "Invalid or Missing SWIG target: $SWIG_TARGET $SWIG_SYNTAX"
        exit 1
;;
esac

if [ "$SWIG_MODE" == "clean" ]; then
echo
echo ------------
echo $SWIG_TARGET $SWIG_MODE
echo ------------
test -d $SWIG_TARGET || exit 0
echo -ne "Removing: `pwd`/$SWIG_TARGET: "
rm -rf $SWIG_TARGET && echo "removed."
exit
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

if [ "$SWIG_MODE" == "make" ]; then

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

if [ "$SWIG_MODULE_BUILD" != "" ]; then
echo
echo Building: \
$SWIG_MODULE_BUILD
$SWIG_MODULE_BUILD
fi

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
