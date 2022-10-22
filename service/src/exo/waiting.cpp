#include <exodus/library.h>
libraryinit()

function main() {

	breakoff();

	//SENTENCE
	//waiting filepattern seconds milliseconds globalend newfilename newfilepath portno?
	//eg for a database called devdtest and process 3
	//waiting ../data/devdtest/*.1 10 10 ../../global.end neos0000.3 ../data/devdtest/ 5700
	//printl(SENTENCE);
	var filepattern			= field(SENTENCE, " ", 2);
	var waitsecs			= field(SENTENCE, " ", 3);
	var sleepms				= field(SENTENCE, " ", 4);
	var globalendfilename	= field(SENTENCE, " ", 5);
	var databaseendfilename = "global.end";
	var newfilename			= field(SENTENCE, " ", 7) ^ field(SENTENCE, " ", 6);

	//printl("Waiting for file ", filename);
	//printl("New file will be ",newfilename);
	//printl();

	var n		   = 0;
	var oswaitsecs = 1;

	var starttime = ostime();
	do {

		if (TERMINATE_req || RELOAD_req)
			return 0;

		//limit remaining seconds to 0-10
		var elapsedsecs = (ostime() - starttime);
		if (elapsedsecs < 0)
			break;
		var remainingsecs = waitsecs - elapsedsecs;
		if (remainingsecs < 1)
			break;

		//start at one second and double every timeout
		//reason is because events sometimes dont get reported
		//so we need to check for actual files present
		//at least in test where only one process is running
		oswaitsecs += oswaitsecs;
		//but no more than remaining secs
		if (oswaitsecs > remainingsecs)
			oswaitsecs = remainingsecs;

		//oswait doesnt support file pattern yet
		//printl(++n , " ", oswaitsecs," ", remainingsecs);
		var().oswait(oswaitsecs * 1000, field(filepattern, "*", 1));

		//quit if any key pressed (not possible if running as a service)
		if (var().hasinput())
			break;

		//look for required file
		var filenames = oslist(filepattern);

		if (filenames) {

			//if not renaming then quit
			if (!newfilename)
				break;

			//delete any existing newfilename in case left from some crash
			if (osfile(newfilename)) {
				if (not osremove(newfilename))
					printl("Could not delete existing file ", newfilename);
			}

			//if renaming then rename and quit, or process loop if cannot rename

			//rename and quit
			var oldfilename = filepattern.fieldstore(OSSLASH, -1, 1, filenames.f(1));
			if (osrename(oldfilename, newfilename)) {
				//printl("File renamed from ",oldfilename, " to ", newfilename);
				//var x;
				//x.input();
				break;
			}

			//printl("File not renamed from ",oldfilename, " to ", newfilename);
			//printl("Get Last Error reports %d\n", GetLastError ());
		}

		//quit if database.END exists
		if (databaseendfilename) {
			if (osfile(databaseendfilename)) {
				printl("Found file ", databaseendfilename);
				break;
			}
		}

		//quit if GLOBAL.END exists
		if (osfile(globalendfilename)) {
			printl("Found file ", globalendfilename);
			break;
		}

		ossleep(sleepms);

	} while (true);

	breakon();

	return 0;
}

libraryexit()
