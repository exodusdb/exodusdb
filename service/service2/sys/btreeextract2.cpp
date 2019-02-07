#include <exodus/library.h>
libraryinit()

#include <xselect.h>

#include <gen.h>

var v69;
var v70;
var v71;
var withwithout;
var id;
var xx;

function main(in cmd, in filename, in dictfile, out hits, out msg) {
	//c sys in,in,in,out,out

	//replacement for simple btree.extract functions using SELECT command
	//which is SLOWER but perhaps more portable and works without btree indexes

	//should be converted to use BTREE.EXTRACT where possible

	//https://wiki.srpcs.com/display/Commands/Btree.Extract+Subroutine

	//eg cmd contains "SEQUENCE.XREF" VM ABC]&DEF] VM XYZ]&PQR] FM
	//finds records which contain words
	// starting with ABC *and* starting with DEF
	// or
	// starting with XYZ *and* starting with PQR

	//avoid c++ warning about unused variable
	if (dictfile.unassigned()) {
		{}
	}

	call pushselect(0, v69, v70, v71);
	clearselect();

	var fieldname = cmd.a(1, 1);
	var texts = cmd.field(VM, 2, 9999);
	var ntexts = texts.count(VM) + 1;
	hits = "";
	msg = "";

	//analogous code whereever allpunctuation is used
	//ED BP LISTSCHED AGENCY.SUBS SCHEDULES.DICT
	//ED GBP BTREE.EXTRACT2
	//allow ] to indicate STARTING (eg XXXX]
	//allpunctuation=allpunctuation
	//convert '-]' to '' in allpunctuation
	//convert allpunctuation to space(50) in texts
	texts.converter(SYSTEM.a(130), var(50).space());

	//a b mv c d becomes
	//1. select xxx with a and with b
	//2. add hits
	//3. select xxx with c and with d
	//4. add hits

	for (var textn = 1; textn <= ntexts; ++textn) {

		var textcmd = "SELECT " ^ filename;
		fieldname = cmd.a(1, 1);
		var containing = "";
		var words = texts.a(1, textn).trim();
		if (words.length()) {

			var nwords = words.count(" ") + 1;
			for (var wordn = 1; wordn <= nwords; ++wordn) {
				var word = words.field(" ", wordn);
				if (wordn > 1) {
					textcmd ^= " AND";
				}
				if (word[1] == "-") {
					if (wordn == 1) {
						msg = "Search cannot start with -";
						return 0;
					}
					withwithout = " WITHOUT ";
					word.splicer(1, 1, "");
				}else{
					withwithout = " WITH ";
				}
				if (containing) {
					word.converter("]", "");
				}
				textcmd ^= withwithout ^ fieldname ^ containing ^ " " ^ (DQ ^ (word ^ DQ));
				if (fieldname.index(".XREF")) {
					fieldname.swapper(".XREF", "");
					containing = " CONTAINING";
				}
			};//wordn;

			call xselect(textcmd ^ " (S)");

			gosub addhits( hits);

		}

	};//textn;

	clearselect();
	call popselect(0, v69, v70, v71);

	return 0;

}

subroutine addhits(io hits) {
	//addhits(io hits)
	//turn the select list into a string of fields

	if (not LISTACTIVE) {
		return;
	}
	var merge = hits ne "";

nextrec:
	if (readnext(id)) {
		if (hits.length() + id.length() > 65500) {
			clearselect();
			return;
		}
		if (merge) {
			if (not(hits.locateusing(id, VM, xx))) {
				hits ^= VM ^ id;
			}
		}else{
			hits ^= VM ^ id;
		}
		goto nextrec;
	}

	return;

}


libraryexit()
