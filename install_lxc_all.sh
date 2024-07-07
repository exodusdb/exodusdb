#!/bin/bash
set -euxo pipefail
: $0$*
: ==========================================================================================
: Build and install exodus on multiple OS and compilers in one command  using install_lxc.sh
: ==========================================================================================
:
: Syntax:
:
: "$0 <BASE_CONTAINERS,> <STAGES> [g++,clang,g++-default,clang-default,]"
:
: Just the build stage with g++ for two OSes using latest version of g++ available
:
: $0 u2404,u2204 B g++
:
: All stages for both g++ and clang
:
: $0 u2404 A
:
: For more info see ./install_lxc.sh
:
: Config
: ------
:
	BASE_CONTAINERS=${1:?BASE_CONTAINERS is required. e.g u2404 or using commas: u2404,u2202. Must exist and will be copied.}
	STAGES=${2:?Stages is required e.g. A for all or any consecutive chars of 'bBiITW'}
	COMPILERS=${3:-g++ clang}
:
: MAIN
: ----
:
#	for OS in u2404 u2204 u2004; do
	for OS in ${BASE_CONTAINERS//,/ }; do

: Check if starting from stage 1
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
: Upgrade all in base container $OS
: -----------------------------
:
			lxc exec $OS -- bash -c "apt-get update && apt-get -y dist-upgrade || true"
:
		fi # upgrading base container

:
: For each requested compiler $COMPILERS
: ---------------------------
:
		for COMPILER in ${COMPILERS//,/ }; do
:
: Install using specific compiler $COMPILER
: -------------------------------
:
			export CXX_OPTIONS=-fdiagnostics-color=always
			LOGFILE=${OS}${COMPILER:0:1}-$STAGES.log
			./install_lxc.sh $OS ${OS} $STAGES $COMPILER |& tee $LOGFILE

		done

	done
: ====================================================================
: Finished $0 $* in $((SECONDS / 60)) mins and $((SECONDS % 60)) secs.
: ====================================================================
