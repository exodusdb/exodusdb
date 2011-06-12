#!/bin/bash
set -e

export SWIG_TARGET=$1

export EXO_EXODUS_INCLUDE_FLAGS="-I../../exodus/exodus"
export EXO_EXODUS_LDFLAGS="-lexodus"
#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULE_FILEBASE="exodus"

export SWIG_LOCAL_LIBDIR=/usr/local/lib

#php    exodus.so in php extension dir
#python _exodus.si in local lib
#java   libexodus.so in local lib? but this conficts with main exodus library file name
#perl   exodus.so in local lib (also exodus.pm in usr lib perl5

#----------------
#--- "Target" ---
#----------------
case $SWIG_TARGET in

   all )

	./build.sh php
	./build.sh python
	./build.sh php
	./build.sh java
;; php )
        export SWIG_TARGET_INCLUDE_FLAGS="`php-config --includes`"
        export SWIG_TARGET_LIBDIR="`php-config --extension-dir`"
        export SWIG_WRAPPER_EXT=cpp
	export SWIG_MODULE_FILENAME="$SWIG_MODULE_FILEBASE.so"

;; python )
        export SWIG_TARGET_INCLUDE_FLAGS="`python-config --includes`"
	export SWIG_TARGET_LDFLAGS="-lpython2.6"

	export SWIG_MODULE_FILENAME="_$SWIG_MODULE_FILEBASE.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

;; perl )
	export SWIG_TARGET_INCLUDE_FLAGS="`perl -MConfig -e 'print join(\" \", @Config{qw(ccflags optimize cccdlflags)}, \"-I$Config{archlib}/CORE\")'`"
	export SWIG_TARGET_LDFLAGS="`perl -MConfig -e 'print $Config{lddlflags}'`"

	export SWIG_MODULE_FILENAME="$SWIG_MODULE_FILEBASE.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

        export SWIG_PERLMODULE_DIR="/usr/lib/perl5"
        export SWIG_PERLMODULE_FILENAME="$SWIG_MODULE_FILEBASE.pm"

;; java )
        export SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java-6-openjdk/include -I/usr/lib/jvm/java-6-openjdk/include/linux"
        export SWIG_MODULE_FILENAME="lib$SWIG_MODULE_FILEBASE.so"
	#nb dont copy to local lib otherwise main libexodus.so will be lost

;;*)
        echo "Invalid or Missing SWIG target, $SWIG_TARGET"
	echo "Syntax is ./build.sh python|php|java|perl|all"
        exit 1
;;
esac

#------------
#--- "cd" ---
#------------

test -d $SWIG_TARGET || mkdir $SWIG_TARGET
cd $SWIG_TARGET

#--------------
#--- "Swig" ---
#--------------
echo swig -c++ -$SWIG_TARGET $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i
     swig -c++ -$SWIG_TARGET $EXO_EXODUS_INCLUDE_FLAGS -outcurrentdir ../exodus.i

#-----------------
#--- "Compile" ---
#-----------------
echo Compiling
#g++ -I/usr/include/python2.6 -fPIC -c exodus_wrap.cxx
#g++ -shared exodus_wrap.o -o exodus.so -L.libs -lexodus -lpython2.6 -Wl,-no-undefined


gcc -c exodus_wrap.$SWIG_WRAPPER_EXT -fPIC $SWIG_TARGET_INCLUDE_FLAGS $EXO_EXODUS_INCLUDE_FLAGS

g++ -shared exodus_wrap.o -o $SWIG_MODULE_FILENAME $EXO_EXODUS_LDFLAGS $SWIG_TARGET_LDFLAGS


#-----------------
#--- "Install" ---
#-----------------
if [ "$SWIG_TARGET_LIBDIR" != "" ]; then
	echo sudo cp -f $SWIG_MODULE_FILENAME $SWIG_TARGET_LIBDIR/
	     sudo cp -f $SWIG_MODULE_FILENAME $SWIG_TARGET_LIBDIR/
fi
if [ "$SWIG_PERL_MODULEDIR" != "" ]; then
	echo sudo cp -f $SWIG_PERLMODULE_FILENAME $SWIG_PERLMODULE_DIR/
	     sudo cp -f $SWIG_PERLMODULE_FILENAME $SWIG_PERLMODULE_DIR/
fi

