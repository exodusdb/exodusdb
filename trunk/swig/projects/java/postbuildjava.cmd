@echo on

@echo -----------------------------------------------------------------------------------------
@echo Making platform independent JAR (that calls platform dependent jexodus.dll/libjexodus.so)
@echo -----------------------------------------------------------------------------------------

@echo Invoking JAVAC
pushd "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_SUBDIR%"
"%EXO_JAVA32_BIN%\javac" *.java
popd

@echo Invoking JAR
pushd "%EXO_JAVA32_MODULEDIR%"
"%EXO_JAVA32_BIN%\jar" cvf "%EXO_JAVA32_MODULEDIR%\%EXO_JAVA_PACKAGE_NAME%.jar" "%EXO_JAVA_PACKAGE_SUBDIR%"
popd

@echo off