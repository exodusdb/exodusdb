#!/bin/bash
set -e

#this isnt perfected yet since it requires manual input.
#requires manual input change exodus version, package version and package signer

export EXO_PACKVER=1
export EXO_EXOVER=11.5.30
export EXO_PKGNAM=exodus
export DEBEMAIL=steve.bush@neosys.com
export DEBFULLNAME="Steve Bush"
export EXO_GNUPG_KEY=2FE45E65
export EXO_DPUT_OPT=ppa:steve-bush/ppa

#open editor on debian/changelog
debchange --newversion ${EXO_EXOVER}-${EXO_PACKVER}

#irc://irc.freenode.net/launchpad

#NB cannot upload revised exodus_11.5.28.orig.tar.gz files (same name with different contents)
#likewise the exodus_11.5.28-1.debian.tar.gz files so the -1 must change to -2 etc every time you change the packaging

#---------------------------
# gnu privacy guard commands
#---------------------------
#gpg --key-gen # this can be slow over an ssh link real console activity is required to generate random
#gpg --list-keys
#sending a key to launchpad (required to get an account)
#gpg --keyserver keyserver.ubuntu.com --send-key 2FE45E65
#gpg -d temp.txt        #decrypting a "-----BEGIN PGP MESSAGE-----" till "-----END PGP MESSAGE-----" message in temp.txt

#------
# Setup
#------
#sudo apt-get install pbuilder pgp debhelper

#--------------
# Using apt-get
#--------------
#using the repository
#sudo sh -c 'echo "deb http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main\ndeb-src http://ppa.launchpad.net/steve-bush/ppa-exodus/ubuntu lucid main" > /etc/apt/sources.list.d/exodus-ppa-lucid.list'
#sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys E7815451
#sudo apt-get update
#sudo apt-cache search exodus


#make clean || echo already clean
#make distclean || echo already distclean
#./configure
#make dist

export EXO_ORIGDIR=`pwd`
export EXO_APTDIR=$HOME/exodusapt

    test -d ${EXO_APTDIR}/${EXO_PKGNAM}-$EXO_EXOVER/debian \
|| mkdir -p ${EXO_APTDIR}/${EXO_PKGNAM}-$EXO_EXOVER/debian

pushd debian
#cp -r [!.svn]* ${EXO_APTDIR}/${EXO_PKGNAM}-$EXO_EXOVER/debian
cp -r . ${EXO_APTDIR}/${EXO_PKGNAM}-$EXO_EXOVER/debian
find ${EXO_APTDIR}/${EXO_PKGNAM}-$EXO_EXOVER/debian | grep "/\.svn" | xargs rm -rf
popd

#nb change from xxxxxxxx-99.99.99 to xxxxxxxx_99.99.99
export EXO_DISTFILENAME=${EXO_PKGNAM}-${EXO_EXOVER}.tar.gz
export EXO_PACKFILENAME=${EXO_PKGNAM}_${EXO_EXOVER}.orig.tar.gz

cp $EXO_DISTFILENAME $EXO_APTDIR/$EXO_PACKFILENAME

cd $EXO_APTDIR

tar xfz $EXO_PACKFILENAME
cd ${EXO_PKGNAM}-$EXO_EXOVER

#following hex code is a gnu privacy guard id - must be in ~/.gnupg
debuild -S -k$EXO_GNUPG_KEY

cd ..

lintian -Ivi *.dsc

echo "dput -f $EXO_DPUT_OPT ${EXO_PKGNAM}_${EXO_EXOVER}-${EXO_PACKVER}_source.changes"
