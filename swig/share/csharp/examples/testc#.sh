#!/bin/bash
set -e

#not using GAC so csharp has no standard place for modules and libraries
#exodus installed exodus_library.dll in usr/shared/csharp
# since it is platform independent (despite the dll extension)
# and libexodus_wrapper.so in /usr/lib
export LIBDIR=/usr/lib
test -d ${LIBDIR}64 && export LIBDIR=${LIBDIR}64
if [ -f $LIBDIR/libexodus_wrapper.so ]; then
        export MODULEDIR=/usr/share/csharp
else
        export LIBDIR=/usr/local/lib
        test -d ${LIBDIR}64 && export LIBDIR=${LIBDIR}64
        export MODULEDIR=/usr/local/share/csharp
fi

#delete any existing test.exe
test -f test.exe && rm -f test.exe && echo rm -f test.exe

echo -------------------------------------------
echo USING MODULE: $MODULEDIR/exodus_library.dll
echo USING LIBRARY: $LIBDIR/libexodus_wrapper.so
echo -------------------------------------------
echo
echo ======================================================
echo !!! Assuming you have installed mono-gmcs compiler !!!
echo ubuntu - sudo apt-get install mono-gmcs
echo ======================================================
echo --------
echo Compile
echo --------
echo \
gmcs test.cs -r:$MODULEDIR/exodus_library
gmcs test.cs -r:$MODULEDIR/exodus_library

echo ----
echo Run
echo ----
#mono xxx command not necessary in later versions can execute .exe from command line
echo \
env MONO_PATH=$MODULEDIR env LD_LIBRARY_PATH=$LIBDIR mono ./test.exe
env MONO_PATH=$MODULEDIR env LD_LIBRARY_PATH=$LIBDIR mono ./test.exe
