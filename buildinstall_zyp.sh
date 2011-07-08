#!/bin/bash
#=== Building and Installing Exodus Using zypper ===

#you can view/run the latest version of this script directly
# wget -O - -q http://exodusdb.googlecode.com/svn/trunk/buildinstall_zyp.sh | less
# wget -O - -q http://exodusdb.googlecode.com/svn/trunk/buildinstall_zyp.sh | bash
#or
# curl http://exodusdb.googlecode.com/svn/trunk/buildinstall_zyp.sh | less
# curl http://exodusdb.googlecode.com/svn/trunk/buildinstall_zyp.sh | bash

#it takes about 5 minutes

#tested on:

#openSUSE 11.4 "Celadon"
#Linux linux-ehmx 2.6.37.6-0.5-desktop #1 SMP PREEMPT 2011-04-25 21:48:33 +0200 x86_64 x86_64 x86_64 GNU/Linux

echo building on:
 cat /etc/issue|head -n 1
 uname -a

export yum="yum -y"
export yum=zypper --non-interactive

#==== 1. Installing or Building Boost ====

sudo $yum install icu libicu libicu-devel

#rather dumb version centos/redhat 5 detection
if [ "`cat /etc/issue|grep " 5"`" == "" ]; then

 sudo $yum install boost-devel boost-date-time boost-filesystem boost-regex boost-system boost-thread

else

 #IT IS CRITICAL THAT BOOST-DEVEL IS *NOT* INSTALLED if building boost as follows.
 sudo $yum remove boost-devel

 sudo $yum install gcc-c++

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

 sudo $yum install subversion gcc-c++ postgresql-devel make

 cd ~
 svn co HTTPS://exodusdb.googlecode.com/svn/trunk/ exodus
 cd ~/exodus
 ./configure
 make && sudo make install


#==== 3. Configuring Postgresql ====

 sudo $yum install postgresql-server

 test -f /usr/local/bin/installexodus-postgresql && sudo /usr/local/bin/installexodus-postgresql
 test -f /usr/bin/installexodus-postgresql && sudo /usr/bin/installexodus-postgresql

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
