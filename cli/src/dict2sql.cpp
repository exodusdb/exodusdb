#include <exodus/program.h>
programinit()

var verbose;
var dictfilename;
var dictfile;
var dictrec;

function main() {

	// Syntax:
	// dict2sql {filename {dictid}} {V}

	// 1. Creates pgsql functions to calculate columns given data and key
	//
	// functions like "dict_filename_DICT_ID(key,data)" returns text
	// /df
	// List of functions
	// Schema | Name | Result data type | Argument data types | Type
	// -------+---------------------------+--------------------+---------------------+--------
	// public | dict_ads_brand_and_date | text | key text, data text | normal
	// ...

	// NOTE: Multivalued pgsql dictionary functions currently should be written to calculate
	// and return ALL multivalues (as if MV=0) since currently exodus never requests postgres
	// to call a dict functions to get a specific mv.
	// If in future MV is required, then dict function arguments will probably become (key,data,mv)

	// 2. Creates dict_all file which represents all dictionary files and their items in one file
	//
	// only performed when filename is omitted
	//
	// see "listdict all"

	// 3. Creates various exodus pgsql utility functions
	//
	// only performed when filename is omitted
	//
	// /df
	// List of functions
	// Schema | Name | Result data type | Argument data types | Type
	// -------+---------------------------+--------------------+---------------------+--------
	// public | exodus_count | integer | text, text | normal
	// public | exodus_trim | text | data text
	// ...

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

	//do one file
	int nfiles=dcount(filenames,FM);
	for (int filen=1;filen<=nfiles;++filen)
		onefile(filenames.a(filen),dictid,viewsql);

	//do all files and add exodus
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

		//extra functions
		/////////////////

		var sqltemplate=
R"V0G0N(
CREATE OR REPLACE FUNCTION
 $functionname(data text)
 RETURNS text
 AS
 $$
 BEGIN
  $sqlcode
 END;
 $$
 LANGUAGE 'plpgsql'
 IMMUTABLE
 SECURITY
 DEFINER
 COST 10;
)V0G0N";

		//exodus_trim
		var trimsql=R"(return regexp_replace(regexp_replace(data, '^\s+|\s+$', '', 'g'),'\s{2,}',' ','g');)";
		do_sql("exodus_trim",trimsql,sqltemplate);

	}

	return 0;
}

subroutine onefile(in dictfilename, in reqdictid, io viewsql) {

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
		onedictid(dictfilename, dictid, reqdictid);
	}

	if (viewsql) {
		viewsql ^= "SELECT '" ^ dictfilename.substr(6).ucase() ^ "'||'*'||key as key, data\n";
		viewsql ^= "FROM " ^ dictfilename ^ "\n";
		viewsql ^= "UNION\n";
	}

	return;
}

subroutine onedictid(in dictfilename, io dictid, in reqdictid) {

	//get the dict source code
	var dictrec;
	if (not dictrec.read(dictfile,dictid)) {
		dictid.ucaser();
		if (!dictrec.read(dictfile,dictid))
			stop(quote(dictid)^" cannot be read in "^quote(dictfilename));
	}
	var sourcecode=dictrec.a(8);
	var ismv=dictrec.a(4)[1]=="M";

	//remove anything before sql code
	var pos=index(sourcecode,"/" "*pgsql");
	if (!pos) {
		if (reqdictid)
			stop(quote(dictfilename)^" "^quote(dictid)^" does not have any pgsql section");
		return;
	}
	var sql=sourcecode.substr(pos+8);

	printl(dictfilename, " ",dictid);

	//should take everything up to the end ??
	//so pgsql comments like /* */ are respected ??
	//remove anything after sql code
	//find the LAST * /  pair and remove everything after it
	var lastpos=0;
	for (int ii=1;;++ii) {
		//find the ii'th * / terminator
		pos=index(sql,"*" "/",ii);
		if (!pos)
			break;
		lastpos=pos;
	}
	if (lastpos)
		sql.splicer(lastpos-1,"");

	var xlatetemplate;
	if (ismv)
		xlatetemplate=
R"V0G0N(
$RETVAR := array_to_string
(
 array
 (
  SELECT
    $TARGET_EXPR
   FROM
    unnest
    (
     string_to_array($SOURCEKEY_EXPR,chr(29))
    )
   LEFT JOIN
    $TARGETFILE on $TARGETFILE.key = unnest
 ),
 chr(29),
 ''
);
)V0G0N";
		else
			xlatetemplate=
R"V0G0N(
$RETVAR :=
  $TARGET_EXPR
  FROM $TARGETFILE
  WHERE $TARGETFILE.key=$SOURCEKEY_EXPR;
  $RETVAR := coalesce($RETVAR,'');
)V0G0N";

	// expand lines like the following to sql
	// (all spaces are MANDATORY)
	// ans := xlate filename fromfn tofn
	// e.g.
	// xlate=xlate jobs 2 14
	// ->
	// ans:=split_part(jobs.data,chr(30),14)
	// FROM jobs
	// WHERE jobs.key=split_part($2,chr(30),2);
	//
	// fromfn and tofn can be functions
	// "from" expression has access to source file data in variable $2 (argument 2)
	// "to" expression has access to target file data eg invoices.data
	int nlines=dcount(sql,VM);
	for (int ln=1;ln<=nlines;++ln) {
		var line=sql.a(1,ln).trim();
		if (line.substr(1,2)!="--" && field(line," ",2,2) eq ":= xlate") {

			//line.outputl("xlate=");

			//eg ans
			var targetvariablename=line.field(" ",1);

			//target file name eg jobs
			var target_filename=line.field(" ",4);

			//source file field number or expression for key to target file
			var source_key_expr=line.field(" ",5);
			//if key field numeric then extract from source file date
			if (source_key_expr.isnum())
				source_key_expr="split_part($2,chr(30),"^source_key_expr^")";

			//target file field number or expression
			var target_expr=line.field(" ",6);
			if (target_expr.isnum())
				target_expr="split_part("^target_filename^".data,chr(30),"^target_expr^")";

			//line=targetvariablename^" := ";
			//if (ismv) {
			//	line^="array_to_string"
			//	"("
			//	" array"
			// " ("
			// " select ";
			//}
			//line^=target_expression;
			//line^="\n FROM "^target_filename;
			//line^="\n WHERE "^target_filename^".key="^source_key_expression^";";
			line=xlatetemplate;
			line.swapper("$RETVAR",targetvariablename);
			line.swapper("$TARGETFILE",target_filename);
			line.swapper("$SOURCEKEY_EXPR",source_key_expr);
			line.swapper("$TARGET_EXPR",target_expr);
			//line.outputl("sql=");

			sql.r(1,ln,line);
		}
	}

	//convert to text
	sql.trimmerf(VM).trimmerb(VM);
	sql.converter(VM,"\n");

	var sqltemplate=
R"V0G0N(CREATE OR REPLACE FUNCTION
 $functionname(key text, data text)
 RETURNS text
 AS
 $$
  DECLARE
   ans text;
  BEGIN
$sqlcode
  RETURN ans;
  END;
 $$
 LANGUAGE 'plpgsql'
 IMMUTABLE
 SECURITY
 DEFINER
 COST 10;
 )V0G0N";

	//set the function name
	var functionname=dictfilename^"_"^dictid;

	do_sql(functionname,sql,sqltemplate);

}

subroutine do_sql(in functionname, in sql, in sqltemplate) {

	var functionsql=sqltemplate;

	functionsql.swapper("$functionname",functionname);

	functionsql.swapper("$sqlcode",sql);

	if (verbose)
		functionsql.outputl();

	var errmsg;
	var().sqlexec(functionsql,errmsg);

	if (errmsg) {
		if (not verbose) {
			//functionsql.outputl();
			int nlines=count(functionsql,"\n");
			for (int linen=1;linen<=nlines;++linen) {
				printl(linen-2+2,". ", field(functionsql,"\n",linen) );
			}
			outputl();
		}
		errmsg.outputl();
	}
	return;
}

programexit()
