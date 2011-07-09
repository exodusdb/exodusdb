#!/bin/bash
set -e

#java has no standard place for modules and libraries
#exodus installed jexodus.jar in usr/shared/java
# since it is platform independent
# and libjexodus.so in /usr/lib
export LIBDIR=/usr/lib
test -d ${LIBDIR}64 && export LIBDIR=${LIBDIR}64
if [ -f $LIBDIR/libjexodus.so ]; then
        export MODULEDIR=/usr/share/java
else
        export LIBDIR=/usr/local/lib
        test -d ${LIBDIR}64 && export LIBDIR=${LIBDIR}64
        export MODULEDIR=/usr/local/share/java
fi

#delete any existing test.exe
test -f test.exe && rm -f test.exe && echo rm -f test.exe

echo -------------------------------------------
echo USING MODULE: $MODULEDIR/exodus_library.dll
echo USING LIBRARY: $LIBDIR/libexodus_wrapper.so
echo -------------------------------------------
echo Assuming you have installed default-jdk - for javac
echo --------
echo Compile
echo --------
echo \
javac -cp $MODULEDIR/jexodus.jar test.java
javac -cp $MODULEDIR/jexodus.jar test.java

echo ----
echo Run
echo ----
echo \
java -cp $MODULEDIR/jexodus.jar:. -Djava.library.path=$LIBDIR test
java -cp $MODULEDIR/jexodus.jar:. -Djava.library.path=$LIBDIR test



