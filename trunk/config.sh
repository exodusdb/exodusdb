#!/bin/bash

export EXO_CONFIGMODE=$1

export EXO_CODENAME=exodus

test -f ./version.sh     && source ./version.sh
test -f ./configlocal.sh && source ./configlocal.sh

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
 --summary=Mac OSX $TARGET_CPU Installer \
 --project=exodusdb \
 --user=$EXO_UPLOADUSER \
 --password=$UPLOADPASS_EXO \
 --labels=Type-Installer,OpSys-OSX,Featured \
 $EXO_INSTALLFILENAME"


#afteruploader:

set|grep EXO_
