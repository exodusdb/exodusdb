#include <exodus/library.h>
libraryinit()

#include <service_common.h>

function main(in cmd, in filename, in /*dictfile*/, out hits) {

	// replacement for simple btree.extract functions using SELECT command
	// which is slower but perhaps more portable and works without btree indexes

	// eg cmd contains "SEQUENCE.XREF" VM XYZ]&ABC] FM
	// finds records which contain words starting with XYZ *AND* ABC

	// separate cursor
	var	 v69;
	var	 v70;
	var	 v71;
	call pushselect(v69);

	hits = "";

	// defeat compiler warning of unused
//	if (false)
//		printx(dictfile);

	// XREF should be able to do it all in one go
	let fieldname = cmd.f(1, 1);
	var parts	  = cmd.f(1, 2);
	if (fieldname.ends("XREF")) {

		parts = cmd.f(1).remove(1, 1);

		// XREF sql was implemented as STARTING so ] was not required
		parts.converter("]", "");

		let selectcmd = "SELECT " ^ filename ^ " WITH " ^ fieldname ^ " " ^ quote(parts) ^ " (S)";
		select(selectcmd);

		// otherwise do successive selects each one reducing the list
	} else {
		let nparts = parts.fcount("&");
		// parts.outputl("parts=");
		for (const var partn : range(1, nparts)) {
			let part	  = parts.field("&", partn);
			let selectcmd = "SELECT " ^ filename ^ " WITH " ^ fieldname ^ " " ^ quote(part) ^ " (S)";
			// selectcmd.outputl("selectcmd=");
			// call safeselect(select);
			select(selectcmd);
			// /BREAK;
			if (not LISTACTIVE)
				break;

		}  // partn;
	}

	// turn the select list into a string of fields
	if (LISTACTIVE) {
		hits = "";
nextrec:
		var key;
		if (readnext(key)) {
			if (hits.len() + key.len() < maxstrsize_ - 30) {
				hits ^= key ^ VM;
				goto nextrec;
			}
		}
		hits.popper();
	}

	clearselect();
	call popselect(v69);

	return 0;
}

libraryexit()
