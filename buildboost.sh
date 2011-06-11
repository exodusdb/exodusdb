#!/bin/bash

#------------
#--- Init ---
#------------
set -e
source config.sh
cd ~

#----------------
#--- "Delete" ---
#----------------
#optionally delete the boost download file to force redownloading
if [ "$EXO_BOOST_REUSE_DOWNLOAD" != "YES" ]; then
	test -f ${EXO_BOOST_FILENAME} && rm -f ${EXO_BOOST_FILENAME}
	test -d ${EXO_BOOST_DIR}  && rm -rf ${EXO_BOOST_DIR}
fi

#--------------------------
#--- "Download/Extract" ---
#--------------------------
#untar any existing download already if the directory doesnt exist
if [ -f $EXO_BOOST_FILENAME ]; then
	test -d ${EXO_BOOST_DIR} || echo untarring boost ${EXO_BOOST_FILENAME} to ${EXO_BOOST_DIR}
	test -d ${EXO_BOOST_DIR} || tar xfz ${EXO_BOOST_FILENAME} || rm $EXO_BOOST_FILENAME

#otherwise download and untar
else
	#detect curl or wget
	export EXO_CURL_WGET="curl -L"
	which curl 2>&1 > /dev/null || export EXO_CURL_WGET="wget -O-"

	#download, save and untar (if not already present)
	echo Downloading and untarring boost
	$EXO_CURL_WGET ${EXO_BOOST_URL}|tee ${EXO_BOOST_FILENAME}.part|tar xz
	#save download if downloaded completed
	# set -e in heading should prevent arrival here if download not complete
	cp -f ${EXO_BOOST_FILENAME}.part ${EXO_BOOST_FILENAME}

fi

#-----------------
#--- "Configure" ---
#-----------------
cd ${EXO_BOOST_DIR}

test -f bjam || ./bootstrap.sh

export EXO_BOOST_JAMFILE=exodus-${EXO_UNAME}-${EXO_BOOST_JAM_ARCHITECTURE}-${EXO_BOOST_JAM_ADDRESS_MODEL}-${EXO_MINVER}.jam

# eg   using gcc : 3.4 : : <compileflags>-m64 <linkflags>-m64 ;

if [ "$EXO_UNAME" == "Darwin" ]; then
cat > $EXO_BOOST_JAMFILE << EOF
# Compiler configuration
using $EXO_BOOST_JAM_USING
       <architecture>"$EXO_BOOST_JAM_ARCHITECTURE"
      <address-model>"$EXO_BOOST_JAM_ADDRESS_MODEL"
     <macosx-version>"$EXO_MINVER"
 <macosx-version-min>"$EXO_MINVER"
              #<root>"/Developer"
       <compileflags>"$EXO_BOOST_FLAGS"
          <linkflags>"$EXO_BOOST_LDFLAGS $EXO_BOOST_LIBS" ;
EOF

else

cat > $EXO_BOOST_JAMFILE << EOF
# Compiler configuration
using $EXO_BOOST_JAM_USING
       <compileflags>"$EXO_BOOST_FLAGS"
          <linkflags>"$EXO_BOOST_LDFLAGS $EXO_BOOST_LIBS" ;
EOF

fi

#export EXO_BOOST_LINKAGE="define=U_STATIC_IMPLEMENTATION=1 link=static" #done in EXO_FLAGS now?
export EXO_BOOST_LINKAGE="link=static"

echo -----------------------------------------------------------------
echo cat $EXO_BOOST_JAMFILE
cat $EXO_BOOST_JAMFILE

#------------------------
#--- "Make & Install" ---
#------------------------
#staging so only copied libs and no copying of zillons include files
echo -----------------------------------------------------------------
echo ./bjam \
 --stagedir=$EXO_BOOST_EPREFIX \
 --user-config=$EXO_BOOST_JAMFILE \
 --with-date_time --with-filesystem --with-regex --with-system --with-thread \
 $EXO_BOOST_LINKAGE \
 variant=release \
 -a -j2 \
 stage
echo -----------------------------------------------------------------
sleep 1

echo BJAM SHOULD SAY "HAS_ICU BUILDS: YES" OTHERWISE CHECK YOUR ICU INSTALLATION ABOVE
echo !!! CHECK nano ~/boost_1_46_1/bin.v2/config.log for why has_icu.cpp wont compile!!!
echo ###################################################################################
./bjam \
 --stagedir=$EXO_BOOST_EPREFIX \
 --user-config=$EXO_BOOST_JAMFILE\
 --with-date_time --with-filesystem --with-regex --with-system --with-thread \
 $EXO_BOOST_LINKAGE \
 variant=release \
 -a -j2 \
 stage

# --prefix=$EXO_PREFIX \
# --exec-prefix=$EXO_BOOST_EPREFIX \
