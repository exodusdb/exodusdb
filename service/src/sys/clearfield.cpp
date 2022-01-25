#include <exodus/library.h>
libraryinit()

#include <giveway.h>
#include <flushindex.h>

#include <sys_common.h>

var options;
var nindexed;//num

function main() {
	//
	//c sys
	//global nindexed

	if (SENTENCE.index("(")) {
		options = field2(SENTENCE, "(", -1);
		SENTENCE.splicer(-options.length() - 1, 9999, "");
		SENTENCE.trimmerb();
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
	if ((cleartovalue[1] eq DQ) and (cleartovalue[-1] eq DQ)) {
		cleartovalue.splicer(1, 1, "");
		cleartovalue.popper();
	} else if ((cleartovalue[1] eq "'") and (cleartovalue[-1] eq "'")) {
		cleartovalue.splicer(1, 1, "");
		cleartovalue.popper();
	}
	if (options.index("I")) {
		cleartovalue = cleartovalue.invert();
	}
	var file;
	if (not(file.open(filename, ""))) {
		call mssg(filename.quote() ^ " FILE IS MISSING");
		stop();
	}
	if (not(DICT.open("DICT." ^ filename))) {
		DICT = "";
	}
	var interactive = not(SYSTEM.a(33));
	nindexed = 0;

	if (not fns) {
		call mssg("FIELD NO ?" ^ FM ^ syntax);
		stop();
	}
	fns.converter(" ,", FM ^ FM);
	var nfields = fns.count(FM) + 1;

	//check numeric
	for (var ii : range(1, nfields)) {
tryagain:
		var fn = fns.a(ii);
		//prevent zero as in CLEARFIELD X Y Z 0 - needs CLEARFIELD X Y Z/0
		if (not(fn)) {
			call mssg(fn.quote() ^ " field number is not supported");
			stop();
		}
		if (not(fn.isnum())) {
			if (DICT) {
				//readv fn from @dict,fns<i>,2 else null
				var dict;
				if (dict.read(DICT, fns.a(ii))) {
					if (dict.a(1) eq "F") {
						fn = dict.a(2);
					} else if (dict.a(1) eq "G") {
						var tt = dict.a(3);
						tt.converter(" ", FM);
						fns(ii) = tt;
						nfields = fns.count(FM) + 1;
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
	var count = 0;
	var nrecs = file.reccount();
	if (nrecs) {
		nrecs = "/" ^ nrecs;
	}

	var isindexed = file.index("SI.MFS") ne 0;

next:
/////

	call giveway();
	if (not(readnext(ID))) {
		goto exit;
	}

	if (ID[1] eq "%") {
		goto next;
	}
	count += 1;
	//if interactive then PRINT @(@crthigh/2,25) else print @(0):@(-4):
	if (TERMINAL)
		output(at(-40), count, nrecs , "\x09" , ID.oconv("L#30"), " ");
	if (RECORD.read(file, ID)) {

		var cleartovalue2 = cleartovalue;
		if (cleartovalue[1] eq "{") {
			if (cleartovalue[-1] eq "}") {
				cleartovalue2 = calculate(cleartovalue.substr(2, cleartovalue.length() - 2));
			}
		}

		var cleared = 0;
		for (var ii : range(1, nfields)) {
			if (RECORD.a(fns.a(ii)) ne cleartovalue2) {
				cleared = 1;
				//RECORD(fns.a(ii)) = cleartovalue2;
				pickreplacer(RECORD, fns.a(ii), cleartovalue2);
			}
		} //ii;
		if (cleared) {
			while (true) {
				///BREAK;
				if (not(RECORD and ((FM ^ VM ^ SVM).index(RECORD[-1])))) break;
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
