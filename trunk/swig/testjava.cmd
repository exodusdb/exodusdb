@rem set CLASSPATH=.;%EXO_JAVA%\lib
@rem set LD_LIBRARY_PATH=%EXO_JAVA%\lib

rem add path to main exodus.dll (does it matter which way around the 32/64 libs are put)
rem set PATH=%PATH%;..\release;..\x64\release

javac -cp "%EXO_JAVA%\lib\jexodus.jar" test.java

java -cp .;"%EXO_JAVA%\lib\jexodus.jar" -Djava.library.path="%EXO_JAVA%\lib" test
