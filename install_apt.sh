#!/bin/bash
#=== Building and Installing Exodus Using apt-get ===

#you can view/run the latest version of this script directly
# wget -O - https://github.com/exodusdb/exodusdb/trunk/install_apt.sh | less
# wget -O - https://github.com/exodusdb/exodusdb/trunk/install_apt.sh | bash
#or
# curl https://github.com/exodusdb/exodusdb/trunk/install_apt.sh | less
# curl https://github.com/exodusdb/exodusdb/trunk/install_apt.sh | bash

#tested on:

uname -a
cat /etc/issue

#Ubuntu 10.04.2 LTS
#Linux lucid64 2.6.32-31-server #61-Ubuntu SMP Fri Apr 8 19:44:42 UTC 2011 x86_64 GNU/Linux

#Debian GNU/Linux 6.0 \n \l
#Linux debian32 2.6.32-5-686 #1 SMP Wed May 18 07:08:50 UTC 2011 i686 GNU/Linux

#==== 1. Building and Installing Exodus ====

###yum -y install subversion gcc-c++ postgresql-devel
sudo apt-get -y install subversion g++ postgresql-server-dev* libboost-all-dev build-essential cmake

cd ~
svn co https://github.com/exodusdb/exodusdb/trunk/ exodus

cd ~/exodus
cmake .
./configure
make clean #in case half-built with wrong boost libs
make
sudo make install


#==== 2. Installing Postgres and Configuring it for Exodus ====

#export POSTGRESVER=8.4
export POSTGRESVER=`apt-cache search ^postgresql-[0-9]+.[0-9]+$|tail -n 1|cut -d" " -f 1|cut -d- -f 2`

###yum -y install postgresql-server
sudo apt-get -y install postgresql-$POSTGRESVER
# or just sudo apt-get -y install postgresql

###/etc/init.d/postgresql start
#/etc/init.d/postgresql-$POSTGRESVER start

cat > /tmp/exoduspg.input << EOF
#!/bin/bash

psql -U postgres -d template1 << EOF2
\connect template1
CREATE OR REPLACE FUNCTION exodus_call(bytea, bytea, bytea, bytea, bytea, int4, int4) RETURNS bytea AS 'pgexodus', 'exodus_call' LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION exodus_extract_bytea(bytea, int4, int4, int4) RETURNS bytea AS 'pgexodus', 'exodus_extract_bytea' LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION exodus_extract_text(bytea, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_text' LANGUAGE C IMMUTABLE;
CREATE OR REPLACE FUNCTION exodus_extract_sort(bytea, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_sort' LANGUAGE C IMMUTABLE;
-- Remaining functions are STRICT therefore never get called with NULLS -- also return NULL if passed zero length strings
CREATE OR REPLACE FUNCTION exodus_extract_text2(bytea, int4, int4, int4) RETURNS text AS 'pgexodus', 'exodus_extract_text2' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_date(bytea, int4, int4, int4) RETURNS date AS 'pgexodus', 'exodus_extract_date' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_time(bytea, int4, int4, int4) RETURNS time AS 'pgexodus', 'exodus_extract_time' LANGUAGE C IMMUTABLE STRICT;
CREATE OR REPLACE FUNCTION exodus_extract_datetime(bytea, int4, int4, int4) RETURNS timestamp AS 'pgexodus', 'exodus_extract_datetime' LANGUAGE C IMMUTABLE STRICT; 
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

chmod a+xr /tmp/exoduspg.input
sudo su postgres -c "/tmp/exoduspg.input"
rm /tmp/exoduspg.input

#/etc/init.d/postgresql reload
sudo /etc/init.d/postgresql-$POSTGRESVER reload


#==== 3. Configuring Exodus for Postgres ====

echo host=127.0.0.1 \
port=5432 \
dbname=exodus \
user=exodus \
password=somesillysecret \
> ~/.exodus


#==== 4. Testing Exodus ====

cd ~
testsort


#==== 5. Programming with Exodus ====

#you must make some change to hello or actually save it, not just quit
#edic hello
#hello
#compile hello

