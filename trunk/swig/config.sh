#!/bin/bash
set -e

export SWIG_TARGET=$1

export SWIG_ALL_TARGETS="perl php python java csharp"

export EXO_EXODUS_INCLUDE_FLAGS="-I../../exodus/exodus"
export EXO_WRAPPER_FLAGS="-fPIC -fno-strict-aliasing -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes"
export EXO_EXODUS_LDFLAGS="-lexodus"

#defaults
export SWIG_WRAPPER_EXT=cxx
export SWIG_MODULENAME="exodus"

export SWIG_LOCAL_LIBDIR=/usr/local/lib
export SWIG_OPTIONS="-w503,314,389,361,362,370,383,384"

#php    exodus.so in php extension dir
#python _exodus.si in local lib
#java   libexodus.so in local lib? but this conficts with main exodus library file name
#perl   exodus.so in local lib (also exodus.pm in usr lib perl5

#something like python2.6
export SWIG_PYTHON_LIBCODE="`python --version 2>&1|cut -d'.' -f 1,2|sed -e 's/ //;y/P/p/'`"

#try to use user installed swig otherwise system swig which is 1.3 on Ubuntu 10.04 for example
export SWIG_HOME="$HOME/local/bin/"
test -f ${SWIG_HOME}swig || export SWIG_HOME=""
export SWIG_CMD=${SWIG_HOME}swig

#----------------
#--- "Target" ---
#----------------
case $SWIG_TARGET in

   all )

;; php )
	export SWIG_MODULENAME="exo"

        export SWIG_TARGET_INCLUDE_FLAGS="`php-config --includes`"
        export SWIG_WRAPPER_EXT=cpp

        export SWIG_TARGET_LIBDIR="`php-config --extension-dir`"
	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"

	if [ "`$SWIG_CMD -version |grep 1.3`" != "" ]; then
		#export SWIG_POSTGENERATE_CMD="patch $SWIG_MODULENAME.php ../$SWIG_MODULENAME.php.patch"
		export SWIG_POSTGENERATE_CMD="sed -i -e 's/\$this->_cPtr=\$r;/\/\/EXODUSPATCH \$this-\>_cPtr=\$r;/' $SWIG_MODULENAME.php"
	fi

	#centos in /etc/php.ini, ubuntu in /etc/php5 ...
	export PHPINIFILES="/etc/php.ini /etc/php5/cli/php.ini /etc/php5/conf.d/php.ini"
	export SWIG_MODULE_INSTALL="for FILE in $PHPINIFILES; do test -f \$FILE && [ ! \`grep extension=exo.so \$FILE\` ] && echo \"extension=exo.so\"  && echo \"extension=exo.so\" >> \$FILE; done"

;; python )

#	export SWIG_TARGET_INCLUDE_FLAGS="`python-config --includes`"
#	export SWIG_TARGET_LDFLAGS="-l$SWIG_PYTHON_LIBCODE"
#
#	export SWIG_TARGET_LIBFILE="_$SWIG_MODULENAME.so"
#	export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

#	export SWIG_TARGET_MODFILE="$SWIG_MODULENAME.py*"
#	export SWIG_TARGET_MODDIR="$SWIG_LOCAL_LIBDIR/$SWIG_PYTHON_LIBCODE/site-packages"
##	export SWIG_TARGET_MODDIR=`python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"`
##	something like python2.6
#	export SWIG_PYTHON_LIBCODE="`python --version 2>&1|cut -d'.' -f 1,2|sed -e 's/ //;y/P/p/'`"

	export SWIG_MODULE_COMPILE="python ../setup.py build"
	export SWIG_MODULE_INSTALL="python ../setup.py install"

;; perl )
	export SWIG_MODULENAME="exo"

	export SWIG_TARGET_LIBFILE="$SWIG_MODULENAME.so"
	export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

	export SWIG_TARGET_INCLUDE_FLAGS="`perl -MConfig -e 'print join(\" \", @Config{qw(ccflags optimize cccdlflags)}, \"-I$Config{archlib}/CORE\")'`"
	export SWIG_TARGET_LDFLAGS="`perl -MConfig -e 'print $Config{lddlflags}'`"

        export SWIG_TARGET_MODDIR="/usr/lib/perl5"
	test -d /usr/lib/perl5/site_perl && export SWIG_TARGET_MODDIR="/usr/lib/perl5/site_perl"
        export SWIG_TARGET_MODFILE="$SWIG_MODULENAME.pm"

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
        export SWIG_TARGET_LIBDIR=$SWIG_JAVA_LIBDIR

	export SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java-6-openjdk/include -I/usr/lib/jvm/java-6-openjdk/include/linux"
	#centos 5
	test -d /usr/lib/jvm/java/ && SWIG_TARGET_INCLUDE_FLAGS="-I/usr/lib/jvm/java/include -I/usr/lib/jvm/java/include/linux"

        export SWIG_TARGET_LIBFILE="lib$SWIG_MODULENAME.so"

	export SWIG_POSTGENERATE_CMD="javac $SWIG_PACKAGE_SUBDIR/*.java"
	export SWIG_MODULE_BUILD="jar cvf $SWIG_PACKAGENAME.jar $SWIG_PACKAGE_SUBDIR"

	#nb dont copy to local lib otherwise main libexodus.so will be lost

;; csharp )
	export SWIG_OPTIONS="$SWIG_OPTIONS -dllimport ${SWIG_MODULENAME}_wrapper"
        export SWIG_TARGET_INCLUDE_FLAGS=""
        export SWIG_TARGET_LIBFILE="lib${SWIG_MODULENAME}_wrapper.so"
        export SWIG_TARGET_LIBDIR=$SWIG_LOCAL_LIBDIR

	export SWIG_PATCH_CMD="sed -i -e 's/public string ToString/public override string ToString/' mvar.cs"

	export SWIG_MODULENAME="${SWIG_MODULENAME}_library"
	export SWIG_POSTGENERATE_CMD="gmcs $SWIG_MODULENAME.cs *.cs ../AssemblyInfo.cs -target:library -keyfile:../${SWIG_MODULENAME}.snk"

	#export SWIG_MODULE_INSTALL="sn -R ${SWIG_MODULENAME}.dll ../exodus_library.snk ; gacutil -i ${SWIG_MODULENAME}.dll"
	export SWIG_MODULE_INSTALL="cp ${SWIG_MODULENAME}.dll $SWIG_LOCAL_LIBDIR"

;;*)
        echo "$SWIG_SYNTAX all or $SWIG_ALL_TARGETS"
        exit 1
;;
esac


