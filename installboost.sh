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
	test -f ${EXO_BOOST_FILE} && sudo rm -f ${EXO_BOOST_FILE}
	test -d ${EXO_BOOST_DIR}  && sudo rm -rf ${EXO_BOOST_DIR}
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

echo "# Compiler configuration
using $EXO_BOOST_JAM_USING
       <architecture>\"$EXO_BOOST_JAM_ARCHITECTURE\"
      <address-model>\"$EXO_BOOST_JAM_ADDRESS_MODEL\"
     <macosx-version>\"$EXO_MINVER\"
 <macosx-version-min>\"$EXO_MINVER\"
              #<root>\"/Developer\"
       <compileflags>\"\"
          <linkflags>\"$EXO_LDFLAGS $EXO_LIBS_ICU \" ;" \
>exodus-darwin-$EXO_BOOST_JAM_ARCHITECTURE-$EXO_BOOST_JAM_ADDRESS_MODEL-$EXO_MINVER.jam

#--------------------
#--- Make/Install ---
#--------------------
echo BJAM SHOULD SAY "HAS_ICU BUILDS: YES" OTHERWISE CHECK YOUR ICU INSTALLATION ABOVE
echo #################################################################################
#TODO sudo only needed for install
#./bjam
sudo ./bjam \
 --stagedir=$EXO_EPREFIX \
 --user-config=exodus-darwin-$EXO_BOOST_JAM_ARCHITECTURE-$EXO_BOOST_JAM_ADDRESS_MODEL-$EXO_MINVER.jam \
 define=U_STATIC_IMPLEMENTATION=1 \
 --with-date_time --with-filesystem --with-regex --with-system --with-thread \
 link=static \
 stage


# --prefix=$EXO_PREFIX \
# --exec-prefix=$EXO_EPREFIX \
