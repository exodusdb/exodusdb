#include <exodus/library.h>
libraryinit()

#include <sys_common.h>

var indexingfilename;
var indexingrecord;
var recordx;

function main(in mode, in filename, io keys, in fieldnameornos, in oldvalues, in newvalues, io valid, io msg) {
	//c sys in,in,io,in,in,in,io,io
	#include <system_common.h>
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

	if (VOLUMES) {
		indexingfilename = "!INDEXING";
	} else {
		//only used for locking
		indexingfilename = "DEFINITIONS";
	}

	var indexingfile;
	if (not(indexingfile.open(indexingfilename, ""))) {
		msg = "SYSTEM ERROR: " ^ (var("!INDEXING").quote()) ^ " file cannot be opened";
		return 0;
	}

	if (mode eq "INDEXINGLOCK") {
		if (not(lockrecord(indexingfilename, indexingfile, indexingkey, indexingrecord, secstowaitforindexinglock))) {
			msg = "Central !INDEXING record is in use elsewhere";
			return 0;
		}

	} else if (mode eq "INDEXINGUNLOCK") {
		call unlockrecord(indexingfilename, indexingfile, indexingkey);

	} else if ((mode eq "LOCK" or mode eq "UNLOCK") or mode eq "UPDATE") {

		var file;
		if (not(file.open(filename, ""))) {
			msg = "SYSTEM ERROR: " ^ (filename.quote()) ^ " file cannot be opened";
			return 0;
		}

		var indexinglist = "";

		var upd = "";
		var cnt = 0;

		//allow for keys to be passed as vm
		keys.converter(VM, FM);

		var nfields = fieldnameornos.count(FM) + (fieldnameornos ne "");

		var nkeys = keys.count(FM) + (keys ne "");
		for (const var keyn : range(1, nkeys)) {
			var keyx = keys.a(keyn);

			if (mode eq "LOCK") {
				if (not(lockrecord(filename, file, keyx, recordx, secstowaitforlock))) {

					//!!!! return only locked keys
					if (keyn eq 1) {
						keys = "";
					} else {
						keys = keys.field(FM, 1, keyn - 1);
					}

					msg = filename ^ " " ^ keyx ^ " is in use elsewhere";
					return 0;
				}

			} else if (mode eq "UNLOCK") {

				//duplicate code in UPDATE and UNLOCK
				call unlockrecord(filename, file, keyx);

			} else if (mode eq "UPDATE") {

				for (const var fieldn : range(1, nfields)) {

					var fieldnameorno = fieldnameornos.a(fieldn);

					if (oldvalues.a(fieldn) ne newvalues.a(fieldn)) {
						if (fieldnameorno.isnum()) {
							newvalues.a(fieldn).writev(file, keyx, fieldnameorno);

						} else {
							upd ^= fieldnameorno ^ FM ^ keyx ^ FM ^ oldvalues.a(fieldn) ^ FM ^ newvalues.a(fieldn) ^ FM;
						}
						cnt += 1;
					}

				} //fieldn;

				//duplicate code in UPDATE and UNLOCK
				call unlockrecord(filename, file, keyx);

			}

			//random comment to trigger correct adecom to c++ code
		} //keyn;

		//update
		if (mode eq "UPDATE" and upd ne "") {

			var fileaccount = filename.xlate("FILES", 3, "X");
			var filevolume = filename.xlate("FILES", 1, "X");
			var volid = filevolume.xlate("VOLUMES", 1, "X");

			//list:= 'YYY*ADAGENCY*22:06:18  18 JAN 2004':@FM:YYY.CNT:@FM:YYY.UPD
			indexinglist ^= filename ^ "*" ^ fileaccount ^ "*" ^ volid ^ FM ^ cnt ^ FM ^ upd;

			if (not(indexingrecord.read(indexingfile, indexingkey))) {
				indexingrecord = "0";
			}

			if (not(indexinglist[-1] eq FM)) {
				indexinglist ^= FM;
			}
			indexingrecord ^= indexinglist;

			indexingrecord.write(indexingfile, indexingkey);

		}

	}

	valid = 1;
	msg = "";

	return 0;
}

libraryexit()
