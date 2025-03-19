/*
Copyright (c) 2009 steve.bush@neosys.com

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

#include <exodus/program.h>
programinit()

func main() {

	printl("Exodus Copyright (c) 2009 steve.bush@neosys.com");
	printl("http://www.opensource.org/licenses/mit-license.php");
	printl("Using Library Version:", var().version());
	printl("Build:", _OS_NAME, _OS_VERSION, _PLATFORM, _COMPILER, _COMPILER_VERSION, "c++", _CPP_STANDARD);

	let verbose = OPTIONS.contains("V");

	if (verbose)
		EXECPATH.outputl("Executable:");

	let exodusbinpath = field(EXECPATH, OSSLASH, 1, fcount(EXECPATH, OSSLASH) - 1);

	if (verbose)
		exodusbinpath.outputl("Path:");

	//if (not var().load("libpq.dll"))
	//	printl("Warning: Cannot find libpq.dll to connect to postgres");

	let command = COMMAND.field(" ", 2, 999999);

	var shell = osgetenv("SHELL");
	if (not(OSSLASH_IS_BACKSLASH) and shell) {

		// Posix

		let home = osgetenv("HOME");
		let path = osgetenv("PATH");
		let libp = osgetenv("LD_LIBRARY_PATH");

		//ossetenv doesnt work in exodus due to problems with putenv/setenv memory loss/leak
		//so prepend command with env changing statements
		var setenv = "";

		//prefer user binaries then exodus binaries AFTER existing path
		let newpath = path ^ ":" ^ home ^ "/bin:/var/lib/exodus/bin";
		//if (!ossetenv("PATH",newpath))
		//	printl("Could not set PATH="^newpath);
		setenv ^= "PATH=" ^ newpath;

		//printx("LD_LIBRARY_PATH","~/lib:"^osgetenv("LD_LIBRARY_PATH"));
		let newlibpath = home ^ "/lib:" ^ libp;
		//if (!ossetenv("LD_LIBRARY_PATH",newlibpath))
		//	printl("Could not set LD_LIBRARY_PATH="^newlibpath);
		setenv ^= " LD_LIBRARY_PATH=" ^ newlibpath;

		//var path2=osgetenv("PATH");
		//if (path2!=newpath) {
		//	printl("Failed to set PATH to "^newpath);
		//	printl("PATH was: "^path);
		//	printl("PATH is:  "^path2);
		//}

		if (verbose) {
			osgetenv("HOME").outputl("HOME=");
			osgetenv("PATH").outputl("PATH=");
			osgetenv("LD_LIBRARY_PATH").outputl("LD_LIBRARY_PATH=");
		}

		//enable core dumps
		if (not osshell("ulimit -c unlimited"))
			lasterror().errputl("exodus:");

		//execute command or enter exodus shell
		//if (not osshell("env PS1='exodus [\\u@\\h \\W]\\$ '  "^(command?command:shell)))

		// This may have no effect since setting environment variables is error prone
		let shellcmd = setenv ^ " " ^ (command ? command : shell);
		if (verbose)
			printl("SHELL=\n" ^ shellcmd);
		if (not osshell(shellcmd))
			lasterror().errputl("exodus: ");

	} else if (OSSLASH == "\\" and shell.osgetenv("ComSpec")) {

		//windows

		//set EXO_PATH used by compile to find LIB and INCLUDE paths
		var exoduspath = exodusbinpath;
		if (exoduspath.field(OSSLASH, -1) == "bin")
			exoduspath = field(exoduspath, OSSLASH, 1, fcount(exoduspath, OSSLASH) - 1);
		if (exoduspath)
			ossetenv("EXO_PATH", exoduspath);

		let oldpath = osgetenv("PATH");
		var newpath = oldpath;

		//forcibly PREFIX exodus bin to path
		//when debugging in VS ... we need to overcome any standard exodus path!
		//if (not newpath.locateusing(exodusbinpath,";"))
		newpath = exodusbinpath ^ ";" ^ newpath;

		//APPEND user's Exodus binaries path (from compile/catalog)
		//could use LOCALAPPDATA on WIN7 up but getting too complicated so lets use USERPROFILE everywhere
		let homedir = osgetenv("USERPROFILE");
		if (homedir) {
			let bindir = homedir ^ "\\Exodus\\bin";
			if (not newpath.locateusing(";", bindir))
				newpath ^= ";" ^ bindir;
		}

		//update path
		if (newpath != oldpath)
			ossetenv("PATH", newpath);

		//set INCLUDE path
		//prefix path to exodus.h to INCLUDE environment variable
		//1. ..\include folder parallel to path of executable
		//2. build source folder of exodus
		//3. EXO_INCLUDE envvar
		var exodusincludepath = exodusbinpath ^ "\\..\\include";
		var searched = exodusincludepath;
		if (!osdir(exodusincludepath)) {
			exodusincludepath = exodusbinpath ^ "\\..\\exodus\\exodus";
			searched(-1) = exodusincludepath;
		}
		if (!osdir(exodusincludepath)) {
			exodusincludepath = exodusbinpath ^ "\\..\\..\\exodus\\exodus";
			searched(-1) = exodusincludepath;
		}
		if (!osfile(exodusincludepath ^ "\\exodus\\exodus.h")) {
			exodusincludepath = osgetenv("EXO_INCLUDE");
			searched(-1) = exodusincludepath;
			if (!osfile(exodusincludepath ^ "\\exodus.h")) {
				errputl("exodus: Couldnt find exodus include path (exodus.h)");
				errputl("exodus: " ^ searched.replace(FM, "\n"));
				exodusincludepath = "";
			}
		}
		ossetenv("EXO_INCLUDE", exodusincludepath);
		if (exodusincludepath)
			ossetenv("INCLUDE", (exodusincludepath ^ ";" ^ osgetenv("INCLUDE")));

		//set LIB path
		//prefix path to exodus.lib to LIB environment variable
		//1. path of executable
		//2. ..\lib folder parallel to path of executable
		//3. EXO_LIBPATH envvar
		var exoduslibpath = exodusbinpath;
		if (!osfile(exoduslibpath ^ "\\exodus.lib"))
			exoduslibpath = exodusbinpath ^ "\\..\\lib";
		if (!osfile(exoduslibpath ^ "\\exodus.lib"))
			exoduslibpath = osgetenv("EXO_LIBPATH");
		if (!osfile(exoduslibpath ^ "\\exodus.lib")) {
			exoduslibpath = osgetenv("EXO_LIBPATH");
			if (!osfile(exoduslibpath ^ "\\exodus.lib")) {
				errput("exodus: Couldnt find exodus include path (exodus.h)");
				exoduslibpath = "";
			}
		}
		// This may have no effect since setting environment variable is error prone
		ossetenv("EXO_LIBPATH", exoduslibpath);
		if (exoduslibpath)
			ossetenv("LIB", (exoduslibpath ^ ";" ^ osgetenv("LIB")));

		if (not osshell((command ?: shell)))
			lasterror().errputl("exodus: ");

	} else
		abort("Cannot find SHELL or ComSpec in environment");

	return 0;
}

programexit()
