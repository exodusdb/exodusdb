#!/bin/bash

EXODUS_VERSION=$1
echo EXODUS_VERSION=$EXODUS_VERSION

#./googlecode_upload.py --help
#Usage: googlecode-upload.py -s SUMMARY -p PROJECT [options] FILE
#
#Options:
#  -h, --help            show this help message and exit
#  -s SUMMARY, --summary=SUMMARY
#                        Short description of the file
#  -p PROJECT, --project=PROJECT
#                        Google Code project name
#  -u USER, --user=USER  Your Google Code username
#  -w PASSWORD, --password=PASSWORD
#                        Your Google Code password
#  -l LABELS, --labels=LABELS
#                        An optional list of comma-separated labels to attach
#                        to the file

#save current directory (to find googlecode_upload.py later)
export ORIG_DIRXYZ=`pwd`

#get into BitRock's .app output folder
pushd /Applications/BitRock*/output

#delete any old zip
if [ -f "exodus-${EXODUS_VERSION}-osx-installer.app.zip" ]
then
rm exodus-${EXODUS_VERSION}-osx-installer.app.zip
fi

#make a new zip
zip -r \
 exodus-${EXODUS_VERSION}-osx-installer.app.zip \
 exodus-${EXODUS_VERSION}-osx-installer.app/*


#view new zip size
ls -l exodus-${EXODUS_VERSION}-osx-installer.app.zip

#upload the new zip to google code
${ORIG_DIRXYZ}/googlecode_upload.py \
 -s "Mac OSX 64bit Snow Leopard" \
 -p "exodusdb" \
 -u "neosys.com@gmail.com" \
 -w "${GOOGLECODE_PASS}" \
 -l "Featured,Type-Package,OpSys-OSX" \
 exodus-${EXODUS_VERSION}-osx-installer.app.zip

#get back to whereever we started
popd
