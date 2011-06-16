@rem set CLASSPATH=.;%EXO_JAVA%\lib
@rem set LD_LIBRARY_PATH=%EXO_JAVA%\lib

javac -cp "%EXO_JAVA%\lib" test.java

java -cp .;"%EXO_JAVA%\lib" -Djava.library.path="%EXO_JAVA%\lib" test