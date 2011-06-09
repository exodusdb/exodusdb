#!/bin/bash

#------------
#--- Init ---
#------------
set -e
source config.sh
cd ~

#--------------
#--- Delete ---
#--------------
#optionally delete the boost download file to force redownloading
if [ "$EXO_BOOST_REUSE_DOWNLOAD" != "YES" ]; then
	test -f ${EXO_BOOST_FILE} && rm -f ${EXO_BOOST_FILE}
	test -d ${EXO_BOOST_DIR}  && rm -rf ${EXO_BOOST_DIR}
fi

#replaced by curl - see below
#wget ${EXO_BOOST_URL}


#------------------------
#--- Download/Extract ---
#------------------------
#untar any existing download already if the directory doesnt exist
if [ -f ${EXO_BOOST_FILE} ]; then
	test -d ${EXO_BOOST_DIR} || echo untarring boost ${EXO_BOOST_FILE} to ${EXO_BOOST_DIR}
	test -d ${EXO_BOOST_DIR} || tar xfz ${EXO_BOOST_FILE}
else
	#download, save and untar (if not already present)
	echo Downloading and untarring boost
	curl -L ${EXO_BOOST_URL}|tee ${EXO_BOOST_FILE}.part|tar xz
	#save download if downloaded completed
	# set -e in heading should prevent arrival here if download not complete
	cp -f ${EXO_BOOST_FILE}.part ${EXO_BOOST_FILE}
fi

#----------
#--- cd ---
#----------
cd ${EXO_BOOST_DIR}

#-----------------
#--- Configure ---
#-----------------
test -f bjam || ./bootstrap.sh

# eg   using gcc : 3.4 : : <compileflags>-m64 <linkflags>-m64 ;

export EXO_BOOST_JAMFILE=exodus-${EXO_UNAME}-${EXO_BOOST_JAM_ARCHITECTURE}-${EXO_BOOST_JAM_ADDRESS_MODEL}-${EXO_MINVER}.jam

if [ "$EXO_UNAME" == "Darwin" ]; then
cat > $EXO_BOOST_JAMFILE << EOF
# Compiler configuration
using $EXO_BOOST_JAM_USING
       <architecture>"$EXO_BOOST_JAM_ARCHITECTURE"
      <address-model>"$EXO_BOOST_JAM_ADDRESS_MODEL"
     <macosx-version>"$EXO_MINVER"
 <macosx-version-min>"$EXO_MINVER"
              #<root>"/Developer"
       <compileflags>"$EXO_FLAGS"
          <linkflags>"$EXO_LDFLAGS $EXO_LIBS_ICU" ;
EOF

else

cat > $EXO_BOOST_JAMFILE << EOF
# Compiler configuration
using $EXO_BOOST_JAM_USING
       <compileflags>"$EXO_FLAGS"
          <linkflags>"$EXO_LDFLAGS $EXO_LIBS_ICU" 
          <linkflags>"$EXO_LDFLAGS $EXO_LIBS_ICU $EXO_FLAGS" ;
EOF
fi

echo -----------------------------------------------------------------
cat $EXO_BOOST_JAMFILE

#--------------------
#--- Make/Install ---
#--------------------
#staging so only copied libs and no copying of zillons include files
echo -----------------------------------------------------------------
echo ./bjam \
 --stagedir=$EXO_EPREFIX \
 --user-config=$EXO_BOOST_JAMFILE \
 define=U_STATIC_IMPLEMENTATION=1 \
 --with-date_time --with-filesystem --with-regex --with-system --with-thread \
 link=static \
 variant=release \
 -a -j2 \
 stage
echo -----------------------------------------------------------------
sleep 1

echo BJAM SHOULD SAY "HAS_ICU BUILDS: YES" OTHERWISE CHECK YOUR ICU INSTALLATION ABOVE
echo  and  nano ~/boost_1_46_1/bin.v2/config.log for the compile options on test has_icu.cpp
echo #######################################################################################
./bjam \
 --stagedir=$EXO_EPREFIX \
 --user-config=$EXO_BOOST_JAMFILE\
 define=U_STATIC_IMPLEMENTATION=1 \
 --with-date_time --with-filesystem --with-regex --with-system --with-thread \
 link=static \
 variant=release \
 -a -j2 \
 stage

# --prefix=$EXO_PREFIX \
# --exec-prefix=$EXO_EPREFIX \
