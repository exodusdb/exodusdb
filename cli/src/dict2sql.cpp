#include <exodus/program.h>
programinit()

var dictfilename;
var dictfile;

function main() {

	dictfilename=field(SENTENCE," ",2).lcase();
	if (dictfilename.substr(1,5) ne "dict_")
		dictfilename.splicer(1,0,"dict_");
	if (!open(dictfilename,dictfile)) {
		call fsmsg();
		stop();
	}

	var dictid=field(SENTENCE," ",3);
	if (dictid)
		makelist("",dictid);
	else
		select(dictfilename);

	while (readnext(dictid)) {
		doone(dictid);
	}

	return 0;
}

subroutine doone(io dictid) {

	//get the dict record
	var dictrec;
	if (!dictrec.read(dictfile,dictid)) {
		dictid.ucaser();
		if (!dictrec.read(dictfile,dictid))
			stop(quote(dictid)^" cannot be read in "^quote(dictfilename));
	}

	//remove anything before sql code
	var pos=index(dictrec,"/" "*plsql");
	if (!pos) {
		//stop(quote(dictfilename)^" "^quote(dictid)^" does not have any plsql section");
		return;
	}
	var sql=dictrec.substr(pos+8);

	//remove anything after sql code
	pos=index(sql,"*" "/");
	sql=sql.substr(1,pos-1);

	//convert to text
	sql.trimmerf(VM).trimmerb(VM);
	sql.converter(VM,"\n");

	var plsql=R"V0G0N(
CREATE OR REPLACE FUNCTION $functionname(key text, data text)
RETURNS text AS
$$
DECLARE
$sqldeclare
BEGIN

$sqlcode

RETURN ans;
END;
$$
LANGUAGE 'plpgsql' IMMUTABLE
SECURITY DEFINER
COST 10;
	)V0G0N";

	plsql.swapper("$sqlcode",sql);

	//create declaration section
	var sqldeclare="ans text;";
	plsql.swapper("$sqldeclare",sqldeclare);

	//set the function name
	var functionname=dictfilename^"_"^dictid;
	plsql.swapper("$functionname",functionname);

	dictid.outputl();
	//plsql.outputl();

	var errmsg;
	var().sqlexec(plsql,errmsg);

	if (errmsg) {
		plsql.outputl();
		errmsg.outputl();
	}
	return;
}

programexit()
