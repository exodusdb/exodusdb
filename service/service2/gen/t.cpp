#include <exodus/program.h>
programinit()

#include <win.h>
#include <initgeneral.h>
#include <readagp.h>

function main()

{
	printl("t says 'Hello World!'");

	var xx=RECORD.a(2);

	//CREATE LABELLED COMMON
	mv.labelledcommon[1]=new win_common;
	PSEUDO="";
	SYSTEM="";
	//initialise or abort
	if (not perform("initgeneral")){
		return 1;
	}
	call readagp();
	return 0;
}

programexit()

