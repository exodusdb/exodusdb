#!/bin/bash
set -euxo pipefail
:
: ------------------------------
: Build, install and test exodus
: ------------------------------
:
: '====== ===========   ========  ====  ====='
: 'Status Ubuntu        Postgres  g++   clang'
: '====== ===========   ========  ====  ====='
: 'OK     23.10            15.4   13.2     16'
: 'OK     23.04            15.4   12.3       '
: 'OK     22.04.3 LTS      14.9   11.4     14'
: 'OK     20.04.6 LTS      12.16   9.4     10'
: '====== ===========   ========  ====  ====='
: 'KO     18.04   LTS      10                '
: '====== ===========   ========  ====  ====='
: 'Exodus now requires c++20 so will no longer build on 18.04'
:
: ------
: Syntax
: ------
:
: $0 ' [<STAGES>] [gcc|clang] [<PG_VER>]'
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
: W = Install web service
:
: PG_VER e.g. 14 or default depends on apt and the Ubuntu version
:
: Parse command line
: ------------------
:
	STAGES=${1:-bBiIT}
	COMPILER=${2:-gcc}
	PG_VER=${3:-}
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
		sudo apt -y update
	fi
:
: ------------------------
: Work out postgres suffix
: ------------------------
:
: If PG_VER not specified, use the latest version
: of postgres installed as per pg_config
:
	PGBINROOT="/usr/lib/postgresql"
	if [[ -z $PG_VER ]]; then
		#redhat# PGBINROOT="/usr/pgsql-"
		PG_VER=`ls -v $PGBINROOT* 2>/dev/null|tail -n1 || true`
	fi
:
: Need to know version number for postgresql-server-dev-NN e.g. -14
:
	if [[ -n $PG_VER ]]; then
		PG_VER_SUFFIX=-$PG_VER
		SERVER_PG_VER=$PG_VER
		export PGPATH=/usr/lib/postgresql/$PG_VER
	else
		PG_VER_SUFFIX=
	fi
:
: 1. From latest version of installed postgresql
:    Same method as used by pg_config
:
#	SERVER_PG_VER=$(ls -v /usr/lib/postgresql/|tail -n 1||true)
#	PG_VER=`ls -v $PGBINROOT* 2>/dev/null|tail -n1 || true`
	SERVER_PG_VER=$PG_VER
:
: 2. From latest version available in apt
:
	if [[ -z $SERVER_PG_VER ]]; then
		SERVER_PG_VER=$(apt list postgresql-server-* 2> /dev/null|grep -o -P 'dev-[0-9]{2}\b'|sort|tail -n1|cut -d- -f2||true)
	fi

	if [[ -z $SERVER_PG_VER ]]; then
		echo "Cannot work out postgres server version"
		exit 1
	fi
:
: ------------------------------
: Discover postsgresql db socket
: ------------------------------
:
: Version to port can be found in /etc/postgresql/NN/main/postgresql.conf
:
: Required in case multiple postgres versions are running
: and psql default connects to the wrong one.
:
: Info
: ----
:
	grep '^\s*port\s*=\s*\([0-9]\)*' /etc/postgresql/*/main/postgresql.conf || true
:
	pgrep postgres -a|grep postgresql || true
:
	ls -l /usr/lib/postgresql/ || true
:
	ls -l /run/postgresql/ || true
:
: Ports
: -----
:
	if [[ -n $PG_VER ]]; then
		PG_PORT=`grep '^\s*port\s*=\s*\([0-9]\)*' /etc/postgresql/$PG_VER/main/postgresql.conf|grep [0-9]* -o || true`
		PSQL_PORT_OPT=-p$PG_PORT
: For exodus programs like dict2sql and testsort
		export EXO_PORT=$PG_PORT
	else
		PG_PORT=
		PSQL_PORT_OPT=
	fi
:

function get_dependencies_for_build {
:
: --------------------------
: GET DEPENDENCIES FOR BUILD
: --------------------------
:
: Postgresql package
: ------------------
:
	sudo apt install -y postgresql-common
:
: pgexodus and fmt submodules
: ---------------------------
:
	if ! test -f exodus/pgexodus/CMakeLists.txt || ! test -f fmt/CMakeLists.txt; then
		git submodule init
		git submodule update
	fi
:
: Get the full postgres debian repos IF we require a specific version
: -------------------------------------------------------------------
	if [[ -n $PG_VER ]]; then
		yes | sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh || true
	fi
:
: List installed postgresql
:
	apt list postgresql* --installed
:
: List available postgresql
:
	apt list postgresql*dev*
:
: exodus and pgexodus
: -------------------
:
	sudo apt install -y cmake
:
: exodus
: ------
:
	if [[ $COMPILER == gcc ]]; then
		sudo apt install -y g++
	else
		sudo apt install -y clang
		sudo update-alternatives --set c++ /usr/bin/clang++
		sudo update-alternatives --set cc /usr/bin/clang
:
		apt list libstdc++*dev --installed || true
:
		dpkg -S /usr/include/c++ || true
:
		apt list libstdc++*dev || true
:
: Remove troublesome latest versions of libstdc++ which are troublesome to clang 14 on Ubuntu 22.04
:
		if [[ `lsb_release -rs` == 22.04 ]]; then
			sudo apt remove -y libstdc++-12-dev || true
			sudo apt remove -y libstdc++-13-dev || true
			sudo apt autoremove -y || true
:
			apt list libstdc++*dev --installed || true
		fi
	fi
:
	sudo apt install -y libpq-dev libboost-regex-dev libboost-locale-dev
	#sudo apt install -y g++ libboost-date-time-dev libboost-system-dev libboost-thread-dev
:
: pgexodus
: --------
:
	ls -l /usr/lib/postgresql || true
:
	sudo apt install -y postgresql-server-dev-$SERVER_PG_VER
	sudo apt install -y postgresql-common
:
	pg_config
:
	ls -l /usr/lib/postgresql/ || true
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
: Info
:
	ls -l /usr/lib/gcc/x86_64-linux-gnu/ || true
:
	dpkg -S /usr/lib/gcc/x86_64-linux-gnu || true
:
	dpkg -S /usr/include/c++ || true
:
	c++ -v -print-search-dirs || true
:
: Build
:
	cd $EXODUS_DIR
	git submodule init
	git submodule update
	echo PGPATH=${PGPATH:-}
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
	sudo apt install -y postgresql$PG_VER_SUFFIX #for pgexodus install
}

function install_all {
: -------
: INSTALL
: -------
:
	sudo cmake --install $EXODUS_DIR/build
:
#:
#: -------------------------------
#: Test exodus except db functions
#: -------------------------------
#:
#	cd $EXODUS_DIR/build
#	CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
:
: Add \~/bin to all users PATH
: ----------------------------
:
: Enable exodus programs created with edic/compile
: to be run from command line without full path to \~/bin
: Requires re-login after installation.
:
	echo 'export PATH="${PATH}:~/bin"' | sudo dd of=/etc/profile.d/exodus.sh status=none
:
: ----------------------------------------
: Check that postgresql is running locally
: ----------------------------------------
	psql $PSQL_PORT_OPT --version || true
	#pg_ctl start || true
	sudo systemctl start postgresql || true # no systemctl on docker
	sudo systemctl status postgresql || true
	sudo pgrep postgres -a || true
	ls /var/run/postgresql || true
:
: Optional grant others cd into HOME.
: -----------------------------------
:
: Removes warning by enable others/postgres to cd into build dir
: 'could not change directory to "/root": Permission denied'
:
	sudo chmod o+x $HOME
:
: -------------------------------
: Configure postgresql for exodus
: -------------------------------
:
	# Waiting for postgresql to start?
	#(while ! nc -z -v -w1 localhost 5432 2>/dev/null; do echo "Waiting for port 5432 to open..."; sleep 2; done)
:
: Install into template1
: ----------------------
:
: --  extension 'pgexodus'
: --  collation 'exodus_natural'
: --  extension 'unaccent' and function 'immutable_unaccent'
: --  schema 'dict'
: --  table 'lists'
: --  table 'dict.voc'
:
	sudo -u postgres psql $PSQL_PORT_OPT template1 < $EXODUS_DIR/install_template1.sql
:
: Grant exodus login, createrole and createdatabase but not superuser
: -------------------------------------------------------------------
:
	sudo -u postgres psql $PSQL_PORT_OPT <<-V0G0N
		--
		-- user 'exodus'
		--
	    -- The role was created without login permission when creating the extension
	    -- since its objects are assigned to exodus.
	    --
	    -- Allow login with password and creation of users and databases
	    -- but not superuser
		--
	    ALTER ROLE exodus
	        LOGIN
	        PASSWORD 'somesillysecret'
	        CREATEDB
	        CREATEROLE
	    ;
V0G0N
:
: Create exodus user login and database
: -------------------------------------
:
: 1. exodus user login/password
: 2. exodus database
:
	#sudo -u postgres psql $PSQL_PORT_OPT < $EXODUS_DIR/install_exodus.sql
	if ! sudo -u postgres psql $PSQL_PORT_OPT exodus -c 'select version();'; then
		sudo -u postgres psql $PSQL_PORT_OPT -c 'CREATE DATABASE exodus OWNER exodus;'
	fi
:
: Install into into exodus database as well.
: ------------------------------------------
:
: Only required if exodus db already existed and was
: therefore not created from template1 in above step.
:
	sudo -u postgres psql $PSQL_PORT_OPT exodus < $EXODUS_DIR/install_template1.sql
:
: Configure exodus for postgres
: -----------------------------
:
	mkdir -p ~/.config/exodus
	echo "host=127.0.0.1 port=${EXO_PORT:-5432} dbname=exodus user=exodus password=somesillysecret" > ~/.config/exodus/exodus.cfg
:
: Lots of pgsql utility functions
: -------------------------------
:
: Necessary to test many exodus pgsql function e.g. exodus_date
:
: TODO make sure it connects to the right postgresql service/port
:
	dict2sql
:
}

function test_all {
: ----
: TEST
: ----
:
: SERVER_PG_VER ${SERVER_PG_VER} EXO_PORT=$EXO_PORT
:
	sudo systemctl start postgresql
:
: Many tests
: ----------
:
	cd $EXODUS_DIR/build && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
:
: Demo program
: ------------
:
	testsort
:
: 'Recommended: "export PATH=${PATH}:~/bin"'
: 'or logout/login to get new path from /etc/profile.d/exodus.sh'
: 'which will enable running exodus programs created by edic/compile'
: 'from the command line without prefixing ~/bin/'
}
function install_www_service {
: -------------------
: Install www service
: -------------------
:
	cd $EXODUS_DIR/service
	./install_all.sh
}
:
#function build_service {
#: -------------
#: BUILD SERVICE
#: -------------
#:
#	cd $EXODUS_DIR/service/src
#	./compall
#}
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

	[[ $STAGES =~ W ]] && install_www_service
:
: ----------------------------------------------------------------
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs.
: ----------------------------------------------------------------
