#include <exodus/library.h>
libraryinit()

#include <xselect.h>

	var rr;
var id;
var dict;

function main(in selectx) {

	RECORD.move(rr);
	ID.move(id);
	DICT.move(dict);
	//if @username='EXODUS' then oswrite selectx on 'SAFESELE'
	//CALL PERF(selectX)
	call xselect(selectx);

	rr.move(RECORD);
	id.move(ID);
	dict.move(DICT);

	return 0;
}

libraryexit()
