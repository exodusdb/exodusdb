#include <exodus/library.h>
libraryinit()


#include <gen.h>

var indexingrecord;

function main(in mode, in filename, io keys, in fieldnames, in oldvalues, in newvalues, io valid, io msg) {
	//c job in,in,io,in,in,in,io,io
	//update symbolic indexed files
	//this is designed to be called in prewrite
	//to update the indexes of associated records
	//that are symbolic and depend on the current record
	//since indexing on derived fields fails to be updated
	//when the derived fields change

	//1. CALL('LOCK' - to lock all associated records
	//2. CALL('UNLOCK' - in case some associated records cannot be locked
	//3. CALL('INDEXINGLOCK') to lock !INDEXING
	//4. CALL('UPDATE' - to add to the !INDEXING record
	//5. CALL('INDEXING' - to unlock !INDEXING

	var secstowaitforindexinglock = 99;
	var secstowaitforlock = 9;
	var indexingkey = "0";

	valid = 0;
	msg = "Unknown error in UPD.SECINDEX";

	var indexingfile;
	if (not(indexingfile.open("!INDEXING", ""))) {
		msg = "SYSTEM ERROR: " ^ (DQ ^ ("!INDEXING" ^ DQ)) ^ " file cannot be opened";
		return 0;
	}

	if (mode == "INDEXINGLOCK") {
		if (not(lockrecord("!INDEXING", indexingfile, indexingkey, indexingrecord, secstowaitforindexinglock))) {
			msg = "Central !INDEXING record is in use elsewhere";
			return 0;
		}

	} else if (mode == "INDEXINGUNLOCK") {
		call unlockrecord("!INDEXING", indexingfile, indexingkey);

	} else if (mode == "LOCK" or mode == "UNLOCK" or mode == "UPDATE") {

		var file;
		if (not(file.open(filename, ""))) {
			msg = "SYSTEM ERROR: " ^ (DQ ^ (filename ^ DQ)) ^ " file cannot be opened";
			return 0;
		}

		var indexinglist = "";

		var upd = "";
		var cnt = 0;

		//allow for keys to be passed as vm
		keys.converter(VM, FM);

		var nkeys = keys.count(FM) + (keys ne "");
		for (var keyn = 1; keyn <= nkeys; ++keyn) {
			var keyx = keys.a(keyn);

			if (mode == "LOCK") {
				if (not lockrecord(filename, file, keyx, RECORD, secstowaitforlock)) {

					//!!!! return only locked keys
					if (keyn == 1) {
						keys = "";
					}else{
						keys = keys.field(FM, 1, keyn - 1);
					}

					msg = filename ^ " " ^ keyx ^ " is in use elsewhere";
					return 0;
				}

			} else if (mode == "UNLOCK") {

				//duplicate code in UPDATE and UNLOCK
				call unlockrecord(filename, file, keyx);

			} else if (mode == "UPDATE") {

				var nfields = fieldnames.count(FM) + (fieldnames ne "");
				for (var fieldn = 1; fieldn <= nfields; ++fieldn) {

					var fieldname = fieldnames.a(fieldn);

					if (oldvalues.a(fieldn) ne newvalues.a(fieldn)) {
						upd ^= fieldname ^ FM ^ keyx ^ FM ^ oldvalues.a(fieldn) ^ FM ^ newvalues.a(fieldn) ^ FM;
						cnt += 1;
					}

				};//fieldn;

				//duplicate code in UPDATE and UNLOCK
				call unlockrecord(filename, file, keyx);

			}
//L486:
			//random comment to trigger correct adecom to c++ code
		};//keyn;

		//update
		if (mode == "UPDATE" and upd ne "") {

			var fileaccount = filename.xlate("FILES", 3, "X");
			var filevolume = filename.xlate("FILES", 1, "X");
			var volid = filevolume.xlate("VOLUMES", 1, "X");

			//list:= 'YYY*ADAGENCY*22:06:18 18 JAN 2004':@FM:YYY.CNT:@FM:YYY.UPD
			indexinglist ^= filename ^ "*" ^ fileaccount ^ "*" ^ volid ^ FM ^ cnt ^ FM ^ upd;

			if (not indexingrecord.read(indexingfile, indexingkey)) {
				indexingrecord = "0";
			}

			if (not(indexinglist[-1] == FM)) {
				indexinglist ^= FM;
			}
			indexingrecord ^= indexinglist;

			indexingrecord.write(indexingfile, indexingkey);

		}

	}
//L631:
	valid = 1;
	msg = "";

	return 0;

}


libraryexit()
