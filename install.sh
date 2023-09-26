#!/bin/bash
set -euxo pipefail
:
: ------------------------------
: Build, install and test exodus
: ------------------------------
:
:	Ubuntu	Postgres
:
:	23.04	15 OK
:	22.04	14 OK - current LTS
:	20.04	12 OK
:	18.04	10 KO Exodus requires c++20 so will not build on 18.04
:
: ------
: Syntax
: ------
:
: $0 ' [<PG_VER>] [<STAGES>]'
:
: PG_VER e.g. 14 The default depends on apt and the Ubuntu version
:
: STAGES is one or more letters. Default is "'bBiIT'"
:
: b = Get dependencies for build
: B = Build
:
: i = Get dependencies for install
: I = Install
:
: T = Test
:
	PG_VER=${1:-}
	STAGES=${2:-bBiIT}
:
: ------
: CONFIG
: ------
:
	export DEBIAN_FRONTEND=noninteractive
	#EXODUS_DIR=${GITHUB_WORKSPACE:-~/exodus}
	EXODUS_DIR=$(pwd)
:
: ----------
: Update apt
: ----------
:
	if ! ls /var/cache/apt/*.bin &>/dev/null; then
		apt update
	fi
:
: ------------------------
: Work out postgres suffix
: ------------------------
:
: Need to know version number for postgresql-server-dev-NN e.g. -14
:
	if [[ -n $PG_VER ]]; then
		PG_VER_SUFFIX=-$PG_VER
		SERVER_PG_VER=$PG_VER
	else
		PG_VER_SUFFIX=
:
: 1. From latest version of installed postgresql
:    Same method as used by pg_config
:
		SERVER_PG_VER=$(ls -v /usr/lib/postgresql/|head -1||true)
:
: 2. From latest version available in apt
:
		if [[ -z $SERVER_PG_VER ]]; then
			SERVER_PG_VER=$(apt list postgresql-server-* 2> /dev/null|grep -o -P 'dev-[0-9]{2}\b'|sort|head -n1|cut -d- -f2||true)
		fi

		if [[ -z $SERVER_PG_VER ]]; then
			echo "Cannot work out postgres server version"
			exit 1
		fi
	fi
:

function get_dependencies_for_build {
: --------------------------
: GET DEPENDENCIES FOR BUILD
: --------------------------
:
	apt install -y postgresql-common

:
: Get the full postgres debian repos IF we require a specific version
: -------------------------------------------------------------------
	if [[ -n $PG_VER ]]; then
		yes | /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh || true
	fi
:
: List installed postgresql
:
	apt list postgresql* --installed
:
: List available postgresql
:
	apt list postgresql*
:
: exodus and pgexodus
: -------------------
:
	apt install -y cmake
:
: exodus
: ------
:
	apt install -y g++ libpq-dev libboost-regex-dev libboost-locale-dev
	#apt install -y g++ libboost-date-time-dev libboost-system-dev libboost-thread-dev
:
	pg_conftool show all || true
:
	ls -l /usr/lib/postgresql || true
:
: pgexodus
: --------
:
	apt install -y postgresql-server-dev-$SERVER_PG_VER
	apt install -y postgresql-common
:
:
	pg_config
:
	ls -l /usr/lib/postgresql || true
}

function build_all {
: -----
: BUILD
: -----
:
: 1. libexodus
: 2. exodus cli
: 3. pgexodus extension
:
	cmake -S $EXODUS_DIR -B $EXODUS_DIR/build
	cmake --build $EXODUS_DIR/build -j$((`nproc`+1))
}

function get_dependencies_for_install {
: ----------------------------
: GET DEPENDENCIES FOR INSTALL
: ----------------------------
:
: exodus
: ------
:
	#postgresql-client-$PG_VER already installed by ?
:
: pgexodus
: --------
:
	apt install -y postgresql$PG_VER_SUFFIX #for pgexodus install
}

function install_all {
: -------
: INSTALL
: -------
:
	cmake --install $EXODUS_DIR/build
:
#:
#: -------------------------------
#: Test exodus except db functions
#: -------------------------------
#:
#	cd $EXODUS_DIR/build
#	CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
:
: ----------------------------------------
: Check that postgresql is running locally
: ----------------------------------------
	psql --version || true
	#pg_ctl start || true
	systemctl start postgresql || true # no systemctl on docker
	systemctl status postgresql || true
	pgrep postgres -a || true
	ls /var/run/postgresql || true
:
: -------------------------------
: Configure postgresql for exodus
: -------------------------------

	# Waiting for postgresql to start?
	#(while ! nc -z -v -w1 localhost 5432 2>/dev/null; do echo "Waiting for port 5432 to open..."; sleep 2; done)

	# Optional. Remove warning by enable others/postgres to cd into build dir
	# 'could not change directory to "/root": Permission denied'
	chmod o+x $HOME

	apt install -y sudo # for docker

	# Install into template1
	sudo -u postgres psql < $EXODUS_DIR/install_template1.sql

	# Create
	# 1. exodus user login/password
	# 2. exodus database
	sudo -u postgres psql < $EXODUS_DIR/install_exodus.sql

	# Lots of pgsql utility functions
	# Necessary to test many exodus pgsql function e.g. exodus_date()
	dict2sql
}

function test_all {
: ----
: TEST
: ----
:
	systemctl start postgresql

	#testsort
	cd $EXODUS_DIR/build && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
:
	testsort
}
#:
#: -------------------
#: Install www service
#: -------------------
#
#	cd $EXODUS_DIR/service
#	./install_all.sh'
:
: ----
: MAIN
: ----
:
	[[ $STAGES =~ b ]] && get_dependencies_for_build
	[[ $STAGES =~ B ]] && build_all

	[[ $STAGES =~ i ]] && get_dependencies_for_install
	[[ $STAGES =~ I ]] && install_all

	[[ $STAGES =~ T ]] && test_all
:
: -------------------------------------------------------------
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs.
: -------------------------------------------------------------
