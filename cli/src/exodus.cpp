/*
Copyright (c) 2009 Stephen John Bush

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <exodus/exodus.h>

program()
{
        printl("Exodus Copyright (c) 2009 Stephen Bush");
        printl("http://www.opensource.org/licenses/mit-license.php");
		printl("Using Library Version: ", var().version());

        var verbose=OPTIONS.index("V");

		if (verbose)
			EXECPATH.outputl("Executable:");

        var exodusbinpath=field(EXECPATH,SLASH,1,dcount(EXECPATH,SLASH)-1);

		if (verbose)
			exodusbinpath.outputl("Path:");

        //if (not var().load("libpq.dll"))
        //	printl("Warning: Cannot find libpq.dll to connect to postgres");

        var command=SENTENCE.field(" ",2,999999);
        var shell;

		//non-windows
        if (SLASH eq "/" and shell.osgetenv("SHELL")) {

                var home=osgetenv("HOME");
                var path=osgetenv("PATH");
                var libp=osgetenv("LD_LIBRARY_PATH");

                //prefer user binaries then exodus binaries before all else
                ossetenv("PATH",home^"/bin:/var/lib/exodus/bin:"^path);

//print("LD_LIBRARY_PATH","~/lib:"^osgetenv("LD_LIBRARY_PATH"));
                ossetenv("LD_LIBRARY_PATH",home^"/lib:"^libp);


                //enable core dumps
                osshell("ulimit -c unlimited");

                if (verbose) {
                        osgetenv("HOME").outputl("HOME=");
                        osgetenv("PATH").outputl("PATH=");
                        osgetenv("LD_LIBRARY_PATH").outputl("LD_LIBRARY_PATH=");
                }

                //execute command or enter exodus shell
                //if (not osshell("env PS1='exodus [\\u@\\h \\W]\\$ '  "^(command?command:shell)))

                osshell(command?command:shell);

		//windows
        } else if (SLASH eq "\\" and shell.osgetenv("ComSpec")) {

                //set EXODUS_PATH used by compile to find LIB and INCLUDE paths
                var exoduspath=exodusbinpath;
                if (exoduspath.field2(SLASH,-1)=="bin")
                        exoduspath=field(exoduspath,SLASH,1,dcount(exoduspath,SLASH)-1);
                if (exoduspath and not ossetenv("EXODUS_PATH",exoduspath))
                        errput("Couldnt set EXODUS_PATH environment variable");

				var currpath=osgetenv("PATH");

                //PREFIX exodus bin to path
				var newpath=exodusbinpath^";"^currpath;

				//APPEND user's Exodus binaries path (from compile/catalog)
				var homedir=osgetenv("USERPROFILE");
				if (homedir)
					newpath^=";"^homedir^"\\Application Data\\Exodus";

				//update path
                if (not ossetenv("PATH",newpath))
                        errput("Couldnt set PATH environment variable");

                //set INCLUDE path
                //prefix path to exodus.h to INCLUDE environment variable
                //1. ..\include folder parallel to path of executable
                //2. build source folder of exodus
                //3. EXODUS_INCLUDE envvar
                var exodusincludepath=exodusbinpath^"\\..\\include";
				var searched=exodusincludepath;
				if (!osdir(exodusincludepath)) {
					exodusincludepath=exodusbinpath^"\\..\\exodus\\exodus";
					searched.replacer(-1,exodusincludepath);
				}
                if (!osfile(exodusincludepath^"\\exodus\\exodus.h")) {
                        exodusincludepath=osgetenv("EXODUS_INCLUDE");
						searched.replacer(-1,exodusincludepath);
                        if (!osfile(exodusincludepath^"\\exodus.h")) {
                                errputl("Couldnt find exodus include path (exodus.h)");
								errputl(searched.swap(FM,"\n"));
                                exodusincludepath="";
                        }
                }
                if (not ossetenv("EXODUS_INCLUDE",exodusincludepath))
                        errput("Couldnt set EXODUS_INCLUDE environment variable");
                if (exodusincludepath and not ossetenv("INCLUDE",(exodusincludepath^";"^osgetenv("INCLUDE"))))
                        errput("Couldnt set INCLUDE environment variable");

                //set LIB path
                //prefix path to exodus.lib to LIB environment variable
                //1. path of executable
                //2. ..\lib folder parallel to path of executable
                //3. EXODUS_LIB envvar
                var exoduslibpath=exodusbinpath;
                if (!osfile(exoduslibpath^"\\exodus.lib"))
                        exoduslibpath=exodusbinpath^"\\..\\lib";
                if (!osfile(exoduslibpath^"\\exodus.lib"))
                        exoduslibpath=osgetenv("EXODUS_LIB");
                if (!osfile(exoduslibpath^"\\exodus.lib")) {
                        exoduslibpath=osgetenv("EXODUS_LIB");
                        if (!osfile(exoduslibpath^"\\exodus.lib")) {
                                errput("Couldnt find exodus include path (exodus.h)");
                                exoduslibpath="";
                        }
                }
                if (not ossetenv("EXODUS_LIB",exoduslibpath))
                        errput("Couldnt set EXODUS_LIB environment variable");
                if (exoduslibpath and not ossetenv("LIB",(exoduslibpath^";"^osgetenv("LIB"))))
                        errput("Couldnt set LIB environment variable");

                osshell((command?command:shell));
        } else
                stop("Cannot find SHELL or ComSpec in environment");

}
