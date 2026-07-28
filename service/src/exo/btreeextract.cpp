#include <exodus/library.h>
libraryinit()

#include <sysmsg.h>

#include <service_common.h>

// btreeextract — legacy name for "find record keys matching index terms".
//
// Uses SELECT (not a hand-rolled btree walk). Fields ending XREF go through
// Exodus FTS (to_tsvector/tsquery); createindex builds matching GIN indexes.
//
// Call protocol (unchanged):
//   cmd   = FIELDNAME VM searchterms FM
//   terms = legacy "WORD]&WORD]" (callers still decorate that way)
//   hits  = VM-separated keys (capped below)
//
// TODO: optional one-place term normalise (]& / & / spaces / plain words) if
// product wants callers to pass plain text only — do not change call sites
// until that is an explicit decision.
// TODO: document FTS vs non-XREF successive SELECT in man/notes if needed.

func main(in cmd, in filename, in /*dictfile*/, out hits) {

	// separate cursor
	var	 v69;
	var	 v70;
	var	 v71;
	call pushselect(v69);

	hits = "";

	// Cap only here (not in vardb). SELECT maxhits+1 so we can tell LIMIT
	// truncation from an exact maxhits result. Callers look for "maximum" in msg_.
	var maxhits	  = 10000;
	var selectmax = maxhits + 1;

	// XREF should be able to do it all in one go
	let fieldname = cmd.f(1, 1);
	var parts	  = cmd.f(1, 2);
	if (fieldname.ends("XREF")) {

		parts = cmd.f(1).remove(1, 1);

		// XREF was implemented as STARTING so ] was not required
		parts.converter("]", "");

		var selectcmd =
			"SELECT " ^ selectmax ^ " " ^ filename
			^ " WITH " ^ fieldname ^ " " ^ quote(parts) ^ " (S)";
		select(selectcmd);

		// otherwise do successive selects each one reducing the list
	} else {
		var nparts = parts.fcount("&");
		for (const var partn : range(1, nparts)) {
			var part	  = parts.field("&", partn);
			var selectcmd =
				"SELECT " ^ selectmax ^ " " ^ filename
				^ " WITH " ^ fieldname ^ " " ^ quote(part) ^ " (S)";
			select(selectcmd);
			if (not LISTACTIVE)
				break;
		}
	}

	// turn the select list into a string of fields
	var capped = false;
	if (LISTACTIVE) {
		hits = "";
		var n = 0;
nextrec:
		var key;
		if (readnext(key)) {
			n += 1;
			// Keep at most maxhits; (maxhits+1)th key means SQL/list was truncated
			if (n <= maxhits and hits.len() + key.len() < maxstrsize_ - 30) {
				hits ^= key ^ VM;
				goto nextrec;
			}
			capped = true;
		}
		if (hits)
			hits.popper();
		if (capped) {
			// "maximum" token for selectbtree / LISTSCHED / AGENCY.SUBS rewrite
			msg_ = "maximum " ^ maxhits ^ " " ^ filename ^ " keys for " ^ fieldname;
			call sysmsg(msg_, "BTREEEXTRACT");
		}
	}

	clearselect();
	call popselect(v69);

	return 0;
}

}; // libraryexit()
