#include <exodus/library.h>
libraryinit()

#include <openfile.h>

var autocreate;//num
var reply;

function main(in filename, io file, in similarfilename="", in autocreate="") {

	//evade compiler warning
	if (false&&autocreate) {}

	if (file.open(filename)) {
		return true;
	}

	//option to create file if it does not exist
	if (not similarfilename or not var().open(similarfilename)) {
		call note(filename.quote() ^ " ERROR: FILE IS MISSING OR CANNOT BE ACCESSED");
		return false;
	}

	if (not var().createfile(filename)) {
		call note(filename.quote() ^ " ERROR: FILE CANNOT BE CREATED");
		return false;
	}

	if (file.open(filename)) {
		call note(filename.quote() ^ " WARNING: FILE CREATED AND OPENED");
		return true;
	}

	call note(filename.quote() ^ " ERROR: FILE CREATED BUT CANNOT BE OPENED");
	return false;

}


libraryexit()
