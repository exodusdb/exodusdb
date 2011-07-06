#!/bin/bash
set -ex

export SWIG_SYNTAX="Syntax is ./install.sh "

export SWIG_TARGET=$1

source config.sh

if [ "$SWIG_TARGET" == "all" ]; then
	for  SWIG_TARGET in $SWIG_ALL_TARGETS; do
		./install.sh $SWIG_TARGET
	done
        echo "all done"
        exit 0
fi

#------------
#--- "cd" ---
#------------
test -d $SWIG_TARGET || ( echo "Make $SWIG_TARGET first" && exit 1 )
cd $SWIG_TARGET

echo
echo --------------------
echo $SWIG_TARGET install
echo --------------------
echo Entering: `pwd`

#-----------------
#--- "Install" ---
#-----------------

echo $PHPINIFILES

if [ "$SWIG_TARGET_LIBDIR" != "" ]; then
	echo
	echo -ne "Installing $SWIG_TARGET library: "
	#test -d ${SWIG_TARGET_LIBDIR}64 && ( test -L ${SWIG_TARGET_LIBDIR}64 || export SWIG_TARGET_LIBDIR=${SWIG_TARGET_LIBDIR}64 )
	if [ -d ${SWIG_TARGET_LIBDIR}64 ] && [ ! -L ${SWIG_TARGET_LIBDIR}64 ]; then export SWIG_TARGET_LIBDIR=${SWIG_TARGET_LIBDIR}64 ;fi
	test -d ${SWIG_DESTDIR}$SWIG_TARGET_LIBDIR || mkdir -p ${SWIG_DESTDIR}$SWIG_TARGET_LIBDIR
	echo cp -f $SWIG_TARGET_LIBFILE ${SWIG_DESTDIR}$SWIG_TARGET_LIBDIR/
	     cp -f $SWIG_TARGET_LIBFILE ${SWIG_DESTDIR}$SWIG_TARGET_LIBDIR/
fi

if [ "$SWIG_TARGET_MODDIR" != "" ]; then
	echo
	echo -ne "Installing $SWIG_TARGET module: "
	test -d ${SWIG_DESTDIR}$SWIG_TARGET_MODDIR || mkdir -p ${SWIG_DESTDIR}$SWIG_TARGET_MODDIR
	echo cp -f $SWIG_TARGET_MODFILE ${SWIG_DESTDIR}$SWIG_TARGET_MODDIR/
	     cp -f $SWIG_TARGET_MODFILE ${SWIG_DESTDIR}$SWIG_TARGET_MODDIR/
fi

if [ "$SWIG_MODULE_INSTALL" != "" ]; then
	echo
	echo -ne "Installing Module: "
	echo eval $SWIG_MODULE_INSTALL
	     eval $SWIG_MODULE_INSTALL
fi

sleep 1
