#!/bin/bash

#------------
#--- "Init" ---
#------------
set -e
source config.sh
cd ~

#-------------
#--- "Setup" ---
#-------------

export PATH=EXO_POSTGRES_BIN:$PATH

#--------------
#--- "Delete" ---
#--------------
#optionally delete the EXODUS download file to force redownloading
if [ "$EXO_EXODUS_REUSE_DOWNLOAD" != "YES" ]; then
	test -f $EXO_EXODUS_FILENAME && sudo rm -f $EXO_EXODUS_FILENAME
#dont delete exodus folder MIGHT LOSE SOME DEVELOPMENT WORK!!!
#	test -d $EXO_EXODUS_DIR  && sudo rm -rf $EXO_EXODUS_DIR
fi

#------------------------
#--- "Download/Extract" ---
#------------------------
if [ "$EXO_EXODUS_DOWNLOADER" = "SUBVERSION" ]; then
        echo svn checkout/update $EXO_EXODUS_URL_SUBVERSION to $EXO_EXODUS_DIR
        test -d $EXO_EXODUS_DIR             && svn update $EXO_EXODUS_DIR
        test -d $EXO_EXODUS_DIR             || svn co $EXO_EXODUS_URL_SUBVERSION $EXO_EXODUS_DIR
else

#untar any existing download already if the directory doesnt exist
if [ -f $EXO_EXODUS_FILENAME ]; then
	test -d $EXO_EXODUS_DIR || echo untarring $EXO_EXODUS_FILENAME -> $EXO_EXODUS_DIR
	test -d $EXO_EXODUS_DIR || tar xfz $EXO_EXODUS_FILENAME
else
	#copied from icu install but not tested

        #detect curl or wget
        export EXO_CURL_WGET="curl -L"
        which curl 2>&1 > /dev/null || export EXO_CURL_WGET="wget -O-"

	#download, save and untar (if not already present)
	echo Downloading and untarring EXODUS
	$EXO_CURL_WGET $EXO_EXODUS_URL_FILE | tee $EXO_EXODUS_FILENAME.part|tar xz

	#save download if downloaded completed
	# set -e in heading should prevent arrival here if download not complete
	cp -f $EXO_EXODUS_FILENAME.part $EXO_EXODUS_FILENAME
fi
fi

#------------
#--- "cd" ---
#------------
cd $EXO_EXODUS_DIR

#---------------
#--- "Clean" ---
#---------------
test -f Makefile && make clean && make distclean

#-------------------
#--- "Configure" ---
#-------------------
export EXO_EXODUS_CONFIG_SCRIPTFILE=configexodus-${EXO_UNAME}-${EXO_ARCH}-${EXO_MINVER}.sh

echo ./configure \
 --prefix=\"$EXO_EXODUS_PREFIX\" \
 -exec-prefix=\"$EXO_EXODUS_EPREFIX\" \
  --build=\"$EXO_BUILD\" \
       CC=\"$EXO_CC\" \
      CXX=\"$EXO_CXX\" \
   CFLAGS=\"$EXO_EXODUS_FLAGS\" \
 CXXFLAGS=\"$EXO_EXODUS_FLAGS\" \
  LDFLAGS=\"$EXO_EXODUS_LDFLAGS\" \
     LIBS=\"$EXO_EXODUS_LIBS\" \
 --with-boost-libdir=$EXO_BOOST_EPREFIX/lib >$EXO_EXODUS_CONFIG_SCRIPTFILE
# --enable-shared=yes
# --enable-shared=no \
# --enable-static=yes 
echo ------------------------------------------------------------
echo $EXO_EXODUS_CONFIG_SCRIPTFILE
cat $EXO_EXODUS_CONFIG_SCRIPTFILE
echo ------------------------------------------------------------
sleep 1

. $EXO_EXODUS_CONFIG_SCRIPTFILE

#--------------
#--- "Make" ---
#--------------
make

#-----------------
#--- "Install" ---
#-----------------
echo ----------------
echo To Install
echo " sudo make install"
echo
echo After installtion, get an Exodus console
echo " exodus"
echo
echo Then setup postgres do the following.
echo " configexodus"
echo
echo After configuration, test
echo " testsort"
