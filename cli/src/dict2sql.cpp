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
	//    for each dictitem in each dictfile
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

	var filenames = COMMAND.a(2).lcase();
	var dictid = COMMAND.a(3);
	verbose = index(OPTIONS, 'V');
	var doall = true;

	if (filenames) {
		doall = false;
		if (filenames.substr(1, 5) ne "dict_")
			filenames.splicer(1, 0, "dict_");
	} else
		filenames = var().listfiles();

	//create global view of all dicts in "dict_all"
	var viewsql = "";
	if (doall)
		viewsql ^= "CREATE MATERIALIZED VIEW dict_all AS\n";

	//do one or many/all files
	int nfiles = dcount(filenames, FM);
	for (int filen = 1; filen <= nfiles; ++filen)
		onefile(filenames.a(filen), dictid, viewsql);

	//quit if not doing all files
	/////////////////////////////
	if (!doall)
		return 0;

	var().sqlexec("CREATE OR REPLACE FUNCTION exodus_extract_date(text, int4, int4, int4)     RETURNS date      AS 'pgexodus', 'exodus_extract_date'     LANGUAGE C IMMUTABLE STRICT;");

	var().sqlexec("DROP FUNCTION IF EXISTS exodus_extract_time_array(data text, fn int, vn int, sn int);");
	//create dict_all file

	//ignore error if doesnt exist
	if (not var().sqlexec("DROP MATERIALIZED VIEW dict_all"))
		var().sqlexec("DROP VIEW dict_all");

	if (nfiles) {
		viewsql.splicer(-6, 6, "");	 //remove trailing "UNION" word
		var errmsg;
		if (verbose)
			viewsql.output("SQL:");
		if (var().sqlexec(viewsql, errmsg))
			printl("dict_all file created");
		else {
			if (not verbose)
				viewsql.outputl("SQL:");
			errmsg.outputl("Error:");
		}
	}

	//create exodus pgsql functions

	var sqltemplate =
		R"V0G0N(
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
IMMUTABLE
SECURITY
DEFINER
COST 10;
)V0G0N";

	//exodus_trim (leading, trailing and excess inner spaces)
	var trimsql = R"(return regexp_replace(regexp_replace(data, '^\s+|\s+$', '', 'g'),'\s{2,}',' ','g');)";
	do_sql("exodus_trim(data text)", "text", trimsql, sqltemplate);

	//exodus_field_replace (a field)
	do_sql("exodus_field_replace(data text, sep text, fieldno int, replacement text)", "text", field_replace_sql, sqltemplate);

	//exodus_field_remove (a field)
	do_sql("exodus_field_remove(data text, sep text, fieldno int)", "text", field_remove_sql, sqltemplate);

	//exodus_split 123.45USD -> 123.45
	do_sql("exodus_split(data text)", "text", split_sql, sqltemplate);

	//exodus_unique (fields)
	//https://github.com/JDBurnZ/postgresql-anyarray/blob/master/stable/anyarray_uniq.sql
	do_sql("exodus_unique(mvstring text, sepchar text)", "text", unique_sql, sqltemplate);

	//exodus_locate -> int
	do_sql("exodus_locate(substr text, searchstr text, sepchar text default VM)", "int", locate_sql, sqltemplate);

	//exodus_isnum -> bool
	do_sql("exodus_isnum(instring text)", "bool", isnum_sql, sqltemplate);

	//exodus_tobool -> bool
	do_sql("exodus_tobool(instring text)", "bool", tobool_sql, sqltemplate);

	//exodus_date -> int (today's date as a number according to pickos)
	do_sql("exodus_date()", "int", exodus_todays_date_sql, sqltemplate);

	//exodus_extract_date_array -> date[]
	do_sql("exodus_extract_date_array(data text, fn int, vn int, sn int)", "date[]", exodus_extract_date_array_sql, sqltemplate);

	//return time as interval which can handle times like 25:00
	//exodus_extract_time_array -> time[]
	//do_sql("exodus_extract_time_array(data text, fn int, vn int, sn int)", "time[]", exodus_extract_time_array_sql, sqltemplate);
	//exodus_extract_time_array -> time[]
	do_sql("exodus_extract_time_array(data text, fn int, vn int, sn int)", "interval[]", exodus_extract_time_array_sql, sqltemplate);

	//exodus_addcent4 -> text
	do_sql("exodus_addcent4(data text)", "text", exodus_addcent4_sql, sqltemplate);

	return 0;
}

subroutine do_sql(in functionname_and_args, in return_sqltype, in sql, in sqltemplate) {

	printl(functionname_and_args, " -> ", return_sqltype);

	var functionsql = sqltemplate;

	functionsql.swapper("$functionname_and_args", functionname_and_args);
	functionsql.swapper("$return_type", return_sqltype);

	functionsql.swapper("$sqlcode", sql);

	functionsql.replacer("\\bRM\\b", "chr(31)");
	functionsql.replacer("\\bFM\\b", "chr(30)");
	functionsql.replacer("\\bVM\\b", "chr(29)");
	functionsql.replacer("\\bSM\\b", "chr(28)");
	functionsql.replacer("\\bSVM\\b", "chr(28)");
	functionsql.replacer("\\bTM\\b", "chr(27)");
	functionsql.replacer("\\bSTM\\b", "chr(26)");

	if (verbose)
		functionsql.outputl();

	var errmsg;
	var().sqlexec(functionsql, errmsg);

	//do drop function first if suggested
	if (errmsg.index("DROP FUNCTION")) {

		var dropsql = "drop function " ^ functionname_and_args;
		if (verbose)
			dropsql.outputl();

		//drop any index using the previous function
		//TODO identify file/fields like production_orders_date_time
		//var filename=functionname_and_args.field("_",2);
		//var fieldname=functionname_and_args.field("_",3,99).field("(",1);
		var filename = functionname_and_args.field("_", 2, 999).field("(", 1);
		var fieldname = filename.convert(LOWERCASE, "").trim("_");
		filename = filename.convert(UPPERCASE, "").trim("_");
		filename.deleteindex(fieldname);

		var().sqlexec(dropsql, errmsg);
		errmsg.outputl();

		var().sqlexec(functionsql, errmsg);
	}

	if (errmsg) {
		if (not verbose) {
			//functionsql.outputl();
			int nlines = count(functionsql, "\n");
			for (int linen = 1; linen <= nlines; ++linen) {
				printl(linen - 2 + 2, ". ", field(functionsql, "\n", linen));
			}
			printl();
		}
		errmsg.outputl();
	}
	return;
}

subroutine onefile(in dictfilename, in reqdictid, io viewsql) {

	if (dictfilename.substr(1, 5) ne "dict_" or dictfilename == "dict_all")
		return;

	if (!open(dictfilename, dictfile)) {
		call fsmsg();
		abort("onefile");
	}

	if (reqdictid)
		makelist("", reqdictid);
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
	if (conversion.substr(1, 6) == "[DATE," || conversion.substr(1, 6) == "[DATE2")
		dict_returns = "date";
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
		sourcecode.r(1, -1, "ans:=upper(translate(substring(" ^ dictfilename.convert(".", "_") ^ "_" ^ fulltext_dictid ^ "(key,data),0,1000)" ^ ",E'" ^ chars ^ "'" ^ ",repeat(' '," ^ (len(chars) + 20) ^ ")));");
		sourcecode.r(1, -1,
					 "*"
					 "/");
		dictrec.r(8, sourcecode);

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
		xlatetemplate =
			R"V0G0N(
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
	// ans:=split_part(jobs.data,chr(30),14)
	// FROM jobs
	// WHERE jobs.key=split_part($2,chr(30),2);
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
			//if (lcase(var("dict_") ^ target_filename) == dictfilename) {
			//	errputl("> ", dictfilename, " ", dictid.quote(), " possible bad xlate");
			//}
			//allowing xlate jobs in dict_jobs text since it is used for other files
			if (lcase(var("dict_") ^ target_filename) eq dictfilename && not(target_filename.lcase() == "jobs" && dictid.lcase() == "text")) {
				line = " -- Sorry. In " ^ target_filename ^ ", " ^ dictid ^ " you cannot xlate to same file due to pgsql bug.\n -- " ^ line;
			} else {
				//source file field number or expression for key to target file
				//if key field numeric then extract from source file date
				if (source_key_expr.isnum())
					source_key_expr = "split_part($2,chr(30)," ^ source_key_expr ^ ")";

				//target file field number or expression (omit optional ; on the end)
				if (target_expr == 0)
					target_expr = target_filename ^ ".key";
				else if (target_expr == "''")
					target_expr = target_filename ^ ".data";
				else if (target_expr.isnum())
					target_expr = "split_part(" ^ target_filename ^ ".data,chr(30)," ^ target_expr ^ ")";

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
			sql.r(1, ln, line);
		}
	}

	sql.replacer("\\bRM\\b", "chr(31)");
	sql.replacer("\\bFM\\b", "chr(30)");
	sql.replacer("\\bVM\\b", "chr(29)");
	sql.replacer("\\bSM\\b", "chr(28)");
	sql.replacer("\\bSVM\\b", "chr(28)");
	sql.replacer("\\bTM\\b", "chr(27)");
	sql.replacer("\\bSTM\\b", "chr(26)");

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
	do_sql(dictfilename ^ "_" ^ dictid ^ "(key text, data text)", dict_returns, sql, sqltemplate);

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
var field_remove_sql =
	R"V0G0N(
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
var field_replace_sql =
	R"V0G0N(
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
var split_sql =
	R"V0G0N(
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
var unique_sql =
	R"V0G0N(
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
var locate_sql =
	R"V0G0N(
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
var isnum_sql =
	R"V0G0N(
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
var tobool_sql =
	R"V0G0N(
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

//exodus_date -> int
var exodus_todays_date_sql =
	R"V0G0N(
 return current_date-'1968-1-1'::date;
)V0G0N";

//exodus_extract_date_array -> date[]
//almost identical code in exodus_extract_time_array
var exodus_extract_date_array_sql =
	R"V0G0N(
DECLARE
 dates text := exodus_extract_text(data,fn,vn,sn);
 date text;
 ndates int := exodus_count(dates,VM)+(dates!='')::int;
 date_array date[];
BEGIN
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
var exodus_extract_time_array_sql =
	R"V0G0N(
DECLARE
 times text := exodus_extract_text(data,fn,vn,sn);
 timex text;
 ntimes int := exodus_count(times,VM)+(times!='')::int;
 time_array interval[];
BEGIN
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
var exodus_addcent4_sql =
	R"V0G0N(
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

programexit()
