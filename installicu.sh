#!/bin/bash

#------------
#--- Init ---
#------------
set -e
source config.sh
cd ~

#---------------
#--- Delete ----
#---------------
if [ "$EXO_ICU_REUSE_DOWNLOAD" != "YES" ]; then
	echo TODO uninstall all icu libs first
	test -f $EXO_ICU_FILE && rm -f $EXO_ICU_FILE
	test -d $EXO_ICU_DIR && rm -rf $EXO_ICU_DIR
fi

#------------------------
#--- Download/Extract ---
#------------------------
if [ "$EXO_ICU_DOWNLOADER" = "SUBVERSION" ]; then
	echo svn checkout/update $EXO_ICU_URL_SUBVERSION to $EXO_ICU_DIR
	test -d $EXO_ICU_DIR             && svn update $EXO_ICU_DIR
	test -f $EXO_ICU_DIR/readme.html && rm -f $EXO_ICU_DIR/readme.html
	test -d $EXO_ICU_DIR             || svn co $EXO_ICU_URL_SUBVERSION $EXO_ICU_DIR
#else
#	#echo Downloading and untarring icu (in parallel)
#	#curl -L $EXO_ICU_URL_CURL | tee $EXO_ICU_FILE | tar xz
fi


#----------
#--- cd ---
#----------
cd $EXO_ICU_DIR/source

#-------------
#--- Clean ---
#-------------
test -f Makefile && make clean && make distclean

#-----------------
#--- Configure ---
#-----------------
#-fvisibility=hidden is added in a futile attempt to avoid zillions of warnings while building icu
./configure \
 --enable-extras=no \
 --enable-extras=no \
 --enable-icuio=no \
 --enable-layout=no \
 --enable-tests=no \
 --enable-samples=no \
  --build="$EXO_BUILD" \
       CC="$EXO_CC" \
   CFLAGS="$EXO_FLAGS" \
      CXX="$EXO_CXX" \
 CXXFLAGS="$EXO_FLAGS" \
  LDFLAGS="$EXO_LDFLAGS -fvisibility=hidden" \
     LIBS="$EXO_LIBS" \
 --enable-shared=no \
 --enable-static=yes

#------------
#--- Make ---
#------------
make

#---------------
#--- Install ---
#---------------
sudo make install
