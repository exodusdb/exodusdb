#/!bash

echo
echo only needs to be run if any new pgexodus libs are available
echo for adding into pgexodus distribution pkglib

#search for any installed any pgexodus libs we can find ans save them in pgexodus/lib
#per pg version, os and os architecture.
#The bitrock installer etc will install ALL libs and
#then selectivedly install the right ones into the target postgres pkglib directory(ies)

SCRIPT_DIR="$( cd "$( dirname "$0" )" && pwd )"
BASEDIR=${SCRIPT_DIR}/pkglib

#OS=`uname -s`
#following isnt reliable Mac OSX Snow Leopard has 32 bit kernel but runs 64 bit apps!
#ARCH=`uname -m`

#postgresql might be installed 32 bit on 64 bit operating system
#TODO check actual architecture using file or select serial();

#use file pg_config to see what architectures are supported in the binary
#combined with environment variable HOSTTYPE you should be able to work
#out what version is in use but select serial(); in postgres tell you for sure
#"PostgreSQL 9.0.1 on x86_64-apple-darwin, compiled by GCC i686-apple-darwin10-gcc-4.2.1 (GCC) 4.2.1 (Apple Inc. build 5664), 64-bit"

#i386 i586 i686 are all 32 bit

#might be easier to install pgexodus by autotools since it doesnt require boost!

#Use MACHTYPE to collect up binaries although each Linux appears differently
#not using OSTYPE and HOSTTYPE because omits Linux Distro which MIGHT be important
#This doesnt reflect different binaries required by different Versions of each OS!
#Examples
#Os     Ver    Bits HOSTTYPE OSTYPE     MACHTYPE
#Ubuntu 10.04  64   x86_64   linux-gnu  x86_64-pc-linux-gnu
#Centos 5.5    32   i686     linux-gnu  i686-redhat-linux-gnu
#Ubuntu 10.04  32   i486     linux-gnu  i486-pc-linux-gnu
#OSX    10.6.4 64   x86_64   darwin10.0 x86_64-apple-darwin10.0

#multiple cpu/architecture binary from enterprisedb.com on macosx
#$> file /Library/PostgreSQL/9.0/bin/pg_config
#/Library/PostgreSQL/9.0/bin/pg_config: Mach-O universal binary with 3 architectures
#/Library/PostgreSQL/9.0/bin/pg_config (for architecture ppc7400):       Mach-O executable ppc
#/Library/PostgreSQL/9.0/bin/pg_config (for architecture i386):  Mach-O executable i386
#/Library/PostgreSQL/9.0/bin/pg_config (for architecture x86_64):        Mach-O 64-bit executable x86_64

function savelibs()
{

if [ -d "$FROMDIR" ]
then
#SAVEDIR=$BASEDIR/pgexodus-$PGMAJOR-${OS}-${ARCH}/
SAVEDIR=$BASEDIR/pgexodus-$PGMAJOR-${MACHTYPE}/

if [ ! -d "$SAVEDIR" ]
then
if [ ! -d "$BASEDIR" ]
then
mkdir "$BASEDIR"
fi
mkdir "$SAVEDIR"
fi

echo cp -a ${FROMDIR}/pgexodus* $SAVEDIR
cp -a ${FROMDIR}/pgexodus* $SAVEDIR
ls -l $SAVEDIR
fi

if [ -d  "$BASEDIR/../.svn" ]
then
svn --force add "$BASEDIR"
svn --force add "$BASEDIR"/*
svn --force add "$SAVEDIR"/*

fi

}


for SEARCHDIR in /Library /opt
do

echo
echo Searching $SEARCHDIR for pg_config

for PGCONFIG in `find $SEARCHDIR -type f -name pg_config 2>/dev/null`
do

FROMDIR=`$PGCONFIG --pkglibdir`
PGMAJOR=`$PGCONFIG --version|cut -f 2 -d " "|cut -f 1 -d "."`
echo
echo Found version $PGMAJOR using $PGCONFIG

savelibs

done

done
