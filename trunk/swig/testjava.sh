#!/bin/bash
set -e

#we installed exodus_library.so and exodus_wrapper.so in the general lib
#because dotnet has no easy pcentral place for modules and libraries
#the GAC is highly controlled
export LOCAL_LIBDIR=/usr/local/lib
test -d ${LOCAL_LIBDIR}64 && export LOCAL_LIBDIR=${LOCAL_LIBDIR}64
export EXO_LIB_PATH=$LOCAL_LIBDIR
export EXO_CLASSPATH=$LOCAL_LIBDIR/jexodus.jar

#delete any existing test.exe
test -f test.class && rm -f test.class && echo rm -f test.class

#--------
# Compile
#--------
echo \
javac -cp $EXO_CLASSPATH test.java
javac -cp $EXO_CLASSPATH test.java

#----
# Run
#----
echo \
java -cp $EXO_CLASSPATH:. -Djava.library.path=$EXO_LIB_PATH test
java -cp $EXO_CLASSPATH:. -Djava.library.path=$EXO_LIB_PATH test
