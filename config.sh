#!/bin/bash

#assumes icu, boost and exodus are in the home directory

#---------------------------------------------------------
#--- dev.cmd, make.cmd, clean.cmd, pack.cmd and upload.cmd
#---  all call this config.cmd to initialise           ---
#---------------------------------------------------------

#--------------------------------------------------------------
#--- your local configuration is in configlocal.cmd         ---
#--- (copy configlocalEXAMPLE.cmd TO configlocal.cmd)       ---
#--------------------------------------------------------------

export EXO_CONFIGMODE=$1
set -e

test -f ./version.sh     && source ./version.sh
test -f ./configlocal.sh && source ./configlocal.sh

#-------------------
#--- Run options ---
#-------------------
export EXO_ICU_REUSE_DOWNLOAD=YES
export EXO_BOOST_REUSE_DOWNLOAD=YES
export EXO_EXODUS_REUSE_DOWNLOAD=YES

#-------------------------------
#--- Batch Run Configuration ---
#-------------------------------

test "$EXO_BATCH_MAJOR_VER" != "" && export EXO_MAJOR_VER=%EXO_BATCH_MAJOR_VER%
test "$EXO_BATCH_MINOR_VER" != "" && export EXO_MINOR_VER=%EXO_BATCH_MINOR_VER%
test "$EXO_BATCH_MICRO_VER" != "" && export EXO_MICRO_VER=%EXO_BATCH_MICRO_VER%
test "$EXO_BATCH_BUILD_VER" != "" && export EXO_BUILD_VER=%EXO_BATCH_BUILD_VER%

test "$BATCH_TARGET_CPU"    != "" && export TARGET_CPU=%BATCH_TARGET_CPU%
test "$BATCH_RELEASEDEBUG"  != "" && export CONFIGURATION=%BATCH_RELEASEDEBUG%
test "$EXO_BATCH_TOOLSET"   != "" && export EXO_TOOLSET=%EXO_BATCH_TOOLSET%

#test "$EXO_BATCH_POSTGRES_VER" != "" && export EXO_POSTGRES_VER=%EXO_BATCH_POSTGRES_VER%
#test "$EXO_BATCH_BOOST_VER"    != "" && export EXO_BOOST_VER=%EXO_BATCH_BOOST_VER%

#----------------
#--- Defaults ---
#----------------

test "$EXO_MAJOR_VER" == "" && export EXO_MAJOR_VER=0
test "$EXO_MINOR_VER" == "" && export EXO_MINOR_VER=0
test "$EXO_MICRO_VER" == "" && export EXO_MICRO_VER=0
test "$EXO_BUILD_VER" == "" && export EXO_BUILD_VER=0

#-------------
#--- Boost ---
#-------------
export EXO_BOOST_VER=1_46_1
export EXO_BOOST_VERNO=1.46.1
export EXO_BOOST_FILENAME=boost_${EXO_BOOST_VER}.tar.gz
export EXO_BOOST_URL_FILE=http://sourceforge.net/projects/boost/files/boost/${EXO_BOOST_VERNO}/${EXO_BOOST_FILENAME}
export EXO_BOOST_DIR=boost_${EXO_BOOST_VER}

#--- osdetection ---
export EXO_UNAME=`uname`
export EXO_OSNAME=`lsb_release -is`
test "$EXO_OSNAME" == "" && export EXO_OSNAME=`head -n 1 /etc/issue|cut -d' ' -f 1`
test "`which dpkg 2>/dev/null`" != "" &&export EXO_OSBASE="Debian"
test "`which yum 2>/dev/null`" != "" &&export EXO_OSBASE="Redhat"

#--- default build target or fail ---
if [ "$EXO_BUILD_TARGET" = "" ]; then
	test "$EXO_OSBASE" == "Redhat" && export EXO_BUILD_TARGET=Redhat
	test "$EXO_OSBASE" == "Debian" && export EXO_BUILD_TARGET=Debian
	#10.4/10.5/10.6
	test "$EXO_UNAME"  == "Darwin" && export EXO_BUILD_TARGET=10.6
fi
if [ "$EXO_BUILD_TARGET" = "" ]; then
	echo "Missing \$EXO_BUILD_TARGET"
	errorhere
	exit
fi

#---------------
#--- Toolset ---
#---------------
#Some influential environment variables:
#  CC          C compiler command
#  CFLAGS      C compiler flags
#  LDFLAGS     linker flags, e.g. -L<lib dir> if you have libraries in a
#              nonstandard directory <lib dir>
#  LIBS        libraries to pass to the linker, e.g. -l<library>
#  CPPFLAGS    (Objective) C/C++ preprocessor flags, e.g. -I<include dir> if
#              you have headers in a nonstandard directory <include dir>
#  CPP         C preprocessor
#  CXX         C++ compiler command
#  CXXFLAGS    C++ compiler flags
#  CXXCPP      C++ preprocessor

if [ "$EXO_UNAME" == "Linux" ]; then
	export EXO_BOOST_JAM_ARCHITECTURE=""
	export EXO_BOOST_JAM_ADDRESS_MODEL="`uname -m`"
	export EXO_BOOST_JAM_USING="gcc : : :"
fi

if [ "$EXO_UNAME" = "Darwin" ]; then

#export EXO_SDK=/Developer/SDKs/MacOSX10.5.sdk

# OSX 10.6

if [ "$EXO_BUILD_TARGET" = "10.6" ]; then
 echo BUILDING DARWIN $EXO_BUILD_TARGET 
 export EXO_SDK=/Developer/SDKs/MacOSX10.6.sdk
 export EXO_MINVER=10.6
 export EXO_OSCODENAME=Snow
 export EXO_ARCH=x86_64
 export EXO_BUILD=x86_64-apple-darwin10.0.0
 export EXO_CC=gcc-4.2
 export EXO_CXX=g++-4.2

 export EXO_BOOST_JAM_USING="darwin : 10.0 : /usr/bin/g++-4.2 :"
 export EXO_BOOST_JAM_ARCHITECTURE=combined
 export EXO_BOOST_JAM_ADDRESS_MODEL=64
 #export EXO_BOOST_JAM_ADDRESS_MODEL=32_64
fi

# OSX 10.5

if [ "$EXO_BUILD_TARGET" = "10.5" ]; then
 echo BUILDING DARWIN $EXO_BUILD_TARGET 
 export EXO_SDK=/Developer/SDKs/MacOSX10.5.sdk
 export EXO_MINVER=10.5
 export EXO_OSCODENAME=Leopard
 export EXO_ARCH=i386
 export EXO_BUILD=i386-apple-darwin9.0.0
 export EXO_CC=gcc-4.0
 export EXO_CXX=g++-4.0

 export EXO_BOOST_JAM_USING="darwin : 9.0 : /usr/bin/g++-4.0 :"
 export EXO_BOOST_JAM_ARCHITECTURE=combined
 export EXO_BOOST_JAM_ADDRESS_MODEL=32
 #export EXO_BOOST_JAM_ADDRESS_MODEL=32_64
fi

# OSX 10.4

if [ "$EXO_BUILD_TARGET" = "10.4" ]; then
 echo BUILDING DARWIN $EXO_BUILD_TARGET 
 export EXO_SDK=/Developer/SDKs/MacOSX10.4u.sdk #doesnt have backtrace
 export EXO_MINVER=10.4
 export EXO_OSCODENAME=Tiger
 export EXO_ARCH=i386
 export EXO_BUILD=i386-apple-darwin8.11.0
 export EXO_CC=gcc-4.0
 export EXO_CXX=g++-4.0

 export EXO_BOOST_JAM_USING="darwin : 8.11 : /usr/bin/g++-4.0 :"
 export EXO_BOOST_JAM_ARCHITECTURE=combined
 export EXO_BOOST_JAM_ADDRESS_MODEL=32
 #export EXO_BOOST_JAM_ADDRESS_MODEL=32_64
fi

#determine which include files and libs are used from /Developer/SDKs
#export EXO_OSX_FLAGS="-arch $EXO_ARCH -mmacosx-version-min=$EXO_MINVER -march=prescott -isysroot $EXO_SDK"
export EXO_OSX_FLAGS="-arch $EXO_ARCH -mmacosx-version-min=$EXO_MINVER -isysroot $EXO_SDK"

#see XCODE's Cross-Development Programming Guide:Configuring a Makefile-Based Project
export MACOSX_DEPLOYMENT_TARGET=$EXO_MINVER

#end of Darwin
fi

# --- common to all tools ---

test "$EXO_MINVER" = "" && export EXO_MINVER=$EXO_OSBASE
test "$EXO_ARCH" = ""   && export EXO_ARCH=`uname -m`

export EXO_ICU_PREFIX=HOME
export EXO_BOOST_PREFIX=HOME
export EXO_EXODUS_PREFIX=DEBIAN

#static files for now
export EXO_ARCH_DIR=$EXO_MINVER-$EXO_ARCH-s

#-s for stat#mimic /usr/local
test "$EXO_ICU_PREFIX"      == "HOME"   && export EXO_ICU_PREFIX=$HOME/local && export EXO_ICU_EPREFIX=$EXO_ICU_PREFIX/$EXO_ARCH_DIR
test "$EXO_BOOST_PREFIX"    == "HOME"   && export EXO_BOOST_PREFIX=$HOME/local && export EXO_BOOST_EPREFIX=$EXO_BOOST_PREFIX/$EXO_ARCH_DIR
test "$EXO_EXODUS_PREFIX"   == "HOME"   && export EXO_EXODUS_PREFIX=$HOME/local && export EXO_EXODUS_EPREFIX=$EXO_EXODUS_PREFIX/$EXO_ARCH_DIR
test "$EXO_EXODUS_PREFIX"   == "DEBIAN" && export EXO_EXODUS_PREFIX=`pwd`/debian/exodusmvdb/usr/local  && export EXO_EXODUS_EPREFIX=$EXO_EXODUS_PREFIX

export EXO_LIBS_ICU="-licudata -licui18n -licutu -licuuc"
#if static linking then libicu needs to link against libdl to avoid "undefined reference to `dlopen'" when building boost
export EXO_LIBS_ICU="$EXO_LIBS_ICU -ldl"
#added -lgcc to avoid - hidden symbol `icu_48::Locale::~Locale()' in /home/neosys/local/Debian-x86_64-s/lib/libicuuc.a(locid.ao) is referenced by DSO
export EXO_LIBS_ICU="$EXO_LIBS_ICU -lgcc"

export EXO_LIBS_BOOST="-lboost_date_time -lboost_filesystem -lboost_regex -lboost_system -lboost_thread"
export EXO_LIBS_BOOST="-Wl,-Bstatic $EXO_LIBS_BOOST -Wl,-Bdynamic"

#!!! -fPIC -DPIC is needed to compile static libraries in order to be linked into SHARED libraries (eg libexodus is SHARED)
export EXO_FLAGS="-fPIC -DPIC -DU_STATIC_IMPLEMENTATION=1"
export EXO_LDFLAGS="-L$EXO_ICU_EPREFIX/lib"
if [ "$EXO_UNAME" == "Darwin" ]; then
	export EXO_FLAGS="$EXO_FLAGS $EXO_OSX_FLAGS"
	export EXO_LDFLAGS="-Bstatic -L$EXO_LDFLAGS"
fi

#--- ICU ---
export EXO_ICU_FLAGS="$EXO_FLAGS"
#export EXO_ICU_FLAGS="$EXO_FLAGS -fvisibility=default"
export EXO_ICU_LDFLAGS="$EXO_LDFLAGS"
export EXO_ICU_LIBS=""

#--- BOOST ---
#note: any minor failure to compile can cause boost not to detect ICU see boosts bin.v2/config.log for errors
export EXO_BOOST_FLAGS="$EXO_FLAGS -I$EXO_ICU_PREFIX/include"
export EXO_BOOST_LDFLAGS="$EXO_LDFLAGS -L$EXO_ICU_EPREFIX/lib"
export EXO_BOOST_LIBS="$EXO_ICU_LIBS $EXO_LIBS_ICU"

export EXO_EXODUS_FLAGS="$EXO_BOOST_FLAGS -I$EXO_BOOST_PREFIX/include -I$HOME/$EXO_BOOST_DIR"
export EXO_EXODUS_LDFLAGS="$EXO_BOOST_LDFLAGS -L$EXO_BOOST_EPREFIX/lib"
export EXO_EXODUS_LIBS="$EXO_BOOST_LIBS $EXO_LIBS_BOOST"

##note: any minor failure to compile can cause boost not to detect ICU see boosts bin.v2/config.log for errors
##!!! need to compile static libraries with -fPIC -DPIC in order to be linked into SHARED libraries (eg libexodus is SHARED)
#export EXO_FLAGS="-fPIC -DPIC -I$EXO_ICU_PREFIX/include -I$HOME/$EXO_BOOST_DIR $EXO_OSX_FLAGS -DU_STATIC_IMPLEMENTATION=1"
#if [ "$EXO_UNAME" == "Darwin" ]; then
#	export EXO_LDFLAGS="-Bstatic -L$EXO_EPREFIX/lib"
#else
#	export EXO_LDFLAGS="-L$EXO_EPREFIX/lib"
#	#export "-Wl,-Bstatic -lsomestaticlib -Wl,-Bdynamic"
#fi

#-----------
#--- Icu ---
#-----------
export EXO_ICU_DOWNLOADER=SUBVERSION
export EXO_ICU_VER=4_8
export EXO_ICU_URL_SUBVERSION=http://source.icu-project.org/repos/icu/icu/tags/release-4-8/
export EXO_ICU_URL_FILE=http://download.icu-project.org/files/icu4c/4.8/icu4c-4_8-src.tgz

export EXO_ICU_FILENAME=icu4c-4_8-src.tgz
export EXO_ICU_DIR=icu

#----------------
#--- Postgres ---
#----------------
export EXO_POSTGRES_BIN32=/Library/PostgreSQL/9.0/bin
export EXO_POSTGRES_BIN=$EXO_POSTGRES_BIN32

#--------------
#--- Exodus ---
#--------------
export EXO_EXODUS_REUSE_DOWNLOAD=YES
export EXO_EXODUS_DOWNLOADER=SUBVERSION
export EXO_EXODUS_URL_SUBVERSION=HTTPS://exodusdb.googlecode.com/svn/trunk/
export EXO_EXODUS_DIR=exodus
export EXO_EXODUS_FILENAME=
export EXO_EXODUS_URL_FILE=

export EXO_CODENAME=exodus
export EXO_CODEWORD=Exodus
export EXO_GOOGLECODE_PROJECTCODE=exodusdb
export EXO_DEBIAN_PKGCODE=exodusmvdb

export PATH=/Library/PostgreSQL/9.0/bin:$PATH

test "$EXO_BATCH_MAJOR_VER" = "" || export EXO_MAJOR_VER=$EXO_BATCH_MAJOR_VER
test "$EXO_BATCH_MINOR_VER" = "" || export EXO_MINOR_VER=$EXO_BATCH_MINOR_VER
test "$EXO_BATCH_MICRO_VER" = "" || export EXO_MICRO_VER=$EXO_BATCH_MICRO_VER
test "$EXO_BATCH_BUILD_VER" = "" || export EXO_BUILD_VER=$EXO_BATCH_BUILD_VER

export EXO_DOTTED_MINOR_VER=$EXO_MAJOR_VER.$EXO_MINOR_VER
export EXO_DOTTED_MICRO_VER=$EXO_MAJOR_VER.$EXO_MINOR_VER.$EXO_MICRO_VER

# --------------------
# --- CLEAN COMMAND ---
# --------------------
  export EXO_CLEAN_CMD="make clean"
  export EXO_CLEAN_OPT=
  export EXO_DISTCLEAN_CMD="make distclean"
  export EXO_DISTCLEAN_OPT=

# --------------------
# --- MAKE COMMAND ---
# --------------------
#if [ $EXO_UNAME == "Darwin" ]; then
#fi
  export EXO_CONFIGURE_CMD=./configure
  export EXO_CONFIGURE_OPT=
  export EXO_MAKE_CMD=make
if [ $EXO_OSBASE == "Debian" ]; then
	export EXO_CONFIGURE_OPT="--prefix=`pwd`/debian/$EXO_DEBIAN_PKGCODE/usr/local"
fi

# -----------------------
# --- INSTALL COMMAND ---
# -----------------------
export EXO_INSTALL_CMD="make"
export EXO_INSTALL_OPT="install"

# ------------------------
# --- INSTALL_FILE NAME ---
# ------------------------
export EXO_INSTALL_FILENAME="$EXO_CODENAME-$EXO_DOTTED_MICRO_VER.tar.gz"

if [ "$EXO_UNAME" = "Darwin" ]; then
	export EXO_INSTALL_APPDIR=${EXO_CODENAME}-${EXO_DOTTED_MICRO_VER}-osx-${EXO_OSCODENAME}-installer.app
	export EXO_INSTALL_FILENAME=${EXO_INSTALL_APPDIR}.zip
	export EXO_GOOGLECODE_SUMMARY=Mac_OSX_${EXO_OSCODENAME}_Installer
	export EXO_GOOGLECODE_LABELS=Type-Installer,OpSys-OSX,Featured
fi
if [ $EXO_OSBASE == "Debian" ]; then
	export EXO_ARCH=`uname -m`
	export EXO_INSTALL_FILENAME=${EXO_DEBIAN_PKGCODE}-${EXO_DOTTED_MICRO_VER}-${EXO_ARCH}.deb
	export EXO_GOOGLECODE_SUMMARY=Debian_Package
	export EXO_GOOGLECODE_LABELS=Type-Installer,OpSys-Linux
fi

#if [ "$EXO_CONFIGMODE" == "CLEAN" ] goto afteruploader
#if [ "$EXO_CONFIGMODE" == "MAKE"  ] goto afteruploader
#if [ "$EXO_CONFIGMODE" == "DEV"   ] goto afteruploader

# -------------------------
# --- COMMAND PACKER UI ---
# -------------------------
  export EXO_PACKER_CMD=make
  export EXO_PACKER_OPT=dist

if [ "$EXO_UNAME" = "Darwin" ]; then
	export EXO_PACKER_CMD=/Applications/BitRock*/bin/Builder.app/Contents/MacOS/installbuilder.sh
	export EXO_PACKER_OPT="--project bitrock_all.xml"
fi

# -----------------------
# --- COMMAND TO PACK ---
# -----------------------
if [ "$EXO_UNAME" = "Darwin" ]; then
	export EXO_PACK_CMD=./bitrock_all_osx.sh
	export EXO_PACK_OPT=
fi
#  export EXO_PACK_CMD=make dist
#  export EXO_PACK_OPT=
if [ $EXO_OSBASE == "Debian" ]; then
	export EXO_PACK_CMD=dpkg
	export EXO_PACK_OPT="-b debian/exodusmvdb $EXO_INSTALL_FILENAME"
fi

# needs EXO_UPLOADUSER UPLOADPASS_EXO EXO_INSTALL_FILENAME
# -------------------------
# --- COMMAND TO UPLOAD ---
# -------------------------
# NOTE GOODLE CODE HAS NO OVERWRITE OPTION OR DELETE OPTION
  export EXO_UPLOAD_CMD=./googlecode_upload.py

#without pass
echo export UPLOAD_OPT_EXO=" \
 --summary=$EXO_GOOGLECODE_SUMMARY \
 --project=$EXO_GOOGLECODE_PROJECTCODE \
 --user=$EXO_UPLOADUSER \
 --password= \
 --labels=$EXO_GOOGLE_CODE_LABELS \
 $EXO_INSTALL_FILENAME"

#with pass
  export UPLOAD_OPT_EXO=" \
 --summary=$EXO_GOOGLECODE_SUMMARY \
 --project=$EXO_GOOGLECODE_PROJECTCODE \
 --user=$EXO_UPLOADUSER \
 --password=$UPLOADPASS_EXO \
 --labels=$EXO_GOOGLE_CODE_LABELS \
 $EXO_INSTALL_FILENAME"


#afteruploader:

#----------
#-- Log ---
#----------
set | grep EXO_

