#!/bin/bash
set -euxo pipefail
PS4='+ [${SECONDS}s] '
: $0$*
: ==========================================================================================
: Build and install exodus on multiple OS and compilers in one command  using install_lxc.sh
: ==========================================================================================
:
: 2026-01-09 Success build on 3 OS x 2 Compilers
: ./install_lxc_all.sh u2204,u2404,u2510 bBdDTW clang,g++
:
: Syntax:
:
: "$0 <BASE_CONTAINERS,> <STAGES> [clang,clang-default,clang-18,clang-latest,g++,g++-default,g++-14,g++-latest, ]"
:
: 'BASE CONTAINERS - e.g. u2404,u2204,u2004'
:
: 'COMPILERS - default is clang'
:
: 'clang or g++ for latest version available. Optionally followed by a version e.g.'
: 'clang-18, clang-min, clang-latest, clang-default, g++-14, g++-min, g++-latest, g++-default'
:
: Example:
:
: Just the build stage with g++ for two OSes
: using whatever version of g++ was installed in stage b
:
: $0 u2404,u2204 B g++
:
: All stages except W for clang
:
: $0 u2404 A
:
: For more info see ./install_lxc.sh
:
: Config
: ------
:
	BASE_CONTAINERS=${1:?BASE_CONTAINERS is required. e.g u2404 or using commas: u2404,u2202. Must exist and will be copied.}
	STAGES=${2:?Stages is required e.g. A for all or any consecutive chars of 'bBdDTW'}
	COMPILERS=${3:-clang}

:
: Main
: ----
:
: Loop through requested base containers
: --------------------------------------
:
#	for OS in u2404 u2204 u2004; do
	for OS in ${BASE_CONTAINERS//,/ }; do

: Check if starting from stage 1 - $OS
: ------------------------------
:
		if [[ "bA" =~ ${STAGES} ]]; then

:
: Upgrade base container
: ----------------------
:
: Check base container exists $OS
: ---------------------------
:
			lxc info $OS >/dev/null
:
: Start base container $OS
: --------------------
:
			lxc start $OS || true
:
: Upgrade all in base container - $OS
: -----------------------------
:
			lxc exec $OS -- bash -c "apt-get update && apt-get -y dist-upgrade || true"
:
		fi # upgrading base container

:
: For each requested compiler - $COMPILERS
: ---------------------------
:
		for COMPILER in ${COMPILERS//,/ }; do
:
: Install using specific compiler - $COMPILER
: -------------------------------
:
			export CXX_OPTIONS=-fdiagnostics-color=always
			LOGFILE=${OS}${COMPILER:0:1}-$STAGES.log
			./install_lxc.sh $OS ${OS} $STAGES $COMPILER |& tee $LOGFILE

		done # next compiler
	done # next OS
:
: ====================================================================
: Finished $0 $* in $((SECONDS / 60)) mins and $((SECONDS % 60)) secs.
: ====================================================================
