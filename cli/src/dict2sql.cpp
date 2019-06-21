#include <exodus/program.h>
programinit()

var verbose;
var dictfilename;
var dictfile;
var dictrec;

function main() {

	//TODO work on more than the default db connection

	var filenames=COMMAND.a(2).lcase();
	var dictid=COMMAND.a(3);
	verbose=index(OPTIONS,'V');
	var doall = true;

	if (filenames)
	{
		doall = false;
		if (filenames.substr(1,5) ne "dict_")
			filenames.splicer(1,0,"dict_");
	}
	else
		filenames=var().listfiles();

	//create global view of all dicts in "dict_all"
	var viewsql="";
	if (doall)
		viewsql ^= "CREATE MATERIALIZED VIEW dict_all AS\n";

	int nfiles=dcount(filenames,FM);
	for (int filen=1;filen<=nfiles;++filen)
		onefile(filenames.a(filen),dictid,viewsql);

	if (doall)
	{
		//ignore error if doesnt exist
		if (not var().sqlexec("DROP MATERIALIZED VIEW dict_all"))
			var().sqlexec("DROP VIEW dict_all");

		viewsql.splicer(-6,6,"");//remove trailing "UNION" word
		var errmsg;
		if (verbose)
			viewsql.output("SQL:");
		if (var().sqlexec(viewsql,errmsg))
			printl("dict_all file created");
		else {
			if (not verbose)
				viewsql.outputl("SQL:");
			errmsg.outputl("Error:");
		}
	}

	return 0;
}

subroutine onefile(in dictfilename, in reqdictid, io viewsql)
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

	if (viewsql) {
		viewsql ^= "SELECT '" ^ dictfilename.substr(6).ucase() ^ "'||'*'||key as key, data\n";
		viewsql ^= "FROM " ^ dictfilename ^ "\n";
		viewsql ^= "UNION\n";
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
		if (not verbose)
			plsql.outputl();
		errmsg.outputl();
	}
	return;
}

programexit()
