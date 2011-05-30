#!/bin/bash

#needs at least the following environment variables set first - see pack.sh and config.sh etc
#EXO_MAJOR_VER
#EXO_MINOR_VER
#EXO_MICRO_VER

export EXO_DOTTED_MINOR_VER=${EXO_MAJOR_VER}.${EXO_MINOR_VER}
export EXO_DOTTED_MICRO_VER=${EXO_DOTTED_MINOR_VER}.${EXO_MICRO_VER}

#/Applications/BitRock*/bin/Builder.app/Contents/MacOS/installbuilder.sh --help
#
#Usage:
# builder [command project.xml [platform]] [options]
# project.xml                    XML project to build
# platform                       Target platform. If it is not provided, the current will be used.
#
#Commands:
# build                          Perform a full build of the installer
# quickbuild                     Just pack new files and project files reusing an existing installer
#
#Options:
# --help                         Display the list of valid options
# --version                      Display product information
# --license <license>            Provide an alternative license to the builder
# --debugtrace <debugtrace>      Debug filename
# --setvars <setvars>            Modify project variables before the build process: --setvars installdir=/tmp project.version=1.5
# --onlyprojectfiles             On quickbuild mode, just update project files without considering new packed files
# --project <project>            Open specified project for editing

#--------------------------------------------------
# 1. Use Bitrock to make an app installer directory
#--------------------------------------------------

/Applications/BitRock*/bin/Builder.app/Contents/MacOS/installbuilder.sh \
 build \
 bitrock_all.xml \
 --setvars \
 installdir=/Applications/Exodus/${EXO_DOTTED_MINOR_VER}/ \
 project.version="${EXO_DOTTED_MICRO_VER}" \
 project.fullName="Exodus Multivalue Database ${EXO_DOTTED_MICRO_VER}"

#-------------------------------------------------------------------
# 2. Zip up the directory into a single uploadable/downloadable file
#-------------------------------------------------------------------

#save current directory
export SAVED_DIR_34635=`pwd`

#get into BitRock's .app output folder
pushd /Applications/BitRock*/output

#delete any old zip
if [ -f "exodus-${EXO_DOTTED_MICRO_VER}-osx-installer.app.zip" ]
then
rm exodus-${EXO_DOTTED_MICRO_VER}-osx-installer.app.zip
fi

#make a new zip
zip -r \
 exodus-${EXO_DOTTED_MICRO_VER}-osx-installer.app.zip \
 exodus-${EXO_DOTTED_MICRO_VER}-osx-installer.app/*

#move to original directory
mv exodus-${EXO_DOTTED_MICRO_VER}-osx-installer.app.zip ${SAVED_DIR_34635}/
popd

#view new zip size
ls -l exodus-${EXO_DOTTED_MICRO_VER}-osx-installer.app.zip
