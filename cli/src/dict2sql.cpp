#include <exodus/program.h>
programinit()

function main() {

	var dictfilename=field(SENTENCE," ",2).lcase();
	if (dictfilename.substr(1,5) ne "dict_")
		dictfilename.splicer(1,0,"dict_");
	var dictfile;
	if (!open(dictfilename,dictfile)) {
		call fsmsg();
		stop();
	}

	//get the dict record
	var dictid=field(SENTENCE," ",3);
	var dictrec;
	if (!dictrec.read(dictfile,dictid)) {
		dictid.ucaser();
		if (!dictrec.read(dictfile,dictid))
			stop(quote(dictid)^" cannot be read in "^quote(dictfilename));
	}

	//remove anything before sql code
	var pos=index(dictrec,"/" "*plsql");
	if (!pos)
		stop(quote(dictfilename)^" "^quote(dictid)^" does not have any plsql section");
	var sql=dictrec.substr(pos+8);

	//remove anything after sql code
	pos=index(sql,"*" "/");
	sql=sql.substr(1,pos-1);

	//convert to text
	sql.trimmerf(VM).trimmerb(VM);
	sql.converter(VM,"\n");

	var plsql=R"V0G0N(
CREATE OR REPLACE FUNCTION $functionname(key bytea, data bytea)
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

	plsql.outputl();

	var errmsg;
	var().sqlexec(plsql,errmsg);

	if (errmsg)
		errmsg.outputl();
	return 0;
}

programexit()
