#include <exodus/program.h>
programinit()

var verbose;
var dictfilename;
var dictfile;
var dictrec;

function main() {

	var filenames=COMMAND.a(2).lcase();
	var dictid=COMMAND.a(3);
	verbose=index(OPTIONS,'V');

	if (filenames)
	{
		if (filenames.substr(1,5) ne "dict_")
			filenames.splicer(1,0,"dict_");
	}
	else
		filenames=var().listfiles();

	int nfiles=dcount(filenames,FM);
	for (int filen=1;filen<=nfiles;++filen)
		onefile(filenames.a(filen),dictid);

	return 0;
}

subroutine onefile(in dictfilename, in reqdictid)
{

	if (dictfilename.substr(1,5) ne "dict_")
		return;

	if (!open(dictfilename,dictfile)) {
		call fsmsg();
		abort();
	}

	if (reqdictid)
		makelist("",reqdictid);
	else
		select(dictfilename);

	var dictid;
	while (readnext(dictid)) {
		onedictid(dictfilename, dictid);
	}

	return;
}

subroutine onedictid(in dictfilename, io dictid) {

	//get the dict record
	if (!dictrec.read(dictfile,dictid)) {
		dictid.ucaser();
		if (!dictrec.read(dictfile,dictid))
			stop(quote(dictid)^" cannot be read in "^quote(dictfilename));
	}

	//remove anything before sql code
	var pos=index(dictrec,"/" "*pgsql");
	if (!pos) {
		//stop(quote(dictfilename)^" "^quote(dictid)^" does not have any plsql section");
		return;
	}
	var sql=dictrec.substr(pos+8);

	printl(dictfilename, " ",dictid);

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

	if (verbose)
		plsql.outputl();

	var errmsg;
	var().sqlexec(plsql,errmsg);

	if (errmsg) {
		plsql.outputl();
		errmsg.outputl();
	}
	return;
}

programexit()
