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
: Ubuntu 20.04.1 LTS
:
: No longer tested:
:
: Ubuntu 18.04.1 LTS
: Ubuntu 10.04.2 LTS
: Linux lucid64 2.6.32-31-server #61-Ubuntu SMP Fri Apr 8 19:44:42 UTC 2011 x86_64 GNU/Linux
: Debian GNU/Linux 6.0 \n \l
: Linux debian32 2.6.32-5-686 #1 SMP Wed May 18 07:08:50 UTC 2011 i686 GNU/Linux
:
:
: 1. Building and Installing Exodus
: =================================
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
		libboost-dev libboost-system-dev libboost-regex-dev libboost-thread-dev libboost-locale-dev libboost-chrono-dev libboost-date-time-dev libboost-atomic-dev
:
: Download exodus if not already present
:
	cd ~
	[ ! -d exodus ] && git clone https://github.com/exodusdb/exodusdb exodus
:
: Refresh git in case reinstalling
:
	cd ~/exodus
	git pull
:
: Config make with all cpus in parallel
:
	cd ~/exodus
	cmake .
:
: Clean all unless suppressed. In case half-built e.g.with wrong boost libs
:
	[ $NOCLEAN ] || make clean
	cmake .

:
: Make all
:
	make -j `nproc`
:
: Install all
:
	sudo make install
:
: 2. Installing Postgres and Configuring it for Exodus
: ====================================================
:
: Install the postgresql client package and restart postgresql
:
	#yum -y install postgresql-server
	sudo DEBIAN_FRONTEND=noninteractive \
	apt-get -y install postgresql postgresql-client
	#/etc/init.d/postgresql reload
	#sudo /etc/init.d/postgresql reload
	sudo /etc/init.d/postgresql restart
:
: Remove all the following since replicated in pgexodus install scripts
:
#: Create a script file to install pgexodus extension functions
#: as postgres superuser into the template1 database.
#: Also create exodus user and database, and dict schema in template1
#:
#:
#	cat > /tmp/exoduspg.input << EOF
##!/bin/bash
#
#psql -U postgres -d template1 << EOF2
#\connect template1
#SET client_min_messages = warning;
#
#CREATE OR REPLACE FUNCTION exodus_extract_text(data text, fn int4, vn int4, sn int4)     RETURNS text      AS 'pgexodus', 'exodus_extract_text'     LANGUAGE C IMMUTABLE;
#-- CREATE OR REPLACE FUNCTION exodus_extract_sort(data text, fn int4, vn int4, sn int4)  RETURNS text      AS 'pgexodus', 'exodus_extract_sort'     LANGUAGE C IMMUTABLE;
#-- Remaining functions are STRICT therefore never get called with NULLS also return NULL if passed zero length strings
#-- CREATE OR REPLACE FUNCTION exodus_extract_text2(data text, fn int4, vn int4, sn int4) RETURNS text      AS 'pgexodus', 'exodus_extract_text2'    LANGUAGE C IMMUTABLE STRICT;
#CREATE OR REPLACE FUNCTION exodus_extract_date(data text, fn int4, vn int4, sn int4)     RETURNS date      AS 'pgexodus', 'exodus_extract_date'     LANGUAGE C IMMUTABLE STRICT;
#CREATE OR REPLACE FUNCTION exodus_extract_time(data text, fn int4, vn int4, sn int4)     RETURNS interval  AS 'pgexodus', 'exodus_extract_time'     LANGUAGE C IMMUTABLE STRICT;
#CREATE OR REPLACE FUNCTION exodus_extract_datetime(data text, fn int4, vn int4, sn int4) RETURNS timestamp AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;
#-- Following return 0 for zero length strings
#CREATE OR REPLACE FUNCTION exodus_extract_number(data text, fn int4, vn int4, sn int4)   RETURNS float8    AS 'pgexodus', 'exodus_extract_number'   LANGUAGE C IMMUTABLE STRICT;
#CREATE OR REPLACE FUNCTION exodus_count(data text, countchar text)                       RETURNS integer   AS 'pgexodus', 'exodus_count'            LANGUAGE C IMMUTABLE;
#
#CREATE ROLE exodus
# LOGIN
# PASSWORD 'somesillysecret'
# CREATEDB CREATEROLE;
#
#CREATE SCHEMA dict
# AUTHORIZATION exodus;
#
#CREATE COLLATION IF NOT EXISTS exodus_natural
# (provider = icu, locale = 'en@colNumeric=yes', DETERMINISTIC = false);
#
#CREATE DATABASE exodus
# WITH ENCODING='UTF8'
# OWNER=exodus;
#
#\df exodus*
#\q
#EOF2
#
#EOF
#:
#: Run the script just created
#:
#	chmod a+xr /tmp/exoduspg.input
#	sudo su postgres -c "/tmp/exoduspg.input"
#	rm /tmp/exoduspg.input
#:
#: Remove the script
#:
#	#/etc/init.d/postgresql reload
#	#sudo /etc/init.d/postgresql reload
#	sudo /etc/init.d/postgresql restart
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
: one of the exodus cli programs
:
	dict2sql
:
: 5. Testing Exodus
: =================
: Every database requires a lists file in order to store select lists
:
	createfile lists || true
	cd ~
	testsort
:
: 6. Programming with Exodus
: ==========================
: you must make some change to hello or actually save it, not just quit
: edic hello
: hello
: compile hello
:
: 7. Finished $0 $* in $((SECONDS / 60)) minutes and $((SECONDS % 60)) seconds
: ======================================================================
