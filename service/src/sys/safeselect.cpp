#include <exodus/library.h>
libraryinit()

#include <xselect.h>

var rr;
var id;
var dict;

function main(in selectx) {
	//c sys

	RECORD.transfer(rr);
	ID.transfer(id);
	DICT.transfer(dict);
	//if @username='NEOSYS' then oswrite selectx on 'SAFESELE'
	//CALL PERF(selectX)
	call xselect(selectx);

	rr.transfer(RECORD);
	id.transfer(ID);
	dict.transfer(DICT);

	return 0;
}

libraryexit()
