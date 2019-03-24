#include <exodus/library.h>
libraryinit()

//#include <pushselect.h>
//#include <safeselect.h>
//#include <popselect.h>

function main(in cmd, in filename, in dictfile, out hits) {

	//replacement for simple btree.extract functions using SELECT command
	//which is slower but perhaps more portable and works without btree indexes

	//eg cmd contains "SEQUENCE.XREF" VM XYZ]&ABC] FM
	//finds records which contain words starting with XYZ *AND* ABC

	//TODO implement separate cursor
	//var v69;
	//var v70;
	//var v71;
	//call pushselect(0, v69, v70, v71);
	//var().clearselect();

	hits="";

	//defeat compiler warning of unused
	if (false) print(dictfile);

	//do successive selects each one reducing the list
	var fieldname = cmd.a(1, 1);
	var parts = cmd.a(1, 2);
	var nparts = parts.count("&") + 1;
	for (var partn = 1; partn <= nparts; ++partn) {
		var part = parts.field("&", partn);
		var selectcmd = "SELECT " ^ filename ^ " WITH " ^ fieldname ^ " " ^ quote(part) ^ " (S)";
		//call safeselect(select);
		select(selectcmd);

		///BREAK;
		if (not LISTACTIVE) break;;

	};//partn;

	//turn the select list into a string of fields
	if (LISTACTIVE) {
		hits = VM;
nextrec:
		var key;
		if (var().readnext(key)) {
			if (hits.length() + key.length() < 65500) {
				hits ^= VM ^ key;
				goto nextrec;
			}
		}
	}

	var().clearselect();
	//call popselect(0, v69, v70, v71);

	return 0;

}


libraryexit()
