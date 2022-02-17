/*
	dict2sql - Install dictionary pgsql functions and exodus extensions

	SYNTAX

		dict2sql [EXTENSIONS] [FILENAME,...] [DICTID]

	EXTENSIONS

		Optional or can be "pgsql" or "pgexodus".

		Normally this is not required since they are installed during installation

		"pgexodus" requires postgres SUPERUSER permissions.

		"pgsql" very easy to install but maybe 25% SLOWER than using pgexodus.so c function  extension

		"pgexodus" - inconvenient due to requirement to install extension in server
		and require super user rights but 30% FASTER than using pgsql functions

	FILENAME

		Optional. If omitted then all dictionary files will be processed.

		Use comma to separate multiple files WITHOUT any spaces.

	DICTID

		If provided then only that dictionary item will be processed.

*/

/*
	time and compare the following. they are are same when using the pgexodus extension written in c

	assert(var().sqlexec("select key from ads order by exodus_extract_text(data,1,0,0)"));
	assert(var().sqlexec("select key from ads order by split_part(data, E'\x1E', 1)"));

	but the pgsql implementation of exodus_extract_text (contained in this program) is significantly slower.

*/

#include <exodus/program.h>
programinit()

var verbose;
var dictfilename;
var dictfile;
var dictrec;
var errors = "";

var install_exodus_extensions = "";

function main() {

	//TODO update comment to reflect the move to schema "dict" and allow for $EXO_DICT and exodus_dict

	// Syntax:
	// dict2sql {filename {dictid}} {V}

	// 1. Creates pgsql functions to calculate columns given data and key
	//    for each dictitem in each dictfile
	//
	// functions like "dict_filename__DICT_ID(key,data)" returns text
	// /df
	// List of functions
	// Schema | Name | Result data type | Argument data types | Type
	// -------+---------------------------+--------------------+---------------------+--------
	// public | dict_ads__brand_and_date | text | key text, data text | normal
	// ...

	// NOTE: Multivalued pgsql dictionary functions currently should be written to calculate
	// and return ALL multivalues (as if MV=0) since currently exodus never requests postgres
	// to call a dict functions to get a specific mv.
	// If in future MV is required, then dict function arguments will probably become (key,data,mv)

	// 2. Creates dict.all file which represents all dictionary files and their items in one file
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

	//establish the default connection BEFORE opening a connection to dict database
	connect();

	//for dicts if not default
	var dictconnection = "";

	var filenames = COMMAND.a(2).lcase();
	var dictid = COMMAND.a(3);
	verbose = index(OPTIONS, 'V');
	var doall = true;

	// Option to install standard exodus functions
	if (locateusing(",", filenames.a(1), "pgsql,pgexodus")) {
		install_exodus_extensions = filenames.a(1);
		filenames.remover(1);
		doall = false;
	}

	if (filenames) {
		doall = false;
		if (filenames.substr(1, 5) ne "dict.")
			filenames.splicer(1, 0, "dict.");

	} else if (doall) {
		var dictdbname = "";
		osgetenv("EXO_DICT", dictdbname);
		if (not dictdbname)
			//must be the same in mvdbpostgres.cpp and dict2sql
			dictdbname = "exodus";
		if (dictdbname) {
			if (not dictconnection.connect(dictdbname)) {
				dictdbname.quote().logputl("dict2sql: Warning: Using default database because cannot connect to ");
			}
		}
		filenames = dictconnection.listfiles();
	}

	var sqltemplate = R"V0G0N(
CREATE OR REPLACE FUNCTION
$functionname_and_args
RETURNS $return_type
AS
$$
BEGIN
$sqlcode
END;
$$
LANGUAGE 'plpgsql'
IMMUTABLE STRICT
SECURITY
DEFINER
COST 10;
)V0G0N";

	// Drop obsolete functions - ignore errors
	//var().sqlexec("DROP FUNCTION IF EXISTS exodus_extract_text2(text, int4, int4, int4);");

	// Create Natural Order Collation if installing extensions
	if (install_exodus_extensions) {
		rawsqlexec("DROP COLLATION IF EXISTS exodus_natural;");
		rawsqlexec("CREATE COLLATION exodus_natural (provider = icu, locale = 'en@colNumeric=yes', DETERMINISTIC = false);");
	}

	if (install_exodus_extensions eq "pgexodus") {

		var sqltemplate_strict =
			R"V0G0N(
CREATE OR REPLACE FUNCTION $functionname_and_args
RETURNS $return_type
AS 'pgexodus', '$functionname'
LANGUAGE 'c'
IMMUTABLE STRICT
SECURITY
DEFINER
COST 10;
)V0G0N";

		var sqltemplate = sqltemplate_strict.swap(" STRICT", "");

		create_function("exodus_extract_text(data text, fn int4, vn int4, sn int4)", "text", "", sqltemplate);
		create_function("exodus_extract_number(data text, fn int4, vn int4, sn int4)", "float8", "", sqltemplate);
		create_function("exodus_count(data text, countchar text)", "integer", "", sqltemplate);

		//create_function("exodus_extract_sort(data text, fn int4, vn int4, sn int4)", "text", "", sqltemplate);
		create_function("exodus_extract_date(data text, fn int4, vn int4, sn int4)", "date", "", sqltemplate_strict);
		create_function("exodus_extract_time(data text, fn int4, vn int4, sn int4)", "interval", "", sqltemplate_strict);
		create_function("exodus_extract_datetime(data text, fn int4, vn int4, sn int4)", "timestamp", "", sqltemplate_strict);

	} else if (install_exodus_extensions eq "pgsql") {

		//rawsqlexec("DROP FUNCTION IF EXISTS exodus_extract_sort(text, int4, int4, int4);");

		//exodus_extract_text<fn,vn,sn> returns an sql text type
		create_function("exodus_extract_text(data text, fn int, vn int, sn int)", "text", exodus_extract_text_sql, sqltemplate);

		//NOT implemented in pgsql. ::select uses COLLATE instead
		//exodus_extract_sortt<fn,vn,sn> returns an sql text type
		//create_function("exodus_extract_sort(data text, fn int, vn int, sn int)", "text", exodus_extract_text_sql, sqltemplate);

		//exodus_extract_date<fn,vn,sn> returns an sql data type
		create_function("exodus_extract_date(data text, fn int4, vn int4, sn int4)", "date", exodus_extract_date_sql, sqltemplate);

		//exodus_extract_time<fn,vn,sn> returns an sql time type
		create_function("exodus_extract_time(data text, fn int4, vn int4, sn int4)", "time", exodus_extract_time_sql, sqltemplate);

		//exodus_extract_date<fn,vn,sn> returns an sql data type
		create_function("exodus_extract_datetime(data text, fn int4, vn int4, sn int4)", "timestamp", exodus_extract_datetime_sql, sqltemplate);

		//exodus_extract_number<fn,vn,sn> returns an sql float8 type
		create_function("exodus_extract_number(data text, fn int4, vn int4, sn int4)", "float8", exodus_extract_number_sql, sqltemplate);

		//exodus_count(str,ch) returns an int
		create_function("exodus_count(data text, countchar text)", "integer", exodus_count_sql, sqltemplate);
	}

	// Various exodus pgsql utility functions
	/////////////////////////////////////////
	if (doall) {

		//create exodus pgsql functions

		//exodus_trim (leading, trailing and excess inner spaces)
		var trimsql = R"(return regexp_replace(regexp_replace(data, '^\s+|\s+$', '', 'g'),'\s{2,}',' ','g');)";
		create_function("exodus_trim(data text)", "text", trimsql, sqltemplate);

		//exodus_field_replace (a field)
		create_function("exodus_field_replace(data text, sep text, fieldno int, replacement text)", "text", field_replace_sql, sqltemplate);

		//exodus_field_remove (a field)
		create_function("exodus_field_remove(data text, sep text, fieldno int)", "text", field_remove_sql, sqltemplate);

		//exodus_split 123.45USD -> 123.45
		create_function("exodus_split(data text)", "text", split_sql, sqltemplate);

		//exodus_unique (fields)
		//https://github.com/JDBurnZ/postgresql-anyarray/blob/master/stable/anyarray_uniq.sql
		create_function("exodus_unique(mvstring text, sepchar text)", "text", unique_sql, sqltemplate);

		//exodus_locate -> int
		create_function("exodus_locate(substr text, searchstr text, sepchar text default VM)", "int", locate_sql, sqltemplate);

		//exodus_isnum -> bool
		create_function("exodus_isnum(instring text)", "bool", isnum_sql, sqltemplate);

		//exodus_tobool(text) -> bool
		create_function("exodus_tobool(instring text)", "bool", text_tobool_sql, sqltemplate);

		//exodus_tobool(numeric) -> bool
		create_function("exodus_tobool(innum numeric)", "bool", numeric_tobool_sql, sqltemplate);

		//exodus_date -> int (today's date as a number according to pickos)
		create_function("exodus_date()", "int", exodus_todays_date_sql, sqltemplate);

		//exodus_extract_date_array -> date[]
		create_function("exodus_extract_date_array(data text, fn int, vn int, sn int)", "date[]", exodus_extract_date_array_sql, sqltemplate);

		//return time as interval which can handle times like 25:00
		//exodus_extract_time_array -> time[]
		//create_function("exodus_extract_time_array(data text, fn int, vn int, sn int)", "time[]", exodus_extract_time_array_sql, sqltemplate);
		//exodus_extract_time_array -> time[]
		create_function("exodus_extract_time_array(data text, fn int, vn int, sn int)", "interval[]", exodus_extract_time_array_sql, sqltemplate);

		//exodus_addcent4 -> text
		create_function("exodus_addcent4(data text)", "text", exodus_addcent4_sql, sqltemplate);
	}

	//create global view of all dicts in "dict.all"
	var viewsql = "";
	if (doall)
		viewsql ^= "CREATE MATERIALIZED VIEW dict.all AS\n";

	// ========================
	// Do one or many/all files
	// ========================
	for (var filename : filenames)
		onefile(filename, dictid, viewsql);

	if (doall) {
		//ignore error if doesnt exist
		if (not dictconnection.sqlexec("DROP MATERIALIZED VIEW IF EXISTS dict.all"))
			rawsqlexec("DROP VIEW IF EXISTS dict.all");

		if (filenames.index(FM)) {
			viewsql.splicer(-6, 6, "");	 //remove trailing "UNION" word
			var errmsg;
			if (verbose)
				viewsql.output("SQL:");
			if (dictconnection.sqlexec(viewsql, errmsg))
				printl("dict.all file created");
			else {
				if (not verbose)
					viewsql.outputl("SQL:");
				errmsg.outputl("Error:");
			}
		}
	}

	if (errors)
		errors.errputl("\nErrors: ");

	return errors ne "";
}

subroutine replace_FM_etc(io sql) {
	sql.replacer(R"(\bRM\b)", R"(E'\\x1F')");
	sql.replacer(R"(\bFM\b)", R"(E'\\x1E')");
	sql.replacer(R"(\bVM\b)", R"(E'\\x1D')");
	sql.replacer(R"(\bSM\b)", R"(E'\\x1C')");
	sql.replacer(R"(\bSVM\b)", R"(E'\\x1C')");
	sql.replacer(R"(\bTM\b)", R"(E'\\x1B')");
	sql.replacer(R"(\bSTM\b)", R"(E'\\x1A')");
}

subroutine create_function(in functionname_and_args, in return_sqltype, in sql, in sqltemplate) {

	printl(functionname_and_args, " -> ", return_sqltype);

	var functionsql = sqltemplate;

	functionsql.swapper("$functionname_and_args", functionname_and_args);
	functionsql.swapper("$functionname", functionname_and_args.field("(", 1));
	functionsql.swapper("$return_type", return_sqltype);

	functionsql.swapper("$sqlcode", sql);

	replace_FM_etc(functionsql);

	if (verbose)
		functionsql.outputl();

	//decide if reindex is required - only if function has changed
	var reindexrequired = false;
	var oldfunction;
	var functionname = field(functionname_and_args, "(", 1).lcase();
	var().sqlexec("select routine_definition from information_schema.routines where routine_name = '" ^ functionname ^ "'", oldfunction);
	oldfunction.substrer(oldfunction.index("\n") + 1);
	if (oldfunction and not functionsql.index(oldfunction)) {
		reindexrequired = true;
		//TRACE(functionsql)
		//TRACE(oldfunction)
	}

	//create the function
	var errmsg;
	//supposedly this is on the default connection
	rawsqlexec(functionsql, errmsg);

	//do drop function first if suggested
	if (errmsg.index("DROP FUNCTION")) {

		var dropsql = "drop function " ^ functionname_and_args;
		if (verbose)
			dropsql.outputl();

		reindexrequired = true;

		rawsqlexec(dropsql, errmsg);
		errmsg.outputl();

		rawsqlexec(functionsql, errmsg);
	}

	if (errmsg) {

		oswrite(functionsql, "dict2sql.err");

		errors ^= functionname_and_args.field("(", 1) ^ " ";

		//		if (not verbose) {
		//			//functionsql.outputl();
		//			int nlines = count(functionsql, "\n");
		//			for (int linen = 1; linen <= nlines; ++linen) {
		//				errputl(linen - 2 + 2, ". ", field(functionsql, "\n", linen));
		//			}
		//			errputl();
		//		}
		errmsg.errputl();
	}
	if (reindexrequired) {
		//drop any index using the previous function
		//TODO identify file/fields like production_orders_date_time
		//var filename=functionname_and_args.field("_",2);
		//var fieldname=functionname_and_args.field("_",3,99).field("(",1);
		var filename = functionname_and_args.field("_", 2, 999).field("(", 1);
		var fieldname = filename.convert(LOWERCASE, "").trim("_");
		filename = filename.convert(UPPERCASE, "").trim("_");
		if (filename.listindexes(filename, fieldname)) {
			logputl("Deleting index " ^ filename ^ " " ^ fieldname);
			filename.deleteindex(fieldname);
			logputl("Creating index " ^ filename ^ " " ^ fieldname);
			filename.createindex(fieldname);
		}
	}

	return;
}

subroutine onefile(in dictfilename, in reqdictid, io viewsql) {

	if (dictfilename.substr(1, 5) ne "dict." or dictfilename == "dict.all")
		return;

	if (!open(dictfilename, dictfile)) {
		call fsmsg();
		abort("dict2sql cannot open " ^ dictfilename);
	}

	if (reqdictid)
		makelist("", reqdictid);
	else
		select(dictfilename);

	var dictid;
	while (readnext(dictid)) {
		onedictid(dictfilename, dictid, reqdictid);
	}

	//add one file for the dict_all sql using sql UNION
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
	if (not dictrec.read(dictfile, dictid)) {
		dictid.ucaser();
		if (!dictrec.read(dictfile, dictid))
			stop(quote(dictid) ^ " cannot be read in " ^ quote(dictfilename));
	}
	var sourcecode = dictrec.a(8);
	var ismv = dictrec.a(4)[1] == "M";

	//dict returns text, date, integer or float
	var dict_returns = "text";
	var conversion = dictrec.a(7);
	if (conversion.substr(1, 6) == "[DATE]" || conversion.substr(1, 6) == "[DATE," || conversion.substr(1, 6) == "[DATE2")
		dict_returns = "date";
	else if (conversion.substr(1, 9) == "[DATETIME")
		dict_returns = "timestamp";
	if (conversion.substr(1, 5) == "[TIME")
		//dict_returns = "time";
		dict_returns = "interval";
	else if (conversion.substr(1, 7) == "[NUMBER") {
		if (conversion[9] == "0")
			//[NUMBER,0]
			dict_returns = "integer";
		else
			dict_returns = "float";
	}

	//auto generate pgsql code for ..._XREF dict records (full text)
	if (sourcecode.substr(1, 11) == "CALL XREF({") {

		//remove any existing pgsql
		var pos = index(sourcecode,
						"/"
						"*pgsql");
		if (pos) {
			sourcecode = sourcecode.substr(1, pos - 1).trimb(VM);
		}

		var fulltext_dictid = field(field(sourcecode, "{", 2), "}", 1);

		//replace all punctuation and delimiters with spaces
		var chars = field(sourcecode.trim("\\"), "\\", 2);
		chars.swapper("FF", "");
		chars.swapper("FE", "");
		chars.swapper("FD", "");
		chars.swapper("FC", "");
		chars.swapper("FB", "");
		chars.swapper("FA", "");
		chars = "\\x1D\\x1E" ^ iconv(chars, "HEX");
		chars.swapper("'", "''");
		chars ^= "\\x1A";  //STM
		chars ^= "\\x1B";  //TM
		chars ^= "\\x1C";  //SVM
		chars ^= "\\x1F";  //RM

		sourcecode.r(1, -1,
					 "/"
					 "*pgsql");
		//note postgres string prefix E'...'
		// E is required to enable \xFF hex decoding
		//LIMIT TO 1000 characters since postgres index limit is around 2700 BYTES
		sourcecode(1, -1) = "ans:=upper(translate(substring(" ^ dictfilename.convert(".", "_") ^ "_" ^ fulltext_dictid.lcase() ^ "(key,data),0,1000)" ^ ",E'" ^ chars ^ "'" ^ ",repeat(' '," ^ (len(chars) + 20) ^ ")));";
		sourcecode.r(1, -1,
					 "*"
					 "/");
		dictrec(8) = sourcecode;

		//write the sql to the dictionary record so the availablily of pgsql is visible to var::selectx
		dictrec.write(dictfile, dictid);
	}

	//remove anything before sql code
	var pos = index(sourcecode,
					"/"
					"*pgsql");
	if (!pos) {
		if (index(sourcecode,
				  "/"
				  "*psql"))
			abort(quote(dictfilename) ^ " " ^ quote(dictid) ^ " 'psql' must be 'pgsql'");
		if (reqdictid)
			abort(quote(dictfilename) ^ " " ^ quote(dictid) ^ " does not have any pgsql section");

		return;
	}
	var sql = sourcecode.substr(pos + 8);

	//printl(dictfilename, " ",dictid, " > sql");

	//should take everything up to the end ??
	//so pgsql comments like /* */ are respected ??
	//remove anything after sql code
	//find the LAST * /  pair and remove everything after it
	var lastpos = 0;
	for (int ii = 1;; ++ii) {
		//find the ii'th * / terminator
		pos = index(sql,
					"*"
					"/",
					ii);
		if (!pos)
			break;
		lastpos = pos;
	}
	if (lastpos)
		sql.splicer(lastpos - 1, "");

	var xlatetemplate;
	if (ismv)
		xlatetemplate = R"V0G0N(
-- $COMMENT
$RETVAR := array_to_string
(
 array
 (
  SELECT
    $TARGET_EXPR
   FROM
    unnest
    (
     string_to_array($SOURCEKEY_EXPR,E'\x1D')
    )
   LEFT JOIN
    $TARGETFILE on $TARGETFILE.key = unnest
 ),
 E'\x1D',
 ''
);
)V0G0N";
	else
		xlatetemplate =
			R"V0G0N(
 --$COMMENT
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
	// ans:=split_part(jobs.data,E'\x1E',14)
	// FROM jobs
	// WHERE jobs.key=split_part($2,E'\x1E',2);
	//
	// fromfn and tofn can be functions
	// "from" expression has access to source file data in variable $2 (argument 2)
	// "to" expression has access to target file data eg invoices.data
	//      and '' means whole record eg invoices.data
	//	and "0" means key (which is not useful)
	int nlines = dcount(sql, VM);
	for (int ln = 1; ln <= nlines; ++ln) {
		var line = sql.a(1, ln).trim();
		if (line.substr(1, 2) != "--" && field(line, " ", 2, 2) eq ":= xlate") {

			//line.printl("xlate=");

			//eg ans
			var targetvariablename = line.field(" ", 1);

			//target file name eg jobs
			var target_filename = line.field(" ", 4);
			var source_key_expr = line.field(" ", 5);
			var target_expr = line.field(" ", 6).field(";", 1);
			//TRACE(dictid)
			//TRACE(reqdictid)
			//allow xlate job in jobs_text because it is for dict_production_orders section
			//if (lcase(var("dict.") ^ target_filename) == dictfilename) {
			//	errputl("> ", dictfilename, " ", dictid.quote(), " possible bad xlate");
			//}
			//allowing xlate jobs in dict_jobs text since it is used for other files
			if (lcase(var("dict.") ^ target_filename) eq dictfilename && not(target_filename.lcase() == "jobs" && dictid.lcase() == "text")) {
				line = " -- Sorry. In " ^ target_filename ^ ", " ^ dictid ^ " you cannot xlate to same file due to pgsql bug.\n -- " ^ line;
			} else {
				//source file field number or expression for key to target file
				//if key field numeric then extract from source file date
				if (source_key_expr.isnum())
					source_key_expr = "split_part($2,E'\\x1E'," ^ source_key_expr ^ ")";

				//target file field number or expression (omit optional ; on the end)
				if (target_expr == 0)
					target_expr = target_filename ^ ".key";
				else if (target_expr == "''")
					target_expr = target_filename ^ ".data";
				else if (target_expr.isnum())
					target_expr = "split_part(" ^ target_filename ^ ".data,E'\\x1E'," ^ target_expr ^ ")";

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
				var origline = line;
				line = xlatetemplate;
				line.swapper("$COMMENT", origline);
				line.swapper("$RETVAR", targetvariablename);
				line.swapper("$TARGETFILE", target_filename);
				line.swapper("$SOURCEKEY_EXPR", source_key_expr);
				line.swapper("$TARGET_EXPR", target_expr);
				//line.outputl("sql=");
			}
			sql(1, ln) = line;
		}
	}

	replace_FM_etc(sql);

	//convert to text
	sql.trimmerf(VM).trimmerb(VM);
	sql.converter(VM, "\n");

	var sqltemplate =
		R"V0G0N(CREATE OR REPLACE FUNCTION
 $functionname_and_args
 RETURNS $return_type
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

	//upload pgsql function to postgres
	create_function(dictfilename.convert(".", "_") ^ "_" ^ dictid ^ "(key text, data text)", dict_returns, sql, sqltemplate);

	// delete calc_fields

	var calc_fields;
	if (calc_fields.open("calc_fields")) {
		var key = ucase(dictfilename.substr(6) ^ "*" ^ dictid);
		if (calc_fields.deleterecord(key)) {
			key.outputl("deleted calc_fields = ");
		}
	}

}  //onedictid

//exodus_field_remove

var field_remove_sql = R"V0G0N(
DECLARE
 charn int;
 nchars int;
 currfieldn int;
 ans text;

BEGIN

 -- SIMILAR CODE IN FIELD_REMOVE AND FIELD_REPLACE

 if fieldno<=0 then
  if fieldno=0 then
   return '';
  end if;
  return data;
 end if;

 ans := '';
 currfieldn :=1;
 nchars := length(data);
 for charn in 1..nchars loop

  continue when substr(data,charn,1) <> sep;

  --if substr(data,charn,1) = sep then

   currfieldn=currfieldn+1;

  if currfieldn=fieldno then
   ans := substr(data,1,charn-1);

  elseif currfieldn>fieldno then
   if ans<>'' then
    ans := ans || sep || substr(data,charn+1);
   else
    ans := substr(data,charn+1);
   end if;
   return ans;
  end if;

  --end if;

 end loop;

 -- deleting beyond the number of existing fields
 if currfieldn<fieldno then
  return data;
  end if;

 return ans;

END;
)V0G0N";

//exodus_field_replace

var field_replace_sql = R"V0G0N(
DECLARE
 charn int;
 nchars int;
 currfieldn int;
 ans text;

BEGIN

 -- SIMILAR CODE IN FIELD_REMOVE AND FIELD_REPLACE

 if fieldno<=0 then
  if fieldno=0 then
   return replacement;
  end if;
  if data='' then
   return replacement;
  else
   return data || sep || replacement;
  end if;

 end if;

 ans := '';
 currfieldn :=1;
 nchars := length(data);
 for charn in 1..nchars loop

  continue when substr(data,charn,1) <> sep;
  --if substr(data,charn,1) = sep then

  currfieldn=currfieldn+1;

  if currfieldn=fieldno then
   ans := substr(data,1,charn-1);

  elseif currfieldn>fieldno then
   if ans<>'' then
    ans := ans || sep || replacement || sep || substr(data,charn+1);
   else
    ans := replacement || sep || substr(data,charn+1);
   end if;
   return ans;
  end if;

  --end if;

 end loop;

 -- deleting beyond the number of existing fields
 if currfieldn<fieldno then
   if replacement<>'' then
    data := data || repeat(sep,fieldno-currfieldn)|| replacement;
   end if;
  return data;
  end if;

 if replacement<>'' then
  ans := ans || sep || replacement;
  end if;

 return ans;

END;
)V0G0N";

//exodus_split

var split_sql = R"V0G0N(
DECLARE
 inputx text;
 temp text;
 numlen int;
 numx text;
 unitx text;
 nn int;
 tt int;
 char1 char(1);
BEGIN
   inputx:=data;
   inputx:=translate(inputx,' ','');
   temp:=inputx;
   char1:=substring(temp,1,1);
   if char1='-' then
    temp=substring(temp,2);
   end if;
   temp:=translate(temp,'123456789.,','           ');

   numlen=length(temp)-length(trim(leading from temp));
   if char1='-' then
    numlen := numlen+1;
    end if;
   numx:=substring(inputx,1,numlen);

   if numx='-' then
    numx:='';
   end if;

   -- unused (function arg output var)
   unitx=substring(inputx,numlen+1,99);

   -- convert to decimal format
   -- remove all , '.' besides last
   numx=translate(numx, ',' , '.');
   nn=exodus_count(numx,'.');
   for ii in 1..nn-1 loop
    tt=strpos(numx,'.');
    if tt<>0 then
     numx:=substring(numx,1,tt-1) || substring(numx,tt+1);
    end if;
   end loop;

   return numx || ' ' || unitx;

END;

)V0G0N";

//exodus_unique

var unique_sql = R"V0G0N(
DECLARE
	-- The variable used to track iteration over "with_array".
	loop_offset integer;

	with_array text[];

	-- Working array to be returned as text
	return_array text[] := '{}';

	value text;

BEGIN
	IF length(mvstring)=0 or mvstring is NULL THEN
		return '';
	END IF;

	with_array := string_to_array(mvstring,sepchar);

	IF ARRAY_LOWER(with_array, 1) = 0 THEN
		return '';
	END IF;

	-- Iterate over each element in "concat_array".
	FOR loop_offset IN ARRAY_LOWER(with_array, 1)..ARRAY_UPPER(with_array, 1) LOOP
		value := with_array[loop_offset];
		IF length(value)>0 THEN
			IF not return_array @> array[value] THEN
				return_array = ARRAY_APPEND(return_array, value);
			END IF;
		END IF;
	END LOOP;

RETURN array_to_string(return_array,sepchar,'');
END;
)V0G0N";

//exodus_locate

var locate_sql = R"V0G0N(
DECLARE
 searchstrarray text [];
 nfields int;
 fieldn int;
BEGIN

 -- empty target "finds" field zero
 if searchstr='' then
  return 0;
 end if;

 nfields := exodus_count(searchstr,sepchar)+1;
 searchstrarray := string_to_array(searchstr,sepchar);

 for fieldn in 1..nfields loop
 if searchstrarray[fieldn]=substr then
   return fieldn;
  end if;

 end loop;

 -- zero indicates not found
 -- (instead of normal locate which returns one after the last element)
 return 0;

END;
)V0G0N";

//exodus_isnum -> bool

var isnum_sql = R"V0G0N(
DECLARE
 tt numeric;
BEGIN

 if position(' ' in $1)::bool then
  return FALSE;
 end if;

 -- note that pickos's '', is numeric unlike pgsql
 if $1='' then
  return TRUE;
 end if;

tt = $1::NUMERIC;
 RETURN TRUE;
EXCEPTION WHEN others THEN
 RETURN FALSE;

END;
)V0G0N";

//exodus_tobool -> bool

var text_tobool_sql = R"V0G0N(
DECLARE
 tt numeric;
BEGIN

 -- note that pickos's '', is numeric 0 and false unlike pgsql
 if $1='' then
  return FALSE;
 end if;

 -- spaces are not numeric in pickos therefore are true
 if position(' ' in $1)::bool then
  return TRUE;
 end if;

-- if numeric then zero is false, and anything else is true
tt = $1::NUMERIC;
 RETURN tt<>0;

-- if not numeric (and not empty string) then is true
EXCEPTION WHEN others THEN
 RETURN TRUE;

END;
)V0G0N";

//exodus_tobool(numeric) -> bool

var numeric_tobool_sql = R"V0G0N(
BEGIN
 return $1 != 0;
END;
)V0G0N";

//exodus_date -> int
var exodus_todays_date_sql =
	R"V0G0N(
 return current_date-'1968-1-1'::date;
)V0G0N";

//exodus_extract_date_array -> date[]
//almost identical code in exodus_extract_time_array

var exodus_extract_date_array_sql = R"V0G0N(
DECLARE
 dates text;
 date text;
 ndates int;
 date_array date[];
BEGIN

 if fn < 1 then
  dates := data;
 else
  dates := split_part(data, E'\x1E', fn);
 end if;

 ndates := exodus_count(dates,VM)+(dates!='')::int;
 for daten in 1..ndates loop
  date := split_part(dates,VM,daten);
  if date = '' then
   date_array[daten] := NULL;
  else
   date_array[daten] := '1967/12/31'::date + date::int;
  end if;
 end loop;
 return date_array;
END;
)V0G0N";

//exodus_extract_time_array -> interval[]
//almost identical code in exodus_extract_date_array

var exodus_extract_time_array_sql = R"V0G0N(
DECLARE
 times text;
 timex text;
 ntimes int;
 time_array interval[];
BEGIN
 if fn < 1 then
  times := data;
 else
  times := split_part(data, E'\x1E', fn);
 end if;

 ntimes := exodus_count(times,VM)+(times!='')::int;
 for timen in 1..ntimes loop
  timex := split_part(times,VM,timen);
  if timex = '' then
   time_array[timen] := NULL;
  else
   timex := timex || ' seconds';
   time_array[timen] := '00:00'::time + timex::interval;
  end if;
 end loop;
 return time_array;
END;
)V0G0N";

//exodus_addcent4 -> text

var exodus_addcent4_sql = R"V0G0N(
DECLARE
 year int;
BEGIN
 year = $1::NUMERIC;
 if year < 50 then
  return '20' || $1;
 elsif year < 100 then
  return '19' || $1;
 end if;
 return $1;
EXCEPTION WHEN others THEN
 RETURN $1;
END;
)V0G0N";

//exodus_extract_text -> text

var exodus_extract_text_sql = R"V0G0N(
 if fn < 1 then
  return data;
 end if;

 if vn < 1 then
  return split_part(data, E'\x1E', fn);
 end if;

 if sn < 1 then
  return split_part(split_part(data, E'\x1E', fn), E'\x1D', vn);
 end if;

  return split_part(split_part(split_part(data, E'\x1E', fn), E'\x1D', vn), E'\x1C', sn);
)V0G0N";

//exodus_extract_date -> date SIMILAR CODE in extract_number, extract_date and extract_time

var exodus_extract_date_sql = R"V0G0N(
DECLARE
 ans text;
BEGIN

 if fn < 1 then
  return case when data = '' then NULL else '1967/12/31'::date + data::int end;
 end if;

 ans = split_part(data, E'\x1E', fn);

 if vn < 1 then
  return case when ans = '' then NULL else '1967/12/31'::date + ans::int end;
 end if;

 ans = split_part(ans, E'\x1D', vn);

 if sn < 1 then
  return case when ans = '' then NULL else '1967/12/31'::date + ans::int end;
 end if;

 ans = split_part(ans, E'\x1C', sn);

 return case when ans = '' then NULL else '1967/12/31'::date + ans::int end;

END;
)V0G0N";

//exodus_extract_time -> interval SIMILAR CODE in extract_number, extract_date and extract_time

var exodus_extract_time_sql = R"V0G0N(
DECLARE
 ans text;
BEGIN

 if fn < 1 then
  return case when text = '' then NULL else make_interval(secs => data::int) end;
 end if;

 ans = split_part(data, E'\x1E', fn);

 if vn < 1 then
  return case when ans = '' then NULL else make_interval(secs => ans::int) end;
 end if;

 ans = split_part(ans, E'\x1D', vn);

 if sn < 1 then
  return case when ans = '' then NULL else make_interval(secs => ans::int) end;
 end if;

 ans = split_part(ans, E'\x1C', sn);

 return case when ans = '' then NULL else make_interval(secs => ans::int) end;

END;
)V0G0N";

//exodus_extract_datetime -> timestamp SIMILAR CODE in extract_number, extract_date and extract_time

var exodus_extract_datetime_sql = R"V0G0N(
DECLARE
 ans text;
BEGIN

 if fn < 1 then
  ans := data;
 else
  ans = split_part(data, E'\x1E', fn);
  if vn >= 1 then
   ans := split_part(ans, E'\x1D', vn);
   if sn >= 1 then
    ans := split_part(ans, E'\x1C', sn);
   end if;
  end if;
 end if;

 if ans = '' then
  return NULL;
 else
  return to_timestamp((split_part(ans,'.',1)::int-732)*86400 + split_part(ans,'.',2)::int);
 end if;

END;
)V0G0N";

//exodus_extract_number -> number SIMILAR CODE in extract_number, extract_date and extract_time

var exodus_extract_number_sql = R"V0G0N(
DECLARE
 ans text;
BEGIN

 if fn < 1 then
  return case when text = '' then 0 else text::float8 end;
 end if;

 ans = split_part(data, E'\x1E', fn);

 if vn < 1 then
  return case when ans = '' then 0 else ans::float8 end;
 end if;

 ans = split_part(ans, E'\x1D', vn);

 if sn < 1 then
  return case when ans = '' then 0 else ans::float8  end;
 end if;

 ans = split_part(ans, E'\x1C', sn);

 return case when ans = '' then 0 else ans::float8 end;

END;
)V0G0N";

//exodus_count

var exodus_count_sql = R"V0G0N(
BEGIN

 return (CHAR_LENGTH(data) - CHAR_LENGTH(REPLACE(data, ch, '')));

END;
)V0G0N";

subroutine rawsqlexec(in sql) {
	printl(sql.field("\n",3));
	var errormsg;
	if (not var().sqlexec(sql, errormsg)) {
		errors ^= "\n" ^ errormsg;
	}
	return;
}

subroutine rawsqlexec(in sql, out errormsg) {
	printl(sql.field("\n",3));
	var().sqlexec(sql, errormsg);
	return;
}

programexit()
