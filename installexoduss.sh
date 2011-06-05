#!/bin/bash

#------------
#--- Init ---
#------------
set -e
source config.sh
cd ~

#-------------
#--- Setup ---
#-------------

export PATH=EXO_POSTGRES_BIN:$PATH

#patch in suppression of EXECINFO.H (backtrace) not available on 10.4
#now relies on HAS_BACKTRACE to include execinfo.h
#test $EXO_MINVER = 10.4 && export EXO_FLAGS="$EXO_FLAGS -DHASNT_EXECINFO"

#--------------
#--- Delete ---
#--------------
#optionally delete the EXODUS download file to force redownloading
if [ "$EXO_EXODUS_REUSE_DOWNLOAD" != "YES" ]; then
	test -f $EXO_EXODUS_FILE && sudo rm -f $EXO_EXODUS_FILE
#dont delete exodus folder MIGHT LOSE SOME DEVELOPMENT WORK!!!
#	test -d $EXO_EXODUS_DIR  && sudo rm -rf $EXO_EXODUS_DIR
fi

#------------------------
#--- Download/Extract ---
#------------------------
if [ "$EXO_EXODUS_DOWNLOADER" = "SUBVERSION" ]; then
        echo svn checkout/update $EXO_EXODUS_URL_SUBVERSION to $EXO_EXODUS_DIR
        test -d $EXO_EXODUS_DIR             && svn update $EXO_EXODUS_DIR
        test -d $EXO_EXODUS_DIR             || svn co $EXO_EXODUS_URL_SUBVERSION $EXO_EXODUS_DIR
else

#untar any existing download already if the directory doesnt exist
if [ -f $EXO_EXODUS_FILE ]; then
	test -d $EXO_EXODUS_DIR || echo untarring $EXO_EXODUS_FILE -> $EXO_EXODUS_DIR
	test -d $EXO_EXODUS_DIR || tar xfz $EXO_EXODUS_FILE
else
	#copied from icu install but not tested

	#download, save and untar (if not already present)
	echo Downloading and untarring EXODUS
	curl -L $EXO_EXODUS_URL | tee $EXO_EXODUS_FILE.part|tar xz

	#save download if downloaded completed
	# set -e in heading should prevent arrival here if download not complete
	cp -f $EXO_EXODUS_FILE.part $EXO_EXODUS_FILE
fi
fi

#---------
#-- cd ---
#---------
cd $EXO_EXODUS_DIR

#-------------
#--- Clean ---
#-------------
test -f Makefile && make clean && make distclean

#-----------------
#--- Configure ---
#-----------------
#./configure
#  --build=i386-apple-darwin8.11.0
#       CC=gcc-4.0 CXX=g++-4.0
#   CFLAGS=-arch i386 -mmacosx-version-min=10.4 -march=prescott -DHASNT_EXECINFO
# CXXFLAGS=-arch i386 -mmacosx-version-min=10.4 -march=prescott -DHASNT_EXECINFO
#  LDFLAGS=-arch i386 -mmacosx-version-min=10.4 -march=prescott -DHASNT_EXECINFO -Bstatic
#     LIBS=-licudata -licui18n -licutu -licuuc -lboost_date_time -lboost_filesystem -lboost_regex -lboost_system -lboost_thread

echo ./configure \
  --build="$EXO_BUILD" \
       CC="$EXO_CC" \
      CXX="$EXO_CXX" \
   CFLAGS="$EXO_FLAGS" \
 CXXFLAGS="$EXO_FLAGS" \
  LDFLAGS="$EXO_FLAGS $EXO_LDFLAGS" \
     LIBS="$EXO_LIBS_ICU $EXO_LIBS_BOOST"

./configure \
  --build="$EXO_BUILD" \
       CC="$EXO_CC" \
      CXX="$EXO_CXX" \
   CFLAGS="$EXO_FLAGS" \
 CXXFLAGS="$EXO_FLAGS" \
  LDFLAGS="$EXO_FLAGS $EXO_LDFLAGS" \
     LIBS="$EXO_LIBS_ICU $EXO_LIBS_BOOST"

#------------
#--- Make ---
#------------
make

#---------------
#--- Install ---
#---------------
sudo make install

#-------------------
#--- Postinstall ---
#-------------------
echo Configure Exodus for Postgres and vice versa
echo After exodus is installed, to setup postgres do the following.
echo
echo   configexodus
echo
echo Test Exodus
echo To get an exodus console - sets some environment variables
echo
echo  exodus
echo  testsort

