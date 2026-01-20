#!/bin/bash
[ $SHLVL -le 3 ] && exec > >(tee -a "$(basename `echo ${0/.sh/}.$*.log | sed 's/ /\./g'`)") 2>&1
set -euxo pipefail
PS4='+ [install ${SECONDS}s] '
:
: $0 $*
: ==========================================================
: Build, install and test exodus
: ==========================================================
:
: 'Default clang compiler on Ubuntu 24.04 is 19 since 20 has problems'
:
: 'Output from cli/exodus by OS for 2 compilers'
: '"c++ 21" indicates c++20 plus informal support for c++23'
: '"c++ 24" indicates c++23 plus informal support for c++26'
:
: 'Build: Ubuntu 25.10 x64 gcc   15 c++ 24'
: 'Build: Ubuntu 24.04 x64 gcc   14 c++ 23'
: 'Build: Ubuntu 22.04 x64 g++   11 c++ 21'
: 'Build: Ubuntu 20.04 x64 g++    9 c++ 17'
:
: 'Build: Ubuntu 25.10 x64 clang 21 c++ 24'
: 'Build: Ubuntu 24.04 x64 clang 19 c++ 24'
: 'Build: Ubuntu 22.04 x64 clang 14 c++ 21'
: 'Build: Ubuntu 20.04 x64 clang 10 c++ 20'
:
: '------ ------------ ----- --------  ----    -----   -----'
: 'Status Ubuntu  LTS  Name  Postgres  g++     clang   boost'
: '------ ------------ ----- --------  ----    -----   -----'
: 'OK     25.10        quest 17.6      15.2.0  21.1.2  88   '
:
: 'OK     24.04   Yes  noble 16.3?     13.3    19.1.1  83   clang-18/19 and g++ work. clang-20 doesnt.'
: 'KO     24.04   Yes  noble 16.3?     14.2    20.?    83   latest'
: 'OK     24.04   Yes  noble 16.3      13.3.0  18.1.3  83   default'
:
: 'OK     23.10              15.4      13.2    16           '
: 'OK     23.04              15.4      12.3                 '
: 'OK     22.04.3 Yes  jammy 14.9      11.4    14.0    74   '
: 'OK     20.04.6 Yes  focal 12.16      9.4    10      71   '
: '------ -----------  ----- --------  ----    -----   -----'
: 'KO     18.04   Yes        10                           '
: '------ -----------  ----- --------  ----  -----   -----'
: 'Exodus now requires c++20 so will no longer build on 18.04'
:
: min/default/latest compiler version
: --------------------------------
:
: OS      g++             clang
:         min def latest  min def latest
: 24.04    09  13  14      14  18  18
: 22.04    09  11  12      11  14  15
: 20.04    07  09  10      07  10  12
:
: OS          Cmake
: Noble 24.04 3.28.3
: jammy 22.04 3.22.1
: focal 20.04 3.16.3

: ------
: Syntax
: ------
:
: $0 ' <STAGES> [<COMPILER>] [<PG_VER>]'
:
: STAGES
:
	ALL_STAGES=bBdDTW
	SUB_STAGES=tI
	DEFAULT_STAGES=bBdDT
:
:	STAGES must be one or more _consecutive_ letters from $ALL_STAGES or from $SUB_STAGES, or A for all stages except W, or AW for all stages.
:
:	A = All stages $DEFAULT_STAGES except W - Web service
:	AW= All stages including W - Web service
:
:	b = Get dependencies for build and install.
:	B = Build, test and install.
:	I = Install exodus. Part of B.
:
:	d = Get dependencies for database.
:	D = Install database.
:
:	t = Test without database. Part of B.
:	T = Test with database.
:
:	W = Install web service.
:
: COMPILER
:
:	"clang or g++ for latest version available. Optionally followed by -version."
:	"clang-18, clang-min, clang-latest, clang-default"
:	"g++-14, g++-min, g++-latest, g++-default"
:
:	If version no is omitted, the LATEST available for the OS will be used except clang 19 on Ubuntu 24.04
:
:	Choosing compiler/version only has effect on the initial b stage when installing build dependencies.
:
: PG_VER
:
:	e.g. 14 or default depends on apt and the Ubuntu version
:
: Parse command line
: ------------------
:
	REQ_STAGES=${1:?STAGES is required and must be one or more *consecutive* letters from $ALL_STAGES or from $SUB_STAGES, or A for all stages except W, or AW for all stages}
	COMPILER=${2:-clang}
	PG_VER=${3:-}
:
: Default clang compiler is 19 on Ubuntu since 20 has problems
:
	if [[ $(lsb_release -rs 2>/dev/null) == "24.04" ]] && [[ $COMPILER == @(clang|clang-default) ]]; then
		COMPILER=clang-19
	fi
:
: Remove the compiler phrase -latest since we treat empty as meaning -latest
:
	COMPILER=${COMPILER/-latest/}
	COMPILER_OR_DEFAULT=${COMPILER/-default/}
:
	CMAKE_BUILD_OPTIONS=-GNinja
#	BUILD_DEPS="ninja-build libfmt-dev libreadline-dev"
	BUILD_DEPS="libreadline-dev"
#    if [[ $COMPILER =~ ^clang ]]; then
#		BUILD_DEPS="$BUILD_DEPS ${COMPILER_OR_DEFAULT/clang/clang-tools}"
#	fi

:
: Validate
: --------
:
	if [[ $REQ_STAGES == AW ]]; then
		REQ_STAGES={$DEFAULT_STAGES}W
	fi
	if [[ $REQ_STAGES == A ]]; then
		REQ_STAGES=$DEFAULT_STAGES
	fi
	if [[ ! $ALL_STAGES =~ $REQ_STAGES ]]; then
		if [[ ! $SUB_STAGES =~ $REQ_STAGES ]]; then
			echo STAGES "'$REQ_STAGES'" must be one or more consecutive letters from $ALL_STAGES or from $SUB_STAGES
			exit 1
		fi
	fi

	# duplicate code in install.sh and install_lxc.sh
	if [[ ! $COMPILER =~ ^((g\+\+)|(clang))(-(([0-9]+)|latest|min|default))?$ ]]; then
		echo COMPILER must be clang or g++ for latest version available. Optionally followed by a version e.g. clang-18, clang-min, clang-latest, clang-default, g++-14, g++-min, g++-latest, g++-default
		exit 1
	fi

:
: ------
: CONFIG
: ------
:
	export DEBIAN_FRONTEND=noninteractive
	#EXODUS_DIR=${GITHUB_WORKSPACE:-~/exodus}
	export EXODUS_DIR=$(pwd)
	# Set by github action if chosen to rerun in debug mode?
	#RUNNER_DEBUG=1

:
: Function to retry three times in case of timeout
: ------------------------------------------------
:
function CMD_RETRY {
:
	for N in 1 2 3; do
:
: Retry three times if it times out. $N/3
: ---------------------------------------
:
		# /dev/null to stop timeout causing random hang until timeout
		# with apt process stuck on tcsetattr call. see gdb -p 9999
		if timeout 300s $* < /dev/null; then
			break
		fi
	done
}

:
: Function to call apt-get install three times in case of timeout
: ---------------------------------------------------------------
:
function APT_INSTALL {
:
	CMD_RETRY sudo apt-get install -y $*
:
: Verify all packages are installed
:
	for pkg in $*; do dpkg -s "$pkg" >/dev/null 2>&1 || { echo "See above 'Unable to locate package'"; exit 1; }; done; echo "All packages installed."
}

:
: Function to download submodules
: -------------------------------
:
function download_submodules {
:
#: Note that main git repo contains just a hash/pointer to the commit to be checked out,
#: not a tag to master, and the initial state will be "HEAD detached".
#:
#	# Does having branch = master in .submodules have any effect?
#	git submodule init
##	git submodule update --remote
#	git submodule update
#:
#: Reconnect submodule to HEAD in case development/push desired
#:
##	git submodule foreach git pull origin master
#	git submodule foreach git checkout master
#	git submodule foreach git pull
##	git submodule foreach git reset --hard
	git submodule init
	git submodule update --recursive --init
	git submodule sync --recursive
	git submodule status --recursive
#	git submodule foreach 'git switch master || git checkout -b master; git reset --soft HEAD'
	git submodule foreach 'git switch master || git checkout -b master; git reset --soft HEAD; git branch --set-upstream-to=origin/master master 2>/dev/null || true'

:	"Verifying submodules..."
	git submodule foreach 'git fetch origin && echo "Submodule: $name" && git branch -r | grep -q "origin/master" && echo "  - Has origin/master: Yes" || echo "  - Has origin/master: No"; git branch -r --contains $(git rev-parse HEAD) | grep -q "origin/master" && echo "  - HEAD $(git rev-parse --short HEAD) is on origin/master: Yes" || echo "  - HEAD $(git rev-parse --short HEAD) is on origin/master: No"'
:	"Verification complete."

:
: Verify submodules exist
: -----------------------
:
	test -f $EXODUS_DIR/fmt/CMakeLists.txt
	test -f $EXODUS_DIR/pgexodus/CMakeLists.txt
}

:
: Wait for systemctl to get its brain in gear and be ready to serve hostname to sudo
: ----------------------------------------------------------------------------------
:
: Loop until success or timeout
:
	while ! hostnamectl status > /dev/null && [[ $SECONDS -lt 60 ]]; do sleep 2; done
:
: Generate error if timeout reached. Otherwise display various host info.
:
	hostnamectl status

:
: ---------------------------------------------
: Disable unattended upgrades until next reboot
: ---------------------------------------------
:
	sudo systemctl disable --runtime apt-daily.timer apt-daily-upgrade.timer unattended-upgrades
#	sudo systemctl mask --runtime unattended-upgrades apt-daily.timer apt-daily-upgrade.timer
	sudo systemctl stop unattended-upgrades apt-daily.timer apt-daily-upgrade.timer

:
: ----------
: Update apt
: ----------
:
#	rm /var/lib/dpkg/lock-frontend || true

	ls -l /var/cache/apt/ 2> /dev/null || true
	while ! ls /var/cache/apt/*.bin 2> /dev/null && ! sudo apt-get -y update; do
		sleep 1
	done

:
: -----------------------------------------
: Work out postgres version suffix e.g. -14
: -----------------------------------------
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
		SERVER_PG_VER=$(apt-cache search postgresql-server-* 2> /dev/null|grep -o -P 'dev-[0-9]{2}\b'|sort|tail -n1|cut -d- -f2||true)
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
: Info about postgres
: -------------------
:
	grep '^\s*port\s*=\s*\([0-9]\)*' /etc/postgresql/*/main/postgresql.conf || true
:
	pgrep postgres -a|grep postgresql || true
:
	ls -l /usr/lib/postgresql/ 2> /dev/null || true
:
	ls -l /run/postgresql/ 2> /dev/null || true
:
: Postgres ports
: --------------
:
	if [[ -n $PG_VER ]]; then
		PG_PORT=`grep '^\s*port\s*=\s*\([0-9]\)*' /etc/postgresql/$PG_VER/main/postgresql.conf|grep [0-9]* -o || true`
		#PSQL_PORT_OPT=-p$PG_PORT
		PSQL_PORT_OPT=''
		if [ "$PG_PORT" ]; then
			PSQL_PORT_OPT="-p $PG_PORT"
		fi
: For exodus programs like dict2sql and testsort
		export EXO_PORT=$PG_PORT
	else
		PG_PORT=
		PSQL_PORT_OPT=
	fi
:

function get_dependencies_for_build_and_install {
:
: --------------------------------------
: GET DEPENDENCIES FOR BUILD AND INSTALL $*
: --------------------------------------
:
: Update apt
: ----------
:
	CMD_RETRY sudo apt-get -y update

:
: Download Postgresql dev and client package
: ------------------------------------------
:
	#Uninstall postgresql-server-dev-all for old existing installations
	#otherwise all postgres versions are installed and pg_config outputs latest version
	#details instead of the specified PG_VER failing pgexodus testing
	#Specified PG_VER for postgresql-server-dev-NN installed later in this stage
	# 1/49 Test  #1: pgexodus_test ....................***Failed    0.08 sec
	#  grep: /etc/postgresql/16/main/postgresql.conf: No such file or directory
	CMD_RETRY sudo apt-get remove -y 'postgresql-server-dev-all' && CMD_RETRY sudo apt-get -y autoremove || true
	APT_INSTALL postgresql-common

:
: Download pgexodus and fmt submodules source in b and B stages
: -------------------------------------------
:
	download_submodules

:
: Get the full postgres debian repos IF we require a specific version
: -------------------------------------------------------------------
	if [[ -n $PG_VER ]]; then
		yes | sudo /usr/share/postgresql-common/pgdg/apt.postgresql.org.sh || true
	fi

:
: List installed postgresql
: -------------------------
:
	apt list postgresql* --installed |& grep postgresql || echo No postgresql

:
: List available postgresql
: -------------------------
:
	apt list postgresql*dev* |& grep postgresql || echo No postgresql

:
: Remove local libfmt if installed
: --------------------------------
:
#	if [[ -d /usr/local/include/fmt ]]; then
		sudo rm /usr/local/include/fmt -rf
		sudo rm /usr/local/lib/libfmt* -f
		sudo rm /usr/local/lib/cmake/fmt/ -rf
		sudo ldconfig
#	fi

:
: Download and install build dependencies for exodus and pgexodus
: ---------------------------------------------------------------
:
: Repeated in Build stage
:
#	CMD_RETRY sudo snap install cmake --classic
	CMD_RETRY ./install_cmake.sh
	APT_INSTALL $BUILD_DEPS

:
: Determine actual compiler version if min/default requested
: ------------------------------------------------------
:
	COMPILER_VERSION=`echo $COMPILER | cut -d'-' -f2`
	if ! [[ $COMPILER_VERSION == "default" || $COMPILER_VERSION =~ ^-?[0-9]+$ ]]; then
		COMPILER_NAME=`echo $COMPILER|cut -d'-' -f1`
		if [[ $COMPILER_VERSION = min ]]; then
			HEAD_OR_TAIL=head
		else
			HEAD_OR_TAIL=tail
		fi
		COMPILER_VERSION=`apt search $COMPILER_NAME |& grep "$COMPILER_NAME-[0-9][0-9.]*" -o | grep '[0-9]*' -o|sort -n|uniq|$HEAD_OR_TAIL -n1`
		COMPILER=$COMPILER_NAME-$COMPILER_VERSION
	fi

:
: Download clang llvm if clang
: ----------------------------
:
	if [[ $COMPILER =~ ^clang ]]; then
:
: 'llvm.sh registers and installs a single presumably stable version, (current default is 20 - 1:20.1.8) regardless of OS.'
: 'if the llvm version is <= the existing ubuntu version then apt prefers the ubuntu version for installion.'
: 'Available clang versions currently are 14-21 (with 22 a development version)'
:
#	CLANG_MAJOR=22
		CLANG_MAJOR=$COMPILER_VERSION
#		CMD_RETRY sudo wget https://apt.llvm.org/llvm.sh
#		chmod +x llvm.sh
#		sudo ./llvm.sh $CLANG_MAJOR
		./install_clang.sh $CLANG_MAJOR
		./install_icu.sh
		./install_boost.sh
	else

:
: Download and install compiler $COMPILER
: ---------------------------------------
:
: e.g. g++, g++-12, clang, clang-12 etc.
:
	APT_INSTALL $COMPILER_OR_DEFAULT

:
: Set as the default ++ compiler
:
: NB SETTING c++ not g++ or clang
:
	sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/$COMPILER_OR_DEFAULT 0
	sudo update-alternatives --set c++ /usr/bin/$COMPILER_OR_DEFAULT
	readlink `which c++` -e

:
: Set as the default c compiler. Why is this necessary?
: 24.04 clang postgres c build fails with an invalid compiler flag -Weverything
:
	C_COMPILER=${COMPILER_OR_DEFAULT//+/c} #g++XXXXX becomes gccXXXXX
	sudo update-alternatives --install /usr/bin/cc cc /usr/bin/$C_COMPILER 0
	sudo update-alternatives --set cc /usr/bin/$C_COMPILER
	readlink `which cc` -e

	sudo update-alternatives --install /usr/bin/c++ clang++ /usr/bin/clang++-21 0
	sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-21 0
:
: Show libstdc++ and clang information
: ------------------------------------
:
	apt list libstdc++*dev --installed |& grep libstdc || true
:
	dpkg -S /usr/include/c++ || true
:
	apt list libstdc++*dev |& grep libstdc || true

:
: Check if clang compiler
: -----------------------
:
	if [[ $COMPILER =~ ^clang ]]; then

:
: Clang module building needs its scan tools in the path
: ------------------------------------------------------
:
:	Avoid "CMAKE_CXX_COMPILER_CLANG_SCAN_DEPS" error
	CLANG_VERSION=`c++ --version | head -n1|cut -d'.' -f 1|grep -Po '\d+'`
	APT_INSTALL clang-tools-$CLANG_VERSION

#:
#: Prevent clang from using later versions of gcc tool chains which are troublesome
#: Force clang to use same version of the gcc tool chain as gcc
#: See initial info on libstdc++ in install log - Build stage, above and below.
#:
#		GCC_VERSION=$(gcc -v|&grep gcc\ version|cut -d' ' -f3|cut -d'.' -f1)
#		GCC_FAKE_VERSION=99
#		sudo ln -snf /usr/lib/gcc/x86_64-linux-gnu/$GCC_VERSION /usr/lib/gcc/x86_64-linux-gnu/$GCC_FAKE_VERSION
#		sudo ln -snf /usr/include/x86_64-linux-gnu/c++/$GCC_VERSION /usr/include/x86_64-linux-gnu/c++/$GCC_FAKE_VERSION
#		sudo ln -snf /usr/include/c++/$GCC_VERSION /usr/include/c++/$GCC_FAKE_VERSION
##		sudo rm /usr/lib/gcc/x86_64-linux-gnu/$GCC_FAKE_VERSION -f
##		sudo rm /usr/include/x86_64-linux-gnu/c++/$GCC_FAKE_VERSION -f
##		sudo rm /usr/include/c++/$GCC_FAKE_VERSION -f

	fi

:
: Download and install dev packages for postgresql client lib and boost
: ---------------------------------------------------------------------
:
	APT_INSTALL libpq-dev libboost-regex-dev libboost-locale-dev libboost-fiber-dev libboost-context-dev
	#APT_INSTALL g++ libboost-date-time-dev libboost-system-dev libboost-thread-dev

:
: End of not clang
:
	fi # not clang llvm

:
: Download, build and install ninja
: ---------------------------------
:
	apt remove ninja-build -qq || true
	./install_ninja.sh

:
: Download and install pgexodus postgres build dependencies
: ---------------------------------------------------------
:
	ls -l /usr/lib/postgresql || true
:
	APT_INSTALL postgresql-server-dev-$SERVER_PG_VER
	APT_INSTALL postgresql-common
:
	pg_config
:
	ls -l /usr/lib/postgresql/ || true

:
: Install pygments syntax highlighter for cli gendoc
: --------------------------------------------------
:
	APT_INSTALL python3-pygments # || FAILED to install pygments

:
: Install exodus lexer plugin for pygment syntax highlighter
: ----------------------------------------------------------
:
	cd $EXODUS_DIR/pygment
	./install.sh && ./test.sh && echo ok # || echo FAILED to install pygment plugin
	cd ..

:
: Show installed compilers and standard library versions
: ------------------------------------------------------
:
	readlink `which c++` -e
	dpkg -l | egrep "g++|clang|libstd|libc\+\+" | awk '{print $2}'

} # end of stage b - Install dependencies

function build_only {
:
: -----
: BUILD $*
: -----
:
: 1. libexodus
: 2. exodus cli
: 3. test
: 4. pgexodus extension
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
: -----
:
	cd $EXODUS_DIR

:
: Download pgexodus and fmt submodules source in b and B stages
: -------------------------------------------------------------
:
	download_submodules

:
: Repeated from Build stage
:
#	CMD_RETRY sudo snap refresh cmake
	APT_INSTALL $BUILD_DEPS

:
: Patch /usr/include/c++/*/ostream
: --------------------------------
:
: Duplicate code in install.sh and CMakeLists.txt - KEEP IN SYNC
:
: For more info, see vardefs.h
:
	for FILENAME in `ls /usr/include/c++/*/ostream || true`; do
		sudo sed -i "s|^# include <format>|# ifndef EXO_FORMAT\\n#  include <format>\\n# endif|" $FILENAME
		sudo sed -i "s|^#if __cpp_lib_print|#if !defined(EXO_FORMAT) \\&\\& __cpp_lib_print|" $FILENAME
	done

:
: Build exodus
: ------------
:
: Prep
:
	echo PGPATH=${PGPATH:-}
	rm $EXODUS_DIR/build -rf
	cmake -L -S $EXODUS_DIR -B $EXODUS_DIR/build $CMAKE_BUILD_OPTIONS #--warn-uninitialized --trace-expanded
:
: Build
:
	#cd $EXODUS_DIR/build
	#make -j$((`nproc`+1))
	#ninja
	#cmake --build $EXODUS_DIR/build -j$((`nproc`+1))
	cmake --build $EXODUS_DIR/build
:
} # end of substage B

function test_exodus_without_database {
:
: Run tests that do not require database
: --------------------------------------
:
#: 1. Test in build/test/src to avoid testing pgexodus lib since postgres is not installed yet
: 1. Test in build/test to avoid testing pgexodus lib since postgres is not installed yet
: 2. EXO_NODATA=1 causes tests related to database to pass automatically
:
	#cd $EXODUS_DIR/build/test/src && EXO_NODATA=1 CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
	cd $EXODUS_DIR/build/test && EXO_NODATA=1 CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
	sleep 1
:
} # end of test_exodus_without_database

function install_exodus {
:
: Install exodus $*
: --------------
:
: Also installs pgexodus.so to /usr/lib/postgresql but not actually required.
:
	NO_INSTALL=${NO_INSTALL:-}
	if [ "$NO_INSTALL" = '' ]; then
		sudo cmake --install $EXODUS_DIR/build
	fi

:
: Add \~/bin to PATH and \~/lib to LD_LIBRARY_PATH for all users
: ------------------------------------------------------------
:
: Enable exodus programs created with edic/compile
: to be run from command line without full path to \~/bin
: Requires re-login after installation.
:
: Note: Using 'echo sudo dd' trick because 'sudo echo xxx > yyy' doesnt sudo the '> yyy' bit.
:
	printf 'export PATH="${PATH}:~/bin"\nexport LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${HOME}/lib"\n' | sudo dd of=/etc/profile.d/exodus.sh status=none
:
} # end of install_exodus

function get_dependencies_for_database {
:
: ----------------------------------------------
: DOWNLOAD AND INSTALL DEPENDENCIES FOR DATABASE $*
: ----------------------------------------------
:
: exodus
: ------
:
	#postgresql-client-$PG_VER already installed by ?
:
: pgexodus
: --------
:
	APT_INSTALL postgresql$PG_VER_SUFFIX #for pgexodus install
}

function install_database {
:
: ----------------
: INSTALL DATABASE $*
: ----------------
:
	# Already installed in b stage along main exodus
	# but here only the postgresl extension is installed
	#sudo cmake --install $EXODUS_DIR/build
	sudo cmake --install $EXODUS_DIR/build/pgexodus/
#
#-- Install configuration: "RELEASE"
#-- Up-to-date: /usr/lib/postgresql/16/lib/pgexodus.so
#-- Up-to-date: /usr/share/postgresql/16/extension/pgexodus.control
#-- Up-to-date: /usr/share/postgresql/16/extension/pgexodus--1.0.sql
:
#:
#: -------------------------------
#: Test exodus except db functions
#: -------------------------------
#:
#	cd $EXODUS_DIR/build
#	CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
: ----------------------------------------
: Check that postgresql is running locally
: ----------------------------------------
	psql $PSQL_PORT_OPT --version || true
	#pg_ctl start || true
	sudo systemctl start postgresql || sudo systemctl start postgresql || true # no systemctl on docker
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
: Required for the following stage.
:
	sudo chmod o+x $HOME

:
: Test the pgexodus postgresql extension
: --------------------------------------
:
#: Restart postgresql in case regression database is stuck
#: Now done in cmake pgexodus_test
#	sudo systemctl restart postgresql
:
	cd $EXODUS_DIR/build/exodus/pgexodus && CTEST_OUTPUT_ON_FAILURE=1 ctest

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
: Force exodus to create new files in public schema instead of exodus in all databases
: ------------------------------------------------------------------------------------
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
		--
		-- Force exodus to create new files in public schema instead of exodus
		-- in all databases.
		--
		ALTER ROLE exodus SET search_path TO public, exodus;
V0G0N

:
: Create exodus user login and database
: -------------------------------------
:
: 1. exodus user login/password
: 2. exodus database
:
	#sudo -u postgres psql $PSQL_PORT_OPT < $EXODUS_DIR/install_exodus.sql
	if ! sudo -u postgres psql $PSQL_PORT_OPT exodus -c 'select version();' 2>/dev/null; then
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
#
#:
#: Test the pgexodus postgresql extension
#: --------------------------------------
#:
#	cd $EXODUS_DIR/build/exodus/pgexodus && CTEST_OUTPUT_ON_FAILURE=1 ctest
#

:
: Install lots of pgsql utility functions
: ---------------------------------------
:
: Necessary to test many exodus pgsql function e.g. exodus_date - not exodus::extract_date
:
: TODO make sure it connects to the right postgresql service/port
:
	dict2sql
:

} # end of stage D - Install database

function test_exodus_and_database {
:
: ------------------------
: TEST EXODUS AND DATABASE $*
: ------------------------
:
: Postgres version and port - SERVER_PG_VER ${SERVER_PG_VER} and EXO_PORT=$EXO_PORT
: -------------------------
:
: Start postgresql although it should be running
:
	sudo systemctl start postgresql

:
: Run cli/testsort once to generate some test files like xo_clients
: used in testing_var.h.cpp
: -----------------------------------------------------------------
:
	testsort > /dev/null

:
: Many tests using ctest - In parallel. Output only on error
: ----------------------
:
#: Note that all the exodus/build/test/src test to that do not require database were already run
: Note that all the exodus/build/test test to that do not require database were already run
: in the build stage using EXO_NODATA=1 and pgexodus test is omitted
: This test is run in the exodus/build
:
	cd $EXODUS_DIR/build && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$((`nproc`+1)) ctest
	sleep 1
:
: Run the demo program - testsort
: --------------------
:
	testsort
:
: 'Recommended: "su -"'
: 'or logout/login to get new path/libs from /etc/profile.d/exodus.sh'
: 'which will enable running exodus programs created by edic/compile'
: 'from the command line without prefixing ~/bin/'

} # end of stage T - Test exodus and database

function install_www_service {
:
: -------------------
: INSTALL WWW SERVICE
: -------------------
:
	export EXODUS_DIR
	cd $EXODUS_DIR/service
	sudo ./install_all.sh
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
: MAIN $*
: ----
:

	if [[ $REQ_STAGES =~ b ]]; then get_dependencies_for_build_and_install; fi
	if [[ $REQ_STAGES =~ B ]]; then
		build_only
		test_exodus_without_database
		install_exodus
	fi
	if [[ $REQ_STAGES =~ I ]]; then install_exodus; fi # Done as part of B

	if [[ $REQ_STAGES =~ d ]]; then get_dependencies_for_database; fi
	if [[ $REQ_STAGES =~ D ]]; then install_database; fi

	if [[ $REQ_STAGES =~ t ]]; then test_exodus_without_database; fi # Done as part B
	if [[ $REQ_STAGES =~ T ]]; then test_exodus_and_database; fi

	if [[ $REQ_STAGES =~ W ]]; then install_www_service; fi
:
: ================================================================
: Finished $0 $* in $((SECONDS/60)) mins and $((SECONDS%60)) secs.
: ================================================================
