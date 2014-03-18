#include <exodus/library.h>
libraryinit()

#include <pushselect.h>
#include <quote2.h>
#include <safeselect.h>
#include <popselect.h>

#include <gen.h>

var v69;
var v70;
var v71;

function main() {

	//replacement for simple btree.extract functions using SELECT command
	//which is slower but perhaps more portable and works without btree indexes

	//eg cmd contains "SEQUENCE.XREF" VM XYZ]&ABC] FM
	//finds records which contain words starting with XYZ *AND* ABC

	call pushselect(0, v69, v70, v71);
	var().clearselect();

	//do successive selects each one reducing the list
	var fieldname = cmd.a(1, 1);
	var parts = cmd.a(1, 2);
	var nparts = parts.count("&") + 1;
	for (var partn = 1; partn <= nparts; ++partn) {
		var part = parts.field("&", partn);
		var select = "SELECT " ^ filename ^ " WITH " ^ fieldname ^ " " ^ quote2(part) ^ " (S)";
		call safeselect(select);
	///BREAK;
	if (not LISTACTIVE) break;;
	};//partn;

	//turn the select list into a string of fields
	if (LISTACTIVE) {
		var hits = FM;
nextrec:
		var ID;
		if (ID.readnext()) {
			if (hits.length() + ID.length() < 65500) {
				hits ^= FM ^ ID;
				goto nextrec;
			}
		}
	}else{
		var hits = "";
	}

	var().clearselect();
	call popselect(0, v69, v70, v71);

	return 0;

}


libraryexit()