#!/bin/bash
#=== Building and Installing Exodus Using yum ===

#you can view/run the latest version of this script directly
# wget -O - http://exodusdb.googlecode.com/svn/trunk/buildinstall_yum.sh | less
# wget -O - http://exodusdb.googlecode.com/svn/trunk/buildinstall_yum.sh | bash
#or
# curl http://exodusdb.googlecode.com/svn/trunk/buildinstall_yum.sh | less
# curl http://exodusdb.googlecode.com/svn/trunk/buildinstall_yum.sh | bash

#it takes about 5 minutes or 15 minutes on Redhat/Centos 5

#tested on:

 uname -a
 cat /etc/issue

#CentOS release 5.6 (Final)
#Linux localhost.localdomain 2.6.18-238.9.1.el5 #1 SMP Tue Apr 12 18:10:56 EDT 2011 i686 i686 i386 GNU/Linux

#CentOS release 5.6 (Final)
#Linux localhost.localdomain 2.6.18-238.9.1.el5 #1 SMP Tue Apr 12 18:10:13 EDT 2011 x86_64 x86_64 x86_64 GNU/Linux

#Fedora release 15 (Lovelock)
#Linux localhost.localdomain 2.6.38.6-26.rcl.fc15.x86_64 #1 SMP Mon May 9 x86_64 x86_64 x86_64 GNU/Linux

#==== 1. Installing or Building Boost ====

sudo yum -y install icu libicu libicu-devel

#rather dumb version centos/redhat 5 detection
if [ "`cat /etc/issue|grep " 5"`" == "" ]; then

 sudo yum -y install boost-devel boost-date-time boost-filesystem boost-regex boost-system boost-thread

else

 #IT IS CRITICAL THAT BOOST-DEVEL IS *NOT* INSTALLED if building boost as follows.
 sudo yum -y remove boost-devel

 sudo yum -y install gcc-c++

 cd ~
 rm -f boost_1_46_1.tar.gz
 wget http://sourceforge.net/projects/boost/files/boost/1.46.1/boost_1_46_1.tar.gz/download
 tar xfz boost_1_46_1.tar.gz
 cd boost_1_46_1

 #bjam should say "has_icu builds: yes" otherwise check your icu installation above
 ./bootstrap.sh
 sudo ./bjam --with-date_time --with-filesystem --with-regex --with-system --with-thread install

fi

#==== 2. Building and Installing Exodus ====

 sudo yum -y install subversion gcc-c++ postgresql-devel

 cd ~
 svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
 cd ~/exodus
 ./configure
 make && sudo make install


#==== 3. Configuring Postgresql ====

 sudo yum -y install postgresql-server
 sudo service postgresql initdb
 sudo service postgresql start
 sudo chkconfig postgresql on

 sudo su - postgres

 ##allow local tcp/ip login
 #change "ident sameuser" to "md5" for all/all/127.0.0.1/32
 #host    all         all         127.0.0.1/32          ident sameuser
 #host    all         all         127.0.0.1/32          md5
 cp ~/data/pg_hba.conf ~/data/pg_hba.conf.preexodus
 egrep -v "^\w*host.*all.*127.0.0.1/.*ident" ~/data/pg_hba.conf.preexodus > ~/data/pg_hba.conf
 echo host all all 127.0.0.1/32 md5 >> ~/data/pg_hba.conf

 psql -U postgres -d template1
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
 exit

 sudo service postgresql reload

#==== 4. Configuring Exodus ====

 echo host=127.0.0.1 \
 port=5432 \
 dbname=exodus \
 user=exodus \
 password=somesillysecret \
 > ~/.exodus

#==== 5. Testing Exodus ====

 cd ~
 testsort

==== 6. Programming with Exodus ====

 #you must make some change to hello or actually save it, not just quit
 #edic hello
 #hello
 #compile hello

 #find /usr|grep exodus.*examples
