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

//#include <cstdlib> //for atexit()

#include <exodus/var.h>
#include <exodus/exoenv.h>
#include <exodus/exofuncs.h>

namespace exodus {


PUBLIC var input() {
	var v;
	v.input();
	return v;
}

PUBLIC var input(CVR prompt) {
	var v;
	v.input(prompt);
	return v;
}

PUBLIC var inputn(const int nchars) {
	var v;
	v.inputn(nchars);
	return v;
}


PUBLIC var replace(CVR instring, SV fromstr, SV tostr) {
	var newstring = instring; //GB ??? why new var unlike others e.g textconvert( or convert(
	return newstring.replace(fromstr, tostr);
}

PUBLIC var regex_replace(CVR instring, SV regex, SV replacement, SV options) {
	var newstring = instring;
	return newstring.regex_replacer(regex, replacement, options);
}

// POP

PUBLIC bool connect(CVR connectioninfo) {
	var conn1;
	return conn1.connect(connectioninfo);
}

PUBLIC bool createfile(CVR dbfilename) {
	//exodus doesnt automatically create dict files

	//remove options like (S)
	var dbfilename2 = dbfilename.field("(", 1).trim();

	//remove pickos volume locations
	dbfilename2.replacer("DATA ", "").replacer("REVBOOT ", "").replacer("DATAVOL ", "").trimmer();

	return dbfilename2.createfile(dbfilename2);
}

PUBLIC bool deletefile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field(" ", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.deletefile(dbfilename2);
}

PUBLIC bool clearfile(CVR dbfilename_or_var) {
	//remove options like (S)
	var dbfilename2 = dbfilename_or_var.field("(", 1).trim();

	//exodus doesnt automatically create dict files
	dbfilename2.replacer("DATA ", "").trimmer();

	return dbfilename_or_var.clearfile(dbfilename2);
}

PUBLIC bool createindex(CVR dbfilename_or_var, CVR fieldname, CVR dictdbfilename) {

	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		var dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		var indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.createindex(fieldname2, dictdbfilename);
	}

	return dbfilename_or_var.createindex(fieldname, dictdbfilename);
}

PUBLIC bool deleteindex(CVR dbfilename_or_var, CVR fieldname) {
	//virtually identical code in createindex and deleteindex
	if (dbfilename_or_var.contains(" ")) {
		var dbfilename2 = dbfilename_or_var.field(" ", 1);
		var fieldname2 = dbfilename_or_var.field(" ", 2);
		var indextype2 = dbfilename_or_var.field(" ", 3);
		if (indextype2 && indextype2 != "BTREE")
			fieldname2 ^= "_" ^ indextype2;
		return dbfilename2.deleteindex(fieldname2);
	}

	return dbfilename_or_var.deleteindex(fieldname);
}

PUBLIC
int exodus_main(int exodus__argc, const char* exodus__argv[], ExoEnv& mv, int threadno) {

	// signal/interrupt handlers
	// install_signals();
	//var().breakon();
	breakon();

	//	tss_environmentns.reset(new int(0));
	//	global_environments.resize(6);
	//int environmentn = 0;
	mv.init(threadno);
	// mv.DICT.outputl("DICT=");
	//	global_environments[environmentn] = &mv;

	mv.EXECPATH = getexecpath();
	if (not mv.EXECPATH) {
		if (exodus__argc)
			mv.EXECPATH = var(exodus__argv[0]);
		if (not mv.EXECPATH.contains(OSSLASH))
			mv.EXECPATH.prefixer(oscwd() ^ OSSLASH);
	}
	//"see getting path to current executable" above
	// or use "which EXECPATH somehow like in mvdebug.cpp
	// if (not EXECPATH.contains(OSSLASH) && not EXECPATH.contains(":"))
	//{
	//	EXECPATH.prefixer(oscwd()^OSSLASH);
	//	if (OSSLASH=="\\")
	//		EXECPATH.converter("/","\\");
	//}
	// leave a global copy where backtrace can get at it
	//EXECPATH2 = mv.EXECPATH;

	mv.SENTENCE = "";  // ALN:TODO: hm, again, char->var-> op=(var)

	mv.COMMAND = "";
	mv.OPTIONS = "";

	// reconstructs complete original sentence unfortunately quote marks will have been lost
	// unless escaped needs to go after various exodus definitions how the MSVCRT tokenizes
	// arguments http://msdn.microsoft.com/en-us/library/a1y7w461.aspx
	// http://stackoverflow.com/questions/4094699/how-does-the-windows-command-interpreter-cmd-exe-parse-scripts
	for (int ii = 0; ii < exodus__argc; ++ii) {
		var word = var(exodus__argv[ii]);
		if (ii == 0) {
			word = word.field2(OSSLASH, -1);
			// remove trailing ".exe"
			if (word.lcase().ends(".exe"))
				word.cutter(-4);
		} else
			mv.SENTENCE ^= " ";

		//dont do this because easier to understand bash expansion without it
		// put back quotes if any spaces
		//if (word.contains(" "))
		//	word.quoter();

		mv.SENTENCE ^= word;
		mv.COMMAND ^= word ^ FM_;
	}
	mv.COMMAND.popper();

	// Cut off OPTIONS from end of COMMAND if present
	// *** SIMILAR code in
	// 1. exofuncs.cpp exodus_main()
	// 2. exoprog.cpp  perform()
	// OPTIONS are in either (AbC) or {AbC} in the last field of COMMAND
	mv.OPTIONS = mv.COMMAND.field2(_FM, -1);
	if ((mv.OPTIONS.starts("{") and mv.OPTIONS.ends("}")) or (mv.OPTIONS.starts("(") and mv.OPTIONS.ends(")"))) {
		// Remove last field of COMMAND TODO fpopper command?
		mv.COMMAND.cutter(-mv.OPTIONS.len() - 1);
		// Remove first { or ( and last ) } chars of OPTIONS
		mv.OPTIONS.cutter(1).popper();
	} else {
		mv.OPTIONS = "";
	}

	var temp;
	// DBTRACE=osgetenv("EXO_DBTRACE",temp)?1:-1;

	// would have to passed in as a function pointer
	// main2(exodus__argc, exodus__argv);

	//atexit(exodus_atexit);

	if (isterminal())
		mv.TERMINAL = osgetenv("TERM");

	return 0;
}

//PUBLIC
//void exodus_atexit()
//{
//	// done in handle cache destructor now
//	//	var().disconnect();
//}

/* getting path to current executable

from http://stackoverflow.com/questions/143174/c-c-how-to-obtain-the-full-path-of-current-directory

Windows:

int bytes = GetModuleFileName(nullptr, pBuf, len);
if(bytes == 0)
	return -1;
else
	return bytes;

Linux but not macosx apparently:

char szTmp[32];
sprintf(szTmp, "/proc/%d/exe", getpid());
int bytes = MIN(readlink(szTmp, pBuf, len), len - 1);
if(bytes >= 0)
	pBuf[bytes] = '\0';
return bytes;
*/

}  // namespace exodus
