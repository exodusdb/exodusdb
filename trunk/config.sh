#!/bin/bash

export EXO_CONFIGMODE=$1

export EXO_CODENAME=exodus

export PATH=/Library/PostgreSQL/9.0/bin:$PATH

test -f ./version.sh     && source ./version.sh
test -f ./configlocal.sh && source ./configlocal.sh

test "$EXO_BATCH_MAJOR_VER" = "" || export EXO_MAJOR_VER=$EXO_BATCH_MAJOR_VER
test "$EXO_BATCH_MINOR_VER" = "" || export EXO_MINOR_VER=$EXO_BATCH_MINOR_VER
test "$EXO_BATCH_MICRO_VER" = "" || export EXO_MICRO_VER=$EXO_BATCH_MICRO_VER
test "$EXO_BATCH_BUILD_VER" = "" || export EXO_BUILD_VER=$EXO_BATCH_BUILD_VER

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
  export EXO_CONFIGURE_CMD=./configure
  export EXO_CONFIGURE_OPT=
  export EXO_MAKE_CMD=make
  export EXO_MAKE_OPT=

# ------------------------
# --- INSTALLFILE NAME ---
# ------------------------
  export EXO_INSTALLFILENAME="$EXO_CODENAME-$EXO_MAJOR_VER.$EXO_MINOR_VER.$EXO_MICRO_VER-osx-installer.app.zip"


#if [ "$EXO_CONFIGMODE" -eq "CLEAN" ] goto afteruploader
#if [ "$EXO_CONFIGMODE" -eq "MAKE"  ] goto afteruploader
#if [ "$EXO_CONFIGMODE" -eq "DEV"   ] goto afteruploader

# -------------------------
# --- COMMAND PACKER UI ---
# -------------------------
  export EXO_PACKER_CMD=/Applications/BitRock*/bin/Builder.app/Contents/MacOS/installbuilder.sh
  export EXO_PACKER_OPT="--project bitrock_all.xml"

# -----------------------
# --- COMMAND TO PACK ---
# -----------------------
  export EXO_PACK_CMD=./bitrock_all_osx.sh
  export EXO_PACK_OPT=
# needs EXO_UPLOADUSER UPLOADPASS_EXO EXO_INSTALLFILENAME

# -------------------------
# --- COMMAND TO UPLOAD ---
# -------------------------
# NOTE GOODLE CODE HAS NO OVERWRITE OPTION OR DELETE OPTION
  export EXO_UPLOAD_CMD=./googlecode_upload.py
  export EXO_UPLOAD_OPT=" \
 --summary=Mac_OSX_${TARGET_CPU}_Installer \
 --project=exodusdb \
 --user=$EXO_UPLOADUSER \
 --password=$UPLOADPASS_EXO \
 --labels=Type-Installer,OpSys-OSX,Featured \
 $EXO_INSTALLFILENAME"


#afteruploader:

set|grep EXO_
