#include <exodus/program.h>
programinit()

function main() {

	//SENTENCE
	//waiting filepattern seconds milliseconds globalend newfilename newfilepath portno?
	//eg for a database called devdtest and process 3
	//waiting ../data/devdtest/*.1 10 10 ../../global.end neos0000.3 ../data/devdtest/ 5700
        var filename=field(SENTENCE," ",2);
        var waitsecs=field(SENTENCE," ",3);
        var sleepms=field(SENTENCE," ",4);
        var globalendfilename=field(SENTENCE," ",5);
        var databaseendfilename="global.end";
        var newfilename=field(SENTENCE," ",7) ^ field(SENTENCE," ",6);

        //printl("Waiting for file ", filename);
        //printl("New file will be ",newfilename);
	//printl();

        for (var ii=0;ii<=(1000/sleepms*waitsecs);ii++) {

		//quit if any key pressed
		if (var().hasinput())
			break;

		//look for required file
                var filenames=oslist(filename);

		//do loop if no file
                if (!filenames) {

		//if exists and not renaming then quit
                } else if (!newfilename) {
                        break;

		//if exists and renaming then rename and quit, or process loop if cannot rename
		} else {

	                var oldfilename=filenames.a(1);
	                printl("Found file ",oldfilename);

			//rename and quit
	                if (osrename(oldfilename,newfilename)) {
	                        printl("File renamed from ",oldfilename, " to ", newfilename);
	                        break;

			//process another loop if cannot rename, presumably because another process renamed first
	                } else {
	                        printl("File not renamed from ",oldfilename, " to ", newfilename);
	                        //printl("Get Last Error reports %d\n", GetLastError ());
			}
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

	}//next

        return 0;
}

programexit()

