#!/bin/bash
set -euxo pipefail

: ===============================================
: 0. Building and Installing Exodus Using apt-get
: ===============================================
:
: You can view/run the latest version of this script directly
:
: "wget -O - https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | less"
: "wget -O - https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash"
: or
: "curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | less"
: "curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash"

:
: Installing on:
:
	uname -a
	cat /etc/issue

:
: Tested on:
:
: "Ubuntu 22.04 Ubuntu Jammy Jellyfish (development branch)"
: "Ubuntu 20.04.3 LTS"
:
: No longer tested:
:
: "Ubuntu 18.04.1 LTS"
: "Ubuntu 10.04.2 LTS"
: "Linux lucid64 2.6.32-31-server" #61-Ubuntu SMP Fri Apr 8 19:44:42 UTC 2011 x86_64 GNU/Linux
: "Debian GNU/Linux 6.0 \n \l"
: "Linux debian32 2.6.32-5-686" #1 SMP Wed May 18 07:08:50 UTC 2011 i686 GNU/Linux

:
: 1. Building and Installing Exodus
: =================================
:
:
: 'Install into $1, $EXODUSDIR or ~/exodus'
:
	EXODUSDIR=${1:-${EXODUSDIR:-~/exodus}}

:
: Update repos in case building on a pristine installation
:
	sudo apt-get update

:
: Install dependencies
:
 	sudo DEBIAN_FRONTEND=noninteractive \
		apt-get -y install \
		git \
		cmake \
		postgresql-server-dev-all \
		g++ build-essential \
		libboost-regex-dev libboost-locale-dev
		# Will be automatically installed
		# atomic chrono date-time serialization system thread
		#
		# was:
		#libboost-dev libboost-system-dev libboost-regex-dev libboost-thread-dev libboost-locale-dev libboost-chrono-dev libboost-date-time-dev libboost-atomic-dev

:
: Download exodus if not already present
:
	cd ~
	[ ! -d exodus ] && git clone https://github.com/exodusdb/exodusdb exodus

:
: Refresh git in case reinstalling
:
	cd $EXODUSDIR
	git stash
	git pull

:
: Config make with all CPUs in parallel
:
	cd $EXODUSDIR
	cmake . -D CMAKE_BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo}

:
: Clean all - unless suppressed for speed. Required in case half-built with wrong libs somehow.
:
	[ ${NOCLEAN:-} ] || make clean
	cmake .

:
: Make all exodus lib, cli and pgexodus with all CPUs in parallel. Hopefully enough memory.
:
	make -j `nproc`

:
: Skip full testing unless FORCE_TEST is set
:
	${FORCE_TEST:-} && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=`nproc` make test

:
: Install all exodus lib and cli - Probably into /usr/local ... lib, bin, include/exodus and share/exodus
:
	sudo make install

:
: 2. Installing Postgres and Configuring it for Exodus
: ====================================================
:
: Install the postgresql client package
:
        #yum -y install postgresql-server
        sudo DEBIAN_FRONTEND=noninteractive \
        apt-get -y install postgresql postgresql-client

:
: Restart postgres
:
        #/etc/init.d/postgresql reload
        #sudo /etc/init.d/postgresql reload
        sudo /etc/init.d/postgresql restart

:
: make install again to install pgexodus extension and functions, and exodus user and database
:
        cd $EXODUSDIR
        make install

:
: 3. Configuring Exodus for Postgres
: ==================================
:
	mkdir -p ~/.config/exodus
:
	echo host=127.0.0.1 \
	port=5432 \
	dbname=exodus \
	user=exodus \
	password=somesillysecret \
	> ~/.config/exodus/exodus.cfg

:
: 4. Add some postgres utility functions
: ======================================
:
: Use an exodus cli program
:
	dict2sql

:
: 5. Testing Exodus
: =================
:
: Every database requires a lists file in order to store select lists
:
	createfile lists || true
	cd ~
	testsort

:
: 6. Programming with Exodus
: ==========================
: You must make some change to hello or actually save it, not just quit.
: edic hello
: hello
: compile hello

:
: 7. Finished $0 $* in $((SECONDS / 60)) minutes and $((SECONDS % 60)) seconds
: ======================================================================
