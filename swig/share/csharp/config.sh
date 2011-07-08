#!/bin/bash
set -ex

export SWIG_TARGET=$1

export SWIG_ALL_TARGETS="perl python java csharp php" put php last because it exits somehow

#[module - (not .so/dll) platform INDEPENDENT .php .py .pm .jar .dll] (yes PLATFORM INDEPENDENT "DLL"!)
#  -> per-language exodus wrapper.so/dll
#    -> standard exodus.so/dll
#       -> boost libs
#         -> icu libs
#         -> postgres libs

#per lang exodus module names and locations

# php    = exo.php             dumped in /usr/lib
# python = exodus.py/pyc       only setup.py knows
# perl   = exo.pm              eg /etc/perl
# java   = jexodus.jar         dumped in /usr/lib
# csharp = exodus_library.so   dumped in /usr/lib

#per lang exodus wrapper names and locations

# php    = exo.so              php package dir eg /usr/lib64/php/modules/
# python = _exodus.so          only setup.py knows
# perl   = exo.so              /usr/lib
# java   = jexodus.so          dumped in /usr/lib
# csharp = exodus_wrapper.so   dumped in /usr/lib

export EXO_EXODUS_INCLUDE_FLAGS="-I../../../../exodus/libexodus -I/usr/include"
export EXO_WRAPPER_FLAGS="-fPIC -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes"
export EXO_EXODUS_LDFLAGS="-lexodus"

#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULENAME="exodus"

export SWIG_OPTIONS="-w503,314,389,361,362,370,383,384"

export EXO_EXODUS_LINK="NOTSTATIC"
#cant link static on debian6 due to missing reference to icudt44_dat
#Cant load /usr/local/lib/exo.so for module exo: /usr/local/lib/exo.so: undefined symbol: icudt44_dat
#export EXO_EXODUS_LINK="STATIC"
#therefore NONE are static if EXODUS isnt
#if exodus is static then try to link boost and icu statically
export EXO_BOOST_LINK="STATIC"
export EXO_ICU_LINK="STATIC"
if [ "$EXO_EXODUS_LINK" == "STATIC" ]; then

 #can only link to exodus statically if compiled with -fPIC
 export EXO_EXODUS_LINKTYPE="-Wl,-Bstatic"

 export EXO_BOOST_LIBS=" -L/usr/lib64 -Wl,-lboost_date_time -Wl,-lboost_filesystem -Wl,-lboost_regex -Wl,-lboost_system -Wl,-lboost_thread"
 export EXO_POSTGRES_LIBS=" -Wl,-lpq"
 export EXO_ICU_LIBS=""

 if [ "$EXO_BOOST_LINK" == "STATIC" ]; then
  export EXO_BOOST_LINKTYPE="-Wl,-Bstatic"
 else
  export EXO_BOOST_LINKTYPE="-Wl,-Bdynamic"
 fi

 export EXO_ICU_LIBS=" -Wl,-licudata -Wl,-licui18n -Wl,-licuuc"
 #export EXO_ICU_LIBS=" -Wl,-licudata -Wl,-licui18n -Wl,-licuuc -Wl,-licuio -Wl,-licule -Wl,-liculx -Wl,-licutu"
 if [ "$EXO_ICU_LINK" == "STATIC" ]; then
  export EXO_ICU_LINKTYPE="-Wl,-Bstatic"
 else
  export EXO_ICU_LINKTYPE="-Wl,-Bdynamic"
 fi

 if [ "$EXO_POSTGRES_LINK" == "STATIC" ]; then
  export EXO_POSTGRES_LINKTYPE="-Wl,-Bstatic"
 else
  export EXO_POSTGRES_LINKTYPE="-Wl,-Bdynamic"
 fi

fi

export EXO_EXODUS_LDFLAGS=" \
 $EXO_EXODUS_LINKTYPE -Wl,-lexodus \
 $EXO_BOOST_LINKTYPE $EXO_BOOST_LIBS \
 $EXO_ICU_LINKTYPE $EXO_ICU_LIBS \
 $EXO_POSTGRES_LINKTYPE $EXO_POSTGRES_LIBS \
 "

#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULENAME="exodus"

if [ "$FAKEROOTKEY$DESTDIR$RPM_BUILD_ROOT" == "" ]; then
 export SWIG_SHARED_LIBDIR=/usr/local/lib
 export SWIG_SHARE_DIR=/usr/local/share
else
 export SWIG_SHARED_LIBDIR=/usr/lib
 export SWIG_SHARE_DIR=/usr/share
fi
if [ -d ${SWIG_SHARED_LIBDIR}64 ] && [ ! -L ${SWIG_SHARED_LIBDIR}64 ]; then export SWIG_SHARED_LIBDIR=${SWIG_SHARED_LIBDIR}64; fi

export SWIG_OPTIONS="-w503,314,389,361,362,370,383,384"

#something like python2.6
export SWIG_PYTHON_LIBCODE="`python --version 2>&1|cut -d'.' -f 1,2|sed -e 's/ //;y/P/p/'`"

#try to use user installed swig otherwise system swig which is 1.3 on Ubuntu 10.04 for example
export SWIG_HOME="$HOME/local/bin/"
test -f ${SWIG_HOME}swig || export SWIG_HOME=""
export SWIG_CMD=${SWIG_HOME}swig

#---------------
#--- DESTDIR ---
#---------------
#export SWIG_DESTDIR="/"
export SWIG_DESTDIR=$DESTDIR

if [ "$SWIG_DESTDIR" == "" ]; then export SWIG_DESTDIR="/"; fi
if [ "$FAKEROOTKEY" != "" ]; then
	export SWIG_DESTDIR="../debian/libexodus-$SWIG_TARGET"
	#avoid error ... dh_usrlocal: debian/libexodus-python/usr/local/lib/python2.6/dist-packages/_exodus.so is not a directory"
	#but causes installation into "sites" instead of "distro" which isnt on python path
	#export PREFIX="/usr"
fi

#--------------
#--- DOCDIR ---
#--------------
if [ "$PREFIX" == "" ]; then export PREFIX="/usr"; fi
export SWIG_DOCDIR=$PREFIX/share/doc
test -d $SWIG_DOCDIR/packages && export SWIG_DOCDIR=$SWIG_DOCDIR/packages
echo $SWIG_DOCDIR

#----------------
#--- "Target" ---
#----------------
case $SWIG_TARGET in

   all )

;; php )
	export SWIG_MODULENAME="exo"

        export SWIG_TARGET_INCLUDE_FLAGS="`php-config --includes`"
        export SWIG_WRAPPER_EXT=cpp

	#dump exo.php in lib dir from where it can be copied to php web directories for including
        #export SWIG_TARGET_MODDIR=$SWIG_SHARED_LIBDIR
        export SWIG_TARGET_MODDIR=$SWIG_SHARE_DIR/php
        export SWIG_TARGET_MODFILE=$SWIG_MODULENAME.php

        export SWIG_TARGET_LIBDIR="`php-config --extension-dir`"
	if [ "$SWIG_TARGET_LIBDIR" == "" ]; then
		test -d /usr/lib/php/modules/ && export SWIG_TARGET_LIBDIR="/usr/lib/php/modules/"
		test -d /usr/lib64/php/modules/ && export SWIG_TARGET_LIBDIR="/usr/lib64/php/modules/"
	fi
	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"

	if [ "`$SWIG_CMD -version |grep 1.3`" != "" ]; then
		#export SWIG_POSTGENERATE_CMD="patch $SWIG_MODULENAME.php ../$SWIG_MODULENAME.php.patch"
		export SWIG_POSTGENERATE_CMD="sed -i -e 's/\$this->_cPtr=\$r;/\/\/EXODUSPATCH \$this-\>_cPtr=\$r;/' $SWIG_MODULENAME.php"
	fi

	#centos in /etc/php.ini, ubuntu in /etc/php5 ...
	export PHPINIFILES="/etc/php.ini /etc/php5/cli/php.ini /etc/php5/conf.d/php.ini"
	#export SWIG_MODULE_INSTALL="for FILE in $PHPINIFILES; do test -f \$FILE && [ ! \`grep extension=exo.so \$FILE\` ] && echo \"extension=exo.so\"  && echo \"extension=exo.so\" >> \$FILE; done"
	if [ "$FAKEROOTKEY$DESTDIR" == "" ]; then
		export SWIG_MODULE_INSTALL="for FILE in $PHPINIFILES; do \
					if [[ -f \$FILE && \"\`grep extension=exo.so \$FILE\`\" == \"\" ]]; then\
						echo \"extension=exo.so\"  && \
						echo \"extension=exo.so\" >> \$FILE; \
					fi; \
				    done"
	fi

;; python )

#	export SWIG_TARGET_INCLUDE_FLAGS="`python-config --includes`"
#	export SWIG_TARGET_LDFLAGS="-l$SWIG_PYTHON_LIBCODE"
#
#	export SWIG_TARGET_LIBFILE="_$SWIG_MODULENAME.so"
#	export SWIG_TARGET_LIBDIR=$SWIG_SHARED_LIBDIR

#	export SWIG_TARGET_MODFILE="$SWIG_MODULENAME.py*"
#	export SWIG_TARGET_MODDIR="$SWIG_SHARED_LIBDIR/$SWIG_PYTHON_LIBCODE/site-packages"
##	export SWIG_TARGET_MODDIR=`python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"`
##	something like python2.6
#	export SWIG_PYTHON_LIBCODE="`python --version 2>&1|cut -d'.' -f 1,2|sed -e 's/ //;y/P/p/'`"

	export SWIG_MODULE_COMPILE="python ../setup.py build "
	export SWIG_MODULE_INSTALL="python ../setup.py install --root=$DESTDIR --prefix=$PREFIX"
	#export SWIG_MODULE_INSTALL="python ../setup.py install --root=$DESTDIR --install-layout=deb"
	#export SWIG_MODULE_INSTALL="python ../setup.py install --root=$DESTDIR"
	if test -d /etc/apt/apt.conf.d; then
		export SWIG_MODULE_INSTALL="$SWIG_MODULE_INSTALL --install-layout=deb"
	fi
;; perl )
	export SWIG_MODULENAME="exo"

	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"
	export SWIG_TARGET_LIBDIR=$SWIG_SHARED_LIBDIR

	export SWIG_TARGET_INCLUDE_FLAGS="`perl -MConfig -e 'print join(\" \", @Config{qw(ccflags optimize cccdlflags)}, \"-I$Config{archlib}/CORE\")'`"
	export SWIG_TARGET_LDFLAGS="`perl -MConfig -e 'print $Config{lddlflags}'`"

        export SWIG_TARGET_MODDIR=`perl -e 'print @INC[0]'`
	#test -d /usr/lib/perl5/site_perl && export SWIG_TARGET_MODDIR="/usr/lib/perl5/site_perl"
	#test -d /usr/lib64/perl5/site_perl && export SWIG_TARGET_MODDIR="/usr/lib64/perl5/site_perl"
        export SWIG_TARGET_MODFILE=$SWIG_MODULENAME.pm

;; java )

	#module name determines name of the SO/DLL and loadlibrary statement
	#package name determines the name of the jar and import statement

	#libjexodus.so and jexodus.jar import jexodus.*;
	#import jexodus.*;
        #System.loadLibrary("jexodus");
	#export SWIG_PACKAGENAME=org.$SWIG_MODULENAME
	#export SWIG_PACKAGE_SUBDIR=org/$SWIG_MODULENAME
	export SWIG_PACKAGENAME="j$SWIG_MODULENAME"
	export SWIG_PACKAGE_SUBDIR="j$SWIG_MODULENAME"
	export SWIG_MODULENAME="j$SWIG_MODULENAME"
	export SWIG_OPTIONS="$SWIG_OPTIONS -package $SWIG_PACKAGENAME -outdir $SWIG_PACKAGE_SUBDIR"
	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"
        export SWIG_TARGET_LIBDIR=$SWIG_SHARED_LIBDIR

	export SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java-6-openjdk/include -I/usr/lib/jvm/java-6-openjdk/include/linux"
	#centos 5
	test -d /usr/lib/jvm/java/ && SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java/include -I/usr/lib/jvm/java/include/linux"

        export SWIG_TARGET_LIBFILE="lib$SWIG_MODULENAME.so"

	export SWIG_POSTGENERATE_CMD="javac $SWIG_PACKAGE_SUBDIR/*.java"
	export SWIG_MODULE_BUILD="jar cvf $SWIG_PACKAGENAME.jar $SWIG_PACKAGE_SUBDIR"
	#export SWIG_MODULE_INSTALL="cp -f ${SWIG_MODULENAME}.jar $SWIG_SHARED_LIBDIR"
	export SWIG_TARGET_MODFILE=${SWIG_MODULENAME}.jar
	export SWIG_TARGET_MODDIR=${SWIG_SHARE_DIR}/java

;; csharp )
	export SWIG_OPTIONS="$SWIG_OPTIONS -dllimport ${SWIG_MODULENAME}_wrapper"
        export SWIG_TARGET_INCLUDE_FLAGS=""
        export SWIG_TARGET_LIBFILE="lib${SWIG_MODULENAME}_wrapper.so"
        export SWIG_TARGET_LIBDIR=$SWIG_SHARED_LIBDIR

	export SWIG_PATCH_CMD="sed -i -e 's/public string ToString/public override string ToString/' mvar.cs"

	export SWIG_MODULENAME="${SWIG_MODULENAME}_library"
	export SWIG_POSTGENERATE_CMD="gmcs $SWIG_MODULENAME.cs *.cs ../AssemblyInfo.cs -target:library -keyfile:../${SWIG_MODULENAME}.snk"

	#export SWIG_MODULE_INSTALL="sn -R ${SWIG_MODULENAME}.dll ../exodus_library.snk ; gacutil -i ${SWIG_MODULENAME}.dll"
	#export SWIG_MODULE_INSTALL="cp -f ${SWIG_MODULENAME}.dll ${SWIG_DESTDIR}${SWIG_SHARE_DIR}/"
	export SWIG_TARGET_MODFILE=${SWIG_MODULENAME}.dll
	export SWIG_TARGET_MODDIR=${SWIG_SHARE_DIR}/csharp

;; *)
        echo "$SWIG_SYNTAX all or $SWIG_ALL_TARGETS"
        exit 1
;;
esac


env
