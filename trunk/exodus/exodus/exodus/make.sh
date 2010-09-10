#!/bin/bash

echo make or make debug or make all

clear

echo Setup parameters

#optimisation O1 gives ~50% increase in speed, other give very little more
#http://www.network-theory.co.uk/docs/gccintro/gccintro_49.html

if [ "$1" == "debug" ] ; then
 #compileoptions="-g -Weffc++ -pedantic"
 compileoptions="-g"
 libfileextension="-gd"
else
 compileoptions="-DNDEBUG -O1"
 libfileextension=""
fi

compilefiles="*.cpp *.c"
#compilefiles="mvfuncs.cpp *.c"
#compilefiles="mvglobalfuncs.cpp"
#compilefiles="mvdbpostgres.cpp"
#compilefiles="mvfuncs.cpp mvioconv.cpp"
#compilefiles="mv.cpp mvioconv.cpp mvdatetime.cpp"
#compilefiles="mvdebug.cpp"

libfilebasename=exodus
libfileversion=1
libfilerelease=0
targetincludedir=/usr/include
targetlibdir=/usr/lib
boostlibs="-lboost_filesystem-gcc41-mt -lboost_regex-gcc41-mt -lboost_thread-gcc41-mt"

echo
echo Setup filenames
libfilename=lib$libfilebasename$libfileextension.so
versionfilename=$libfilename.$libfileversion
releasefilename=$versionfilename.$libfilerelease

echo
echo libfilebasename: $libfilebasename$libfileextension
echo libfilename....:     $libfilename
echo versionfilename: $versionfilename
echo releasefilename: $releasefilename

echo
echo Copy include headers to include
echo sudo cp mv*.h $targetincludedir
echo sudo cp exodus*.h $targetincludedir
sudo cp mv*.h $targetincludedir
sudo cp exodus*.h $targetincludedir
if [[ $? -ne 0 ]]; then echo "FAILED"; exit; fi

echo
echo Get rid of various superfluous files
rm mvdbqm.cpp
rm *.0

echo
echo Compile C++/C
echo g++ -fPIC -c $compilefiles $compileoptions -pass-exit-codes
g++ -fPIC -c $compilefiles $compileoptions -fvisibility=hidden -pass-exit-codes | grep -v "tss.hpp:42:"
#echo $CMD
#`$CMD`
echo $? -------------------------------
if [[ $? -gt 1 ]]; then echo "FAILED"; exit; fi

echo
echo Get rid of superfluous binaries
rm test*.o

echo
echo Link everything up
echo g++ -shared -o $releasefilename *.o -lc -lpq $boostlibs -Wl,-soname,$versionfilename
g++ -shared -o $releasefilename *.o -lc -lpq $boostlibs -Wl,-soname,$versionfilename
if [[ $? -ne 0 ]]; then echo "FAILED"; exit; fi

ld $releasefilename

echo
echo Copy to library folder
echo sudo cp $releasefilename $targetlibdir
sudo cp $releasefilename $targetlibdir
if [[ $? -ne 0 ]]; then echo "FAILED"; exit; fi

echo
echo Register all libs into linux binary loading system - this also creates a symlink file for the soname pointing to the library file
sudo ldconfig -v|grep exodus
if [[ $? -ne 0 ]]; then echo "FAILED"; exit; fi

echo
echo Create a pointer for the current version
echo sudo ln -sf $targetlibdir/$versionfilename $targetlibdir/$libfilename
sudo ln -sf $targetlibdir/$versionfilename $targetlibdir/$libfilename
if [[ $? -ne 0 ]]; then echo "FAILED"; exit; fi

#to build debug libraries
if [ "$1" == "all" ] ; then
 ./make debug
fi

