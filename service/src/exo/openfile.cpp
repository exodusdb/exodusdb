#include <exodus/library.h>
libraryinit()

func main(in filename, io file, in similarfilename = "", in /*autocreate*/ = "") {

	// Success in the usual case that the file can be opened
	if (file.open(filename)) {
		return true;
	}

	// Option to create the file if it cannot be opened and presumable does not exist
	if (not similarfilename or not var().open(similarfilename)) {
		call note(filename.quote() ^ " ERROR: FILE IS MISSING OR CANNOT BE ACCESSED");
		return false;
	}

	// Fail if file is missing and cannot be created
	if (not var().createfile(filename)) {
		call note(filename.quote() ^ " ERROR: FILE CANNOT BE CREATED");
		return false;
	}

	// Retry open and fail if the created file cannot be opened
	if (not file.open(filename)) {
		call note(filename.quote() ^ " ERROR: FILE CREATED BUT CANNOT BE OPENED");
		return false;
	}

	// Success
	call note(filename.quote() ^ " WARNING: FILE CREATED AND OPENED");
	return true;
}

}; // libraryexit()
