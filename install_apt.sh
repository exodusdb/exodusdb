#!/bin/bash
set -x
:
: === Building and Installing Exodus Using apt-get ===
:
: you can view/run the latest version of this script directly
: "wget -O - https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | less"
: "wget -O - https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash"
: or
: "curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | less"
: "curl https://raw.githubusercontent.com/exodusdb/exodusdb/master/install_apt.sh | bash"
:
: tested on:
:
uname -a
cat /etc/issue
:
: Ubuntu 20.04 LTS
: Ubuntu 18.04 LTS
: Ubuntu 10.04.2 LTS
: Linux lucid64 2.6.32-31-server #61-Ubuntu SMP Fri Apr 8 19:44:42 UTC 2011 x86_64 GNU/Linux
:
: Debian GNU/Linux 6.0 \n \l
: Linux debian32 2.6.32-5-686 #1 SMP Wed May 18 07:08:50 UTC 2011 i686 GNU/Linux
:
SECONDS=0
set -x
:
: ==== 1. Building and Installing Exodus ====
:
: required in case building on a pristine installation
sudo apt-get update
:
: ##yum -y install subversion gcc-c++ postgresql-devel
#sudo apt-get -y install subversion g++ postgresql-server-dev-all libboost-all-dev build-essential cmake
sudo apt-get -y install git g++ postgresql-server-dev-all libboost-all-dev build-essential cmake
:
cd ~
#svn co https://github.com/exodusdb/exodusdb/trunk/ exodus
git clone https://github.com/exodusdb/exodusdb exodus
:
cd ~/exodus
#./configure
cmake .
make clean #in case half-built with wrong boost libs
cmake .
make
sudo make install
:
: ==== 2. Installing Postgres and Configuring it for Exodus ====
:
###yum -y install postgresql-server
sudo apt-get -y install postgresql postgresql-client
:
cat > /tmp/exoduspg.input << EOF
#!/bin/bash

psql -U postgres -d template1 << EOF2
\connect template1

CREATE OR REPLACE FUNCTION exodus_count(text, text) RETURNS integer AS 'pgexodus', 'exodus_count' LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION exodus_extract_text(text, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_text' LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION exodus_extract_sort(text, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_sort' LANGUAGE C IMMUTABLE;
-- Remaining functions are STRICT therefore never get called with NULLS also return NULL if passed zero length strings
CREATE OR REPLACE FUNCTION exodus_extract_text2(text, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_text2' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_date(text, int4, int4, int4) RETURNS date AS 'pgexodus', 'exodus_extract_date' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_time(text, int4, int4, int4) RETURNS time AS 'pgexodus', 'exodus_extract_time' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_datetime(text, int4, int4, int4) RETURNS timestamp AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_number(text, int4, int4, int4) RETURNS float8 AS 'pgexodus', 'exodus_extract_number' LANGUAGE C IMMUTABLE STRICT;

CREATE ROLE exodus LOGIN
 PASSWORD 'somesillysecret'
 CREATEDB CREATEROLE;
CREATE DATABASE exodus
 WITH ENCODING='UTF8'
   OWNER=exodus;
\df exodus*
\q
EOF2

EOF
:
chmod a+xr /tmp/exoduspg.input
sudo su postgres -c "/tmp/exoduspg.input"
rm /tmp/exoduspg.input
:
#/etc/init.d/postgresql reload
sudo /etc/init.d/postgresql reload
sleep 5
:
: ==== 3. Configuring Exodus for Postgres ====
:
echo host=127.0.0.1 \
port=5432 \
dbname=exodus \
user=exodus \
password=somesillysecret \
> ~/.exodus
:
: ==== 4. Add some postgres utility functions ====
:
:one of the exodus cli programs
dict2sql
:
: ==== 5. Testing Exodus ====
:
cd ~
testsort
:
: ==== 6. Programming with Exodus ====
:
:you must make some change to hello or actually save it, not just quit
:edic hello
:hello
:compile hello
:
duration=$SECONDS
echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed."
