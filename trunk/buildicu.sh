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
	test -f $EXO_ICU_FILENAME && rm -f $EXO_ICU_FILENAME
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
#
#        #detect curl or wget
#        export EXO_CURL_WGET="curl -L"
#        which curl 2>&1 > /dev/null || export EXO_CURL_WGET="wget -O-"
#
#	 $EXO_CURL_WGET $EXO_ICU_URL_CURL | tee $EXO_ICU_FILENAME | tar xz
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
echo ---------------------------------------------
echo ./configure \
 --prefix=$EXO_ICU_PREFIX \
 --exec-prefix=$EXO_ICU_EPREFIX \
 --enable-extras=no \
 --enable-extras=no \
 --enable-icuio=no \
 --enable-layout=no \
 --enable-tests=no \
 --enable-samples=no \
  --build="$EXO_BUILD" \
       CC="$EXO_CC" \
      CXX="$EXO_CXX" \
   CFLAGS="$EXO_ICU_FLAGS" \
 CXXFLAGS="$EXO_ICU_FLAGS" \
  LDFLAGS="$EXO_ICU_LDFLAGS" \
     LIBS="$EXO_ICU_LIBS" \
 --enable-shared=no \
 --enable-static=yes
echo ---------------------------------------------
sleep 1
./configure \
 --prefix=$EXO_ICU_PREFIX \
 --exec-prefix=$EXO_ICU_EPREFIX \
 --enable-extras=no \
 --enable-extras=no \
 --enable-icuio=no \
 --enable-layout=no \
 --enable-tests=no \
 --enable-samples=no \
  --build="$EXO_BUILD" \
       CC="$EXO_CC" \
      CXX="$EXO_CXX" \
   CFLAGS="$EXO_ICU_FLAGS" \
 CXXFLAGS="$EXO_ICU_FLAGS" \
  LDFLAGS="$EXO_ICU_LDFLAGS" \
     LIBS="$EXO_ICU_LIBS" \
 --enable-shared=no \
 --enable-static=yes

#------------
#--- Make ---
#------------
make

#---------------
#--- Install ---
#---------------
make install
