#!/bin/bash
set -euxo pipefail
:
: -----------------------------
: Builds exodus and installs it
: -----------------------------
:
:	Ubuntu	Postgres
:	23.04	15 OK
:	22.04	14 OK - default
:	20.04	12 OK
:	18.04	10 KO Exodus requires c++20 so will not build on 18.04
:
: -------
: Syntax:
: -------
:
: $0 UBUNTU_VER PG_VER
:
	UBUNTU_VER=${1}
	PG_VER=${2}
:
: ------
: Config
: ------
:
	export DEBIAN_FRONTEND=noninteractive
	#EXODUS=${GITHUB_WORKSPACE:-~/exodus}
	EXODUS=$(pwd)
:
: ------------------
: Prepare the system
: ------------------
:
	if ! test -f upgraded.txt; then
		apt update && DEBIAN_FRONTEND=noninteractive apt -y upgrade
	fi
:
: -----------------------------
: Acquire dependencies to build
: -----------------------------
:
	apt install -y cmake # for exodus and pgexodus building
	apt install -y g++ libboost-regex-dev libboost-locale-dev libpq-dev # for exodus build >= 20.04
	#apt install -y g++ libboost-date-time-dev libboost-system-dev libboost-thread-dev # for exodus build <= 18.04
	apt install -y postgresql-server-dev-$PG_VER #for pgexodus build
:
: ----------------------------------------
: Build libexodus, pgexodus, cli and tests
: ----------------------------------------
:
	cmake -S $EXODUS -B $EXODUS/build
	cmake --build $EXODUS/build -j`nproc`
:
: -------------------------
: Install postgresql server
: -------------------------
	#postgresql-client-$PG_VER already installed by ?
	apt install -y postgresql-$PG_VER #for pgexodus install
:
: --------------
: Install exodus
: --------------
: 1. libexodus
: 2. exodus cli
: 3. pgexodus extension
:
	cmake --install $EXODUS/build
:
#:
#: -------------------------------
#: Test exodus except db functions
#: -------------------------------
#:
#	cd $EXODUS/build
#	CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=`nproc` ctest
:
: -------------------------------
: Configure postgresql for exodus
: -------------------------------

	# Waiting for postgresql to start?
	#(while ! nc -z -v -w1 localhost 5432 2>/dev/null; do echo "Waiting for port 5432 to open..."; sleep 2; done)

	# Optional. Remove warning by enable others/postgres to cd into build dir
	# 'could not change directory to "/root": Permission denied'
	#chmod o+x \$HOME

	# Install into template1
	sudo -u postgres psql < $EXODUS/install_template1.sql

	# Create
	# 1. exodus user login/password
	# 2. exodus database
	sudo -u postgres psql < $EXODUS/install_exodus.sql

	# Lots of pgsql utility functions
	# Necessary to test many exodus pgsql function e.g. exodus_date()
	dict2sql
:
: -------------------------
: Test exodus all functions
: -------------------------
:
	#testsort
	cd $EXODUS/build && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=`nproc` ctest
	testsort
#:
#: -------------------
#: Install www service
#: -------------------
#
#	cd $EXODUS/service
#	./install_all.sh'
:
: -------------------------------------------------------------
: Finished $0 in $((SECONDS/60)) mins and $((SECONDS%60)) secs.
: -------------------------------------------------------------
