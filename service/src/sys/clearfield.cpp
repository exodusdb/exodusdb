#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <flushindex.h>

#include <sys_common.h>

var options;
var nindexed;//num

function main() {
	//

	if (SENTENCE.contains("(")) {
		options = field2(SENTENCE, "(", -1);
		SENTENCE.cutter(-options.len() - 1);
		SENTENCE.trimmerlast();
	} else {
		options = "";
	}

	var filename = SENTENCE.field(" ", 2);
	var syntax = "Syntax is:";
	syntax(-1) = "CLEARFIELD filename fieldnoorname ... /\"clear to value\"";
	syntax(-1) = "eg CLEARFIELD TASKS STATUS /\"Completed\"";
	if (not filename) {
		call mssg(syntax);
		stop();
	}

	var fns = SENTENCE.field(" ", 3, 9999).field("/",1).trim();
	var cleartovalue = SENTENCE.field("/", 2, 9999);
	if ((cleartovalue.starts(DQ)) and (cleartovalue.ends(DQ))) {
		cleartovalue.cutter(1);
		cleartovalue.popper();
	} else if ((cleartovalue.starts("'")) and (cleartovalue.ends("'"))) {
		cleartovalue.cutter(1);
		cleartovalue.popper();
	}
	if (options.contains("I")) {
		cleartovalue.inverter();
	}
	var file;
	if (not(file.open(filename, ""))) {
		call mssg(filename.quote() ^ " FILE IS MISSING");
		stop();
	}
	if (not(DICT.open("DICT." ^ filename))) {
		DICT = "";
	}
	nindexed = 0;

	if (not fns) {
		call mssg("FIELD NO ?" ^ FM ^ syntax);
		stop();
	}
	fns.converter(" ,", FM ^ FM);
	var nfields = fns.fcount(FM);

	//check numeric
	// space to defeat convsyntax
	for ( var ii = 1; ii <= nfields; ii++) {
tryagain:
		var fn = fns.f(ii);
		//prevent zero as in CLEARFIELD X Y Z 0 - needs CLEARFIELD X Y Z/0
		if (not(fn)) {
			call mssg(fn.quote() ^ " field number is not supported");
			stop();
		}
		if (not(fn.isnum())) {
			if (DICT) {
				//readv fn from @dict,fns<i>,2 else null
				var dict;
				if (dict.read(DICT, fns.f(ii))) {
					if (dict.f(1) eq "F") {
						fn = dict.f(2);
					} else if (dict.f(1) eq "G") {
						var tt = dict.f(3);
						tt.converter(" ", FM);
						fns(ii) = tt;
						nfields = fns.fcount(FM);
						goto tryagain;
					}
				}
			}
			if (fn.isnum()) {
				fns(ii) = fn;
			} else {
				call mssg(SENTENCE ^ "|FIELD NUMBER " ^ fn ^ " IS NOT NUMERIC AND|NOT AN \"F\" DICTIONARY ITEM!?" ^ FM ^ FM ^ syntax);
				stop();
			}
		}
	} //ii;

	if (not LISTACTIVE) {
		select(file);
	}
	var recn = 0;
	var nrecs = file.reccount();
	if (nrecs) {
		nrecs = "/" ^ nrecs;
	}

	var isindexed = file.contains("SI.MFS") ne 0;

next:
/////

	call giveway();
	if (not(readnext(ID))) {
		goto exit;
	}

	if (ID.starts("%")) {
		goto next;
	}
	recn += 1;
	//if interactive then PRINT @(@crthigh/2,25) else print @(0):@(-4):
	if (TERMINAL)
		output(AT(-40), recn, nrecs , "\x09" , ID.oconv("L#30"), " ");
	if (RECORD.read(file, ID)) {

		var cleartovalue2 = cleartovalue;
		if (cleartovalue.starts("{")) {
			if (cleartovalue.ends("}")) {
				cleartovalue2 = calculate(cleartovalue.cut(1).pop());
			}
		}

		var cleared = 0;
		for (const var ii : range(1, nfields)) {
			if (RECORD.f(fns.f(ii)) ne cleartovalue2) {
				cleared = 1;
				//RECORD(fns.f(ii)) = cleartovalue2;
				pickreplacer(RECORD, fns.f(ii), cleartovalue2);
			}
		} //ii;
		if (cleared) {
			while (true) {
				///BREAK;
				if (not(RECORD and ((FM ^ VM ^ SM).contains(RECORD[-1])))) break;
				RECORD.popper();
			}//loop;
			if (RECORD eq "") {
				file.deleterecord(ID);

			} else {
				RECORD.write(file, ID);
			}
			nindexed += isindexed;
			if (nindexed gt 100) {
				gosub flush(filename);
			}
			//PRINT 'CLEARED':
			//print
		}
	}
	//PRINT
	goto next;

/////
exit:
/////
	gosub flush(filename);
	stop();

	return "";
}

subroutine flush(in filename) {

	call flushindex(filename);
	nindexed = 0;
	return;
}

libraryexit()
