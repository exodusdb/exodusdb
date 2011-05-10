#!/bin/bash

EXODUS_MAJOR_VERSION=10
EXODUS_MINOR_VERSION=10.10
EXODUS_MICRO_VERSION=10.10.9
EXODUS_BUILD_VERSION=10.10.9.0

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

#TODO let these be set by command line or environment variables

/Applications/BitRock*/bin/Builder.app/Contents/MacOS/installbuilder.sh \
 build \
 bitrock_all.xml \
 --setvars \
 installdir=/Applications/Exodus/${EXODUS_MINOR_VERSION}/ \
 project.version="${EXODUS_MICRO_VERSION}" \
 project.fullName="Exodus Multivalue Database ${EXODUS_MICRO_VERSION}"
