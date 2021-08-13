#include <exodus/mvprogram.h>

//allows and *requires* coding style like exodus application programming
// e.g. must use USERNAME not mv.USERNAME
#include <exodus/exodusmacros.h>

// putting various member functions into all exodus programs allows access to the mv environment
// variable which is also available in all exodus programs.

//#include <unordered_map>

namespace exodus {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
// constructor with an mvenvironment
DLL_PUBLIC
ExodusProgramBase::ExodusProgramBase(MvEnvironment& inmv)
	: mv(inmv)
//	,
//	cache_dictid_(""),
//	cache_perform_libid_(""),
//	dict_exodusfunctorbase_(NULL){};
{
	cache_dictid_ = "";
	cache_perform_libid_ = "";
	dict_exodusfunctorbase_ = NULL;
}
#pragma GCC diagnostic pop

// destructor
DLL_PUBLIC
ExodusProgramBase::~ExodusProgramBase(){};

var ExodusProgramBase::libinfo(const var& command) {
	return var(perform_exodusfunctorbase_.libfilename(command.toString())).osfile();
}

bool ExodusProgramBase::select(const var& sortselectclause_or_filehandle) {

	//TRACE(sortselectclause_or_filehandle)

	//simple select on filehandle
	if (sortselectclause_or_filehandle.index(FM)) {
		CURSOR = sortselectclause_or_filehandle;
		return CURSOR.select();
	}

	var sortselectclause = sortselectclause_or_filehandle;

	//stage 1
	/////////

	//force default connection
	CURSOR.r(2, "");

	//indicate there are no calculated fields
	CURSOR.r(10, "");

	//perform the select (stage 1 of possibly two stages)
	//any fields requiring calculation that cannot be done by the database
	//will be put skipped and put aside in CURSOR.r(10) for stage 2

	//select or fail
	if (!CURSOR.select(sortselectclause)) {
		return false;
		/////////////
	}

	//we are done if there are no calculated fields!
	var calc_fields = CURSOR.a(10);
	if (!calc_fields) {
		return true;
		////////////
	}

	//stage 2
	/////////

	//secondary sort/select on fields that could not be calculated by the database

	//calc_fields.oswrite("calc_fields=");

	//clear the list of calculated fields
	CURSOR.r(10, "");

	//vms to fms etc in calculated fields
	calc_fields.raiser();

	//ONLY TEST MATERIALS FOR NOW
	//if (!calc_fields.ucase().index("MATERIALS"))
	//	return true;

	//debug
	//calc_fields.convert(FM^VM^SM,"   ").outputl("calc=");

	var dictfilename = calc_fields.a(5, 1);

	//debugging
	var calc_fields_file = "";
	calc_fields_file.open("calc_fields");

	//open the dictionary
	if (dictfilename.substr(1, 5).lcase() != "dict_")
		dictfilename = "dict_" ^ dictfilename;
	if (!DICT.open(dictfilename)) {
		dictfilename = "dict_voc";
		if (!DICT.open(dictfilename)) {
			throw MVDBException(dictfilename.quote() ^ " cannot be opened");
		}
	}

	//prepare to create a temporary sql table
	//DROP TABLE IF EXISTS SELECT_STAGE2;
	//CREATE TABLE SELECT_STAGE2(
	// KEY TEXT PRIMARY KEY,
	// EXECUTIVE_CODE TEXT)
	var temptablename = "SELECT_STAGE2_CURSOR_" ^ CURSOR.a(1);
	var createtablesql = "";
	createtablesql ^= "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
	//createtablesql ^= "CREATE TEMPORARY TABLE " ^ temptablename ^ "(\n";
	createtablesql ^= "CREATE TABLE " ^ temptablename ^ "(\n";
	createtablesql ^= " KEY TEXT PRIMARY KEY,\n";

	//prepare to insert sql records
	// INSERT INTO books (id, title, author_id, subject_id)
	// VALUES (41472, 'Practical PostgreSQL', 1212, 4);
	var baseinsertsql = "INSERT INTO " ^ temptablename ^ "(KEY,";

	//parse the fields for speed
	//FIRST_APPEARANCE_DATE>='9/6/2019'
	//UPLOADS=''
	//AUTHORISED<>''
	//DEADLINE<='10/6/2019'
	int nfields = calc_fields.a(1).dcount(VM);
	dim dictids(nfields);
	dim opnos(nfields);
	dim reqivalues(nfields);
	dim reqivalues2(nfields);
	dim ioconvs(nfields);
	dim sqltypes(nfields);

	for (int fieldn = 1; fieldn <= nfields; ++fieldn) {

		//dictids

		var dictid = calc_fields.a(1, fieldn);

		var dictrec;
		if (not dictrec.reado(DICT,dictid))
			dictrec="";
		ioconvs(fieldn) = dictrec.a(7);

		//add colons to the end of every calculated field in the sselect clause
		//so that 2nd stage select knows that these fields are available in the
		//temporary parallel file
		sortselectclause.replacer("\\b" ^ dictid ^ "\\b", dictid ^ ":");

		dictid.converter(".", "_");
		dictids(fieldn) = dictid;
		var sqlcolid = dictid ^ "_calc";

		//ops

		var ovalue = calc_fields.a(3, fieldn).convert(SM, VM).unquote();

		var op = calc_fields.a(2, fieldn);

		//multivalued selections are not well supported from mvdbpostgresql. handle the obvious cases"
		if (ovalue.index(VM)) {
			if (op == "=")
				op = "in";
			else if (op == "<>")
				op = "not_in";
		}

		//turn ops into numbers for speed. see c++ switch statement below
		var opno;
		if (!op)
			opno = 0;
		else if (not var("= <> > < >= <= ~ ~* !~ !~* >< >!< in not_in !! ! ] [ []").locateusing(" ", op.convert(" ", "_"), opno))
			throw MVError(op.quote() ^ " unknown op in sql select");
		opnos(fieldn) = opno;

		//reqivalues
		if (op == "in" and ovalue[1] == "(" and ovalue[-1] == ")") {
			ovalue.splicer(1, 1, "").splicer(-1, 1, "");
			ovalue.swapper("', '", VM);
			ovalue.trimmerb().trimmerf().unquoter();
			//ovalue.convert(VM,"]").outputl("ovalue=");
		}

		var ovalue2 = calc_fields.a(4, fieldn).unquote();

		//iconv
		var ivalue;
		var ivalue2;
		var ioconv = ioconvs(fieldn);
		if (ioconv) {
			ivalue = iconv(ovalue, ioconv);
			ivalue2 = iconv(ovalue2, ioconv);
		}
		else {
			ivalue = ovalue;
			ivalue2 = ovalue2;
		}

		//save reqivalues
		reqivalues(fieldn) = ivalue;
		reqivalues2(fieldn) = ivalue2;

		//sqltype
		var sqltype;
		if (ioconv.index("[NUMBER")) {
			sqltype = "NUMERIC";
		}
		else if (ioconv.index("[DATE")) {
			sqltype = "DATE";
            ioconvs(fieldn) = "D";
		}
		else if (ioconv.index("[TIME")) {
			sqltype = "TIME";//TODO check if works
		}
		else if (ioconv.index("[DATETIME")) {
			sqltype = "DATE";//TODO
		}
		else {
			sqltype = "TEXT";
		}
		sqltypes(fieldn) = sqltype;
		//TRACE(dictid^" "^ioconv^" "^sqltype)

		//sql temp table column
		createtablesql ^= " " ^ sqlcolid ^ " "^ sqltype ^ ",";

		//sql insert column
		baseinsertsql ^= sqlcolid ^ ",";

		//debug
		//dictid.outputt();
		//op.outputt();
		//ovalue.outputt();
		//var("").outputl();

		//debug
		if (calc_fields_file && dictid != "AUTHORISED") {
			var key = dictfilename ^ "*" ^ dictid;
			var rec = sortselectclause ^ FM ^ op ^ FM ^ ovalue ^ FM ^ ovalue2;
			rec.write(calc_fields_file, key);
			key.outputl("written to calc_fields ");
		}
	}

	if (baseinsertsql[-1] == ",") {

		baseinsertsql.splicer(-1, 1, ")");
		baseinsertsql ^= " VALUES (";

		createtablesql.splicer(-1, 1, ")");
		//createtablesql.outputl();

		//create the temporary table
		CURSOR.sqlexec(createtablesql);
	} else
		baseinsertsql = "";

	int maxnrecs = calc_fields.a(6);
	int recn = 0;

	//nextrecord:
	while (CURSOR.readnext(RECORD, ID, MV)) {

		bool allok = true;

		//var id2 = MV ? (ID ^ "*" ^ MV) : ID;
		var insertsql = baseinsertsql ^ ID.swapper("'", "''").squote() ^ ",";

		//some or all fields may have filters on them
		//some or all fields may not have filters, if 'order by' calculated fields
		for (int fieldn = 1; fieldn <= nfields; ++fieldn) {

			var ivalues = calculate(dictids(fieldn));

			if (opnos(fieldn)) {
				//debug
				//ivalues.outputl(dictids(fieldn) ^ " ivalues=");

				var nextsep;
				var pos = 1;
				bool ok = false;

				while (true) {

					var ivalue = ivalues.substr2(pos,nextsep);

					switch (int(opnos(fieldn))) {
						case 0://cannot occur - due to if statement above
							break;
						case 1:	 // =
							ok = ivalue == reqivalues(fieldn);
							//ok = ivalue.locate(reqivalues(fieldn));
							//if (ok) {
							//	TRACE(ivalue)
							//	TRACE(reqivalues(fieldn))
							//	TRACE(calculate("COMPANY_CODE"))
							//}
							break;
						case 2:	 // <>
							ok = ivalue != reqivalues(fieldn);
							//ok = !ivalue.locate(reqivalues(fieldn));
							break;
						case 3:	 // >
							ok = ivalue > reqivalues(fieldn);
							break;
						case 4:	 // <
							ok = ivalue < reqivalues(fieldn);
							break;
						case 5:	 // >=
							ok = ivalue >= reqivalues(fieldn);
							//TRACE(ivalue ^ " " ^ reqivalues(fieldn) ^ " " ^ ok)
							break;
						case 6:	 // <=
							ok = ivalue <= reqivalues(fieldn);
							break;
						case 7:	 // ~ regex
							ok = ivalue.match(reqivalues(fieldn));
							break;
						case 8:	 // ~* regex case insensitive
							ok = ivalue.match(reqivalues(fieldn), "i");
							break;
						case 9:	 // !~ not regex
							ok = !(ivalue.match(reqivalues(fieldn)));
							break;
						case 10:  // !~* not regex case insensitive
							ok = !(ivalue.match(reqivalues(fieldn), "i"));
							break;
						case 11:  // between x and y, from x to
							ok = (ivalue >= reqivalues(fieldn) && ivalue <= reqivalues2(fieldn));
							break;
						case 12:  // not between x and y, not from x to y
							ok = (ivalue < reqivalues(fieldn) || ivalue > reqivalues2(fieldn));
							break;
						case 13:  // in list
							ok = reqivalues(fieldn).locate(ivalue);
							break;
						case 14:  // not in list
							ok = !reqivalues(fieldn).locate(ivalue);
							break;
						case 15:  // is true (not "" 0 "0" "00" "0.0" etc).
							ok = ivalue;
							break;
						case 16:  // is false (isnt true)
							ok = !ivalue;
							break;
						case 17:  // STARTING ]
							ok = reqivalues(fieldn).substr(0, ivalue.length()) == ivalue;
							break;
						case 18:  // ENDING [
							ok = reqivalues(fieldn).substr(-ivalue.length()) == ivalue;
							ok = !ivalue;
							break;
						case 19:  //  CONTAINING []
							ok = reqivalues(fieldn).index(ivalue);
							break;
					}//switch

					//quit searching data values if found or no more
					if (ok || !nextsep)
						break;

				}//for each ivalue

				//if a field fails test then skip and remaining fields and readnext candidate record
				if (!ok) {
					//debug
					//ivalue.outputl("FAILED=");
					allok = false;
					break;// out of fields loop. assuming that all filters are AND. fail one = skip record
				}

			}//if opno

			var sqltype = sqltypes(fieldn);
			var ovalues;
			if (sqltype == "DATE" || sqltype == "TIME" || sqltype == "TIMESTAMP") {
				if (ivalues) {
					ovalues = oconv(ivalues,ioconvs(fieldn)).squoter();
				}
				else
					ovalues = "NULL";
			}
			else {
				//ivalueS (41472, 'Practical PostgreSQL', 1212, 4);
				ovalues = ivalues.swap("'", "''").squoter();
			}

			insertsql ^= " " ^ ovalues ^ ",";

		}//fieldn

		//skip if failed to match
		if (!allok) {
			//ID.outputl("failed ");
			continue;
		}

		//ID.outputl("stage1 ok ");

		//replace final comma with a closing bracket
		insertsql.splicer(-1, 1, ")");

		//insertsql.outputl();

		if (not CURSOR.sqlexec(insertsql)) {
			insertsql.errputl("Error inserting pass2 record:");
		}

		//option to limit number of records returned
		++recn;
		if (maxnrecs && recn > maxnrecs) {
			//break out of readnext loop
			CURSOR.clearselect();
			//break;
		}

	}//readnext

	sortselectclause.errputl("\nstage2=");

	bool result = CURSOR.select(sortselectclause);

	return result;
}

bool ExodusProgramBase::savelist(const var& listname) {
	return CURSOR.savelist(listname.field(" ", 1));
}

bool ExodusProgramBase::getlist(const var& listname) {
	return CURSOR.getlist(listname.field(" ", 1));
}

bool ExodusProgramBase::formlist(const var& filename_or_command, const var& keys /*=""*/, const var fieldno /*=0*/) {
	//remove any options from the filename or command
	var filename2 = filename_or_command;
	if (filename2[-1] eq ")") {
		var options = filename2.field2(" ", -1);
		filename2.splicer(-options.length(), 999999999, "").trimmerb();
	}

	//optionally get keys from filename or command
	var keys2 = (keys == "") ? filename2.field(" ", 2, 999999999) : keys;

	//remove any keys from the filename
	filename2 = filename2.field(" ", 1);

	//open the file
	clearselect();
	if (not CURSOR.open(filename2))
		throw MVError(filename2.quote() ^ " file cannot be opened in formlist(" ^ keys ^ ")");

	return CURSOR.formlist(keys2, fieldno);
}

bool ExodusProgramBase::makelist(const var& listname, const var& keys) {
	return CURSOR.makelist(listname.field(" ", 1), keys);
}

bool ExodusProgramBase::deletelist(const var& listname) {
	return CURSOR.deletelist(listname.field(" ", 1));
}

void ExodusProgramBase::clearselect() {
	CURSOR.clearselect();
}

bool ExodusProgramBase::hasnext() {
	return CURSOR.hasnext();
}

bool ExodusProgramBase::readnext(var& key) {
	return CURSOR.readnext(key);
}

bool ExodusProgramBase::readnext(var& key, var& valueno) {
	return CURSOR.readnext(key, valueno);
}

bool ExodusProgramBase::readnext(var& record, var& key, var& valueno) {
	return CURSOR.readnext(record, key, valueno);
}

bool ExodusProgramBase::deleterecord(const var& filename_or_handle_or_command, const var& key) {
	if (filename_or_handle_or_command.index(" ") || key.length() == 0) {
		var command = filename_or_handle_or_command.a(1);

		var filename = command.field(" ", 1);

		//if any keys provided (remove quotes if present)
		int nwords = command.dcount(" ");

		//find and skip final options like (S)
		bool silent = false;
		if (command[-1] == ")" || command[-1] == "}") {
			silent = command.field2(" ", -1).index("S");
			nwords--;
		}

		if (nwords >= 2) {
			for (int wordn = 2; wordn <= nwords; ++wordn) {
				var key = command.field(" ", wordn).unquote();
				if (filename.deleterecord(key)) {
					silent || key.quote().outputl("Deleted ");
				} else {
					silent || key.quote().errputl("NOT deleted ");
				}
			}
		}
		//delete keys provided in a select list
		else {
			var key;
			while (CURSOR.readnext(key)) {
				if (filename.deleterecord(key)) {
					silent || key.quote().outputl("Deleted ");
				} else {
					silent || key.quote().errputl("NOT deleted ");
				}
			}
		}
		return true;
	}

	return filename_or_handle_or_command.deleterecord(key);
	//return filehandle.deleterecord(key);
}

bool ExodusProgramBase::pushselect([[maybe_unused]] const var& v1, var& v2, [[maybe_unused]] var& v3, [[maybe_unused]] var& v4) {

	// CURSOR.quote().outputl("CURSOR=");
	// CURSOR++;
	// CURSOR has connection number hidden in it, so it cannot be used as an ordinary variable
	// ATM
	CURSOR.transfer(v2);
	CURSOR = var().date() ^ "_" ^ (var().ostime().convert(".", "_"));
	return true;

	/*
	SUBROUTINE PUSH.SELECT(V1, V2, V3, V4)
	11 NULL
	12 *STOPLINECOUNT
	13 *STARTLINECOUNT
	14 *STOPLINECOUNT
	15 *STARTLINECOUNT
	16 TRANSFER SYS109_27 TO V2
	20 V3 = SYS109_63
	24 V3 := @RM : SYS109_99
	33 V3 := @RM : SYS109_140
	42 V3 := @RM : SYS109_64
	51 V3 := @RM : SYS109_61_FILEHANDLE
	60 V3 := @RM : SYS109_87
	69 V3 := @RM : SYS109_65
	78 V3 := @RM : SYS109_66
	87 V3 := @RM : SYS109_135_MAPNAME
	96 V3 := @RM : SYS109_79
	105 V4 = SYS109_58
	109 SYS109_63 = 0
	113 SYS109_100 += 1
	119 RETURN
	*/
}

bool ExodusProgramBase::popselect([[maybe_unused]] const var& v1, var& v2, [[maybe_unused]] var& v3, [[maybe_unused]] var& v4) {
	// CURSOR.quote().outputl("CURSOR=");
	// CURSOR--;
	v2.transfer(CURSOR);
	return true;
	/*
	SUBROUTINE POP.SELECT(V1, V2, V3, V4)
	11 NULL
	12 *STOPLINECOUNT
	13 *STARTLINECOUNT
	14 *STOPLINECOUNT
	15 *STARTLINECOUNT
	16 TRANSFER V2 TO SYS109_27
	20 SYS109_63 = V3[1, @RM]
	28 V3[1, COL2()]=''
	35 SYS109_99 = V3[1, @RM]
	43 V3[1, COL2()]=''
	50 SYS109_140 = V3[1, @RM]
	58 V3[1, COL2()]=''
	65 SYS109_64 = V3[1, @RM]
	73 V3[1, COL2()]=''
	80 SYS109_61_FILEHANDLE = V3[1, @RM]
	88 V3[1, COL2()]=''
	95 SYS109_87 = V3[1, @RM]
	103 V3[1, COL2()]=''
	110 SYS109_65 = V3[1, @RM]
	118 V3[1, COL2()]=''
	125 SYS109_66 = V3[1, @RM]
	133 V3[1, COL2()]=''
	140 SYS109_135_MAPNAME = V3[1, @RM]
	148 V3[1, COL2()]=''
	155 SYS109_79 = V3[1, @RM]
	163 SYS109_58 = V4
	167 SYS109_100 -= 1
	173 RETURN
	*/
}

void ExodusProgramBase::note(const var& msg, const var& options) const {
	var buffer = "";
	mssg(msg, options, buffer);
}
void ExodusProgramBase::note(const var& msg, const var& options, var& buffer, const var& params) const {
	mssg(msg, options, buffer, params);
}

void ExodusProgramBase::mssg(const var& msg, const var& options) const {
	var buffer = "";
	mssg(msg, options, buffer);
}
void ExodusProgramBase::mssg(const var& msg, const var& options, var& buffer, const var& params) const {

	//skip if just "downing" a previous "upped" message
	if (options.index("D")) {
		std::cout << std::endl;
		return;
	}

	var interactive = !SYSTEM.a(33);
	if (interactive)
		std::cout << var("----------------------------------------") << std::endl;

	//we must pass the message unmodified into USER4 below
	//e.g. to pass ACCOUNTLIST back to client with FM/VM etc
	//var msg1 = msg.convert("|" ^ FM ^ VM ^ SM, "\n\n\n\n").trim("\n");
	var msg1 = msg;

	//swap %1, %2 etc with params
	for (var ii = 1; ii <= 9; ++ii)
		msg1.swapper("%" ^ ii, params.a(ii));

	msg1.converter(_FM_ _VM_ "|", "\n\n\n").trimmer("\n");

	std::cout << msg1 << std::endl;

	var origbuffer = buffer.assigned() ? buffer : "";

	//R=Reply required in buffer
	if (options.index("R")) {
		if (interactive) {

			//one space after the prompt
			//std::cout << " ";

			if (buffer.unassigned())
				buffer = "";

			//input with empty prompt allows defaulting and editing
			buffer.input("? ");

			//default
			//if (buffer == "")
			//	buffer=origbuffer;

			//escape anywhere in the input returned as a single ESC character
			//or empty input with ESC option means ESC
			if (options.index("E") and (buffer == "" or buffer.index("\x1B")))
				buffer = "\x1B";  //esc

			std::cout << std::endl;
		} else {

			//input=output if not interactive
			buffer = origbuffer;
		}

		//force upper case
		if (options.index("C"))
			buffer.ucaser();

		return;
	}

	if (!options.index("U")) {
		if (USER4.length() > 8000) {
			var msg2 = "Aborted MSG()>8000";
			if (not USER4.index(msg2)) {
				std::cout << msg2 << std::endl;
				//std::cout << USER4 << std::endl;
				USER4 ^= FM ^ msg2;
			}
		} else {
			USER4.r(-1, msg1);
		}
	}
}

var ExodusProgramBase::authorised(const var& task0) {
	var msg;
	return authorised(task0, msg);
}

var ExodusProgramBase::authorised(const var& task0, var& msg, const var& defaultlock, const var& username0) {

	var username;
	var msgusername;
	var taskn;	// num
	var taskn2;
	var xx;
	var usern;

	var task = task0;
	if (username0.unassigned() or username0 eq "") {
		// allow for username like FINANCE(STEVE)
		// so security is done like FINANCE but record is kept of actual user
		// this allows for example billing module users to post as finance module users
		username = USERNAME.field("(", 1);
		msgusername = USERNAME;
	} else {
		username = username0;
		msgusername = username;
	}

	// if username='EXODUS' or username='STEVE' then call msg(task:'');

	if (task[1] == " ") {
		call mssg(DQ ^ (task0 ^ DQ));
	}
	// Each task may have many "locks", each users may have many "keys"
	// A user must have keys to all the locks in order to pass

	if (not task) {
		return 1;
	}

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SM, "\\\\\\");
	task.swapper(" FILE ", " ");
	task.swapper(" - ", " ");
	task.converter(".", " ");
	task.trimmer();

	msg = "";
	// **CALL note(' ':TASK)

	if (task.substr(1, 2) == "..") {
		// call note(task:'')
		return 1;
	}

	var noadd = task[1] == "!";
	if (noadd) {
		task.splicer(1, 1, "");
	}
	// if noadd else NOADD=((TASK[-1,1]='"') and (len(userprivs)<10000))
	if (not noadd) {
		var lenuserprivs = SECURITY.length();
		noadd = task[-1] == DQ or lenuserprivs > 48000;
	}
	var positive = task[1];
	if (positive == "#")
		task.splicer(1, 1, "");
	else
		positive = "";

	//? as first character of task (after positive) means
	// security is being used as a configuration and user EXODUS has no special privs
	var isadmin;
	if (task[1] == "?") {
		isadmin = 0;
		task.splicer(1, 1, "");
	} else
		isadmin = username == "EXODUS";

	var deleting = task.substr(1, 8) == "%DELETE%";
	if (deleting) {
		task.splicer(1, 8, "");
	}
	var updating = task.substr(1, 8) == "%UPDATE%";
	if (updating) {
		task.splicer(1, 8, "");
	}
	var renaming = task.substr(1, 8) == "%RENAME%";
	if (renaming) {
		task.splicer(1, 8, "");
	}

	// find the task
	if (SECURITY.a(10).locate(task, taskn)) {
		if (deleting) {
			// SECURITY.eraser(10, taskn);
			// SECURITY.eraser(11, taskn);
			SECURITY.remover(10, taskn);
			SECURITY.remover(11, taskn);
updateprivs:
			gosub writeuserprivs();
			return 1;
		} else if (renaming) {
			// delete any existing rename target task
			if (SECURITY.a(10).locate(defaultlock, taskn2)) {
				// SECURITY.eraser(10, taskn2);
				// SECURITY.eraser(11, taskn2);
				SECURITY.remover(10, taskn2);
				SECURITY.remover(11, taskn2);
				if (taskn2 < taskn) {
					taskn -= 1;
				}
			}
			SECURITY.r(10, taskn, defaultlock);
			if (renaming) {
				call note(task ^ "|TASK RENAMED|" ^ defaultlock);
			}
			goto updateprivs;
		} else if (updating) {
			var tt = defaultlock;
			if (SECURITY.a(10).locate(defaultlock, taskn2)) {
				tt = SECURITY.a(11, taskn2);
			}
			SECURITY.r(11, taskn, tt);
			goto updateprivs;
		}
	} else {
		if (deleting) {
			return 1;
		}
		if (renaming) {
			// if the task to be renamed doesnt exist just add the target task
			call authorised(defaultlock, msg);
			return 1;
		}
		if (not noadd) {
			gosub readuserprivs();
			// if (SECURITY.length() < 65000) {
			if (true) {
				var x = var();
				if (not(SECURITY.a(10).locateby("A", task, taskn))) {
					var newlock = defaultlock;
					// get locks on default task if present otherwise new locks
					// are none
					if (newlock and SECURITY.a(10).locate(newlock)) {
						newlock = SECURITY.a(11, xx);
					}
					SECURITY.inserter(10, taskn, task);
					SECURITY.inserter(11, taskn, newlock);
					gosub writeuserprivs();
					if (username == "EXODUS") {
						call note(task ^ "|TASK ADDED");
					}
				}
			}
		}
	}

	// if no locks then pass ok unless positive locking required
	var locks = SECURITY.a(11, taskn);
	if (locks == "") {
		if (positive and not isadmin) {
notallowed:
			// MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not
			// authorised to do this.|'
			if (msgusername != USERNAME)
				msg = capitalise(msgusername) ^ "is not";
			else
				msg = "Sorry, " ^ capitalise(msgusername) ^ ", you are";

			msg ^= " not";
			if (positive) {
				msg ^= " specifically";
			}
			msg ^= " authorised to do||" ^ capitalise(task);

			return 0;
		} else
			return 1;
	} else if (locks == "NOONE") {
		goto notallowed;
	}

	// if index('012',@privilege,1) then goto ok
	if (isadmin) {
		return 1;
	}

	// find the user (add to bottom if not found)
	// surely this is not necessary since users are in already
	if (not(SECURITY.a(1).locate(username, usern))) {
		if (username != "EXODUS" and username != APPLICATION) {
			gosub readuserprivs();
			usern = (SECURITY.a(1)).count(VM) + (SECURITY.a(1) != "") + 1;
			if (SECURITY.length() < 65000) {
				var users;
				if (not(users.open("USERS"))) {
					goto notallowed;
				}
				var USER;
				if (not(USER.read(users, username))) {
					goto notallowed;
				}
				SECURITY.inserter(1, usern, username);
				SECURITY.inserter(2, usern, "");
				// add in memory only
				// gosub writeuserprivs
			}
		}
	}

	// user must have all the keys for all the locks on this task
	// following users up to first blank line also have the same keys
	var keys = SECURITY.a(2).field(VM, usern, 999);
	var temp = keys.index("---", 1);
	if (temp) {
		keys.splicer(temp - 1, 999, "");
	}
	// convert ',' to vm in keys
	// convert ',' to vm in locks
	keys.converter("," _VM_ "", "  ");
	locks.converter(",", " ");
	// NLOCKS=COUNT(LOCKS,vm)+1
	var nlocks = locks.count(" ") + 1;

	for (var lockn = 1; lockn <= nlocks; ++lockn) {
		// LOCKx=FIELD(LOCKS,vm,LOCKN)
		var lockx = locks.field(" ", lockn);
		if (keys.locateusing(" ", lockx)) {
			// call note(task:' ok')
		} else
			// MSG=capitalise(TASK):'||Sorry, ':capitalise(msgusername):', you are not
			// authorised to do this.|' MSG='Sorry, ':capitalise(msgusername):', you are
			// not authorised to do||':capitalise(task)
			// call note(task:' ko')
			// RETURN 0
			goto notallowed;
	};	// lockn;

	// ok:
	// CALL STATUP(2,3,TASK)
	return 1;
}

void ExodusProgramBase::readuserprivs() const {
	if (not DEFINITIONS or not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		SECURITY = "";
	}
	return;
}

void ExodusProgramBase::writeuserprivs() const {
	SECURITY.r(9, "");
	if (DEFINITIONS) {
		SECURITY.write(DEFINITIONS, "SECURITY");
	}
	return;
}

var ExodusProgramBase::capitalise(const var& str0, const var& mode0, const var& wordseps0) const {

	var string2;

	if (mode0.unassigned() || mode0 == "CAPITALISE") {
		string2 = str0;
		// convert @upper.case to @lower.case in string2
		int nn = string2.length();
		var numx = var("1234567890").index(string2[1], 1);
		var cap = 1;
		var wordseps;
		var inquotes = 0;
		// wordseps=' /-.()&'
		if (wordseps0.unassigned())
			wordseps = " .()&_" _RM_ _FM_ _VM_ _SM_ _TM_ _STM_;
		else
			wordseps = wordseps0;
		for (int ii = 1; ii <= nn; ii++) {
			var tt = string2.substr(ii, 1);

			if (inquotes) {
				inquotes = tt != inquotes;
			} else {
				if (tt == DQ && (string2.count(DQ) > 1 || tt == "\'") &&
					string2.count("\'") > 1) {
					inquotes = tt;
				} else {
					if (wordseps.index(tt, 1)) {
						cap = 1;
						if (tt == " ")
							numx = var("1234567890")
									   .index(string2.substr(ii + 1, 1),
											  1);
					} else {
						if (cap || numx) {
							tt.converter(LOWERCASE, UPPERCASE);
							string2.splicer(ii, 1, tt);
							cap = 0;
						} else {
							tt.converter(UPPERCASE, LOWERCASE);
							string2.splicer(ii, 1, tt);
						}
					}
				}
			}

		};	// ii;

		string2.swapper("\'S ", "\'s ");
		if (string2.substr(-2, 2) == "\'S")
			string2.splicer(-2, 2, "\'s");
	} else if (mode0 == "QUOTE") {
		string2 = str0;
		if (string2 != "") {
			string2.converter(FM ^ VM ^ SVM ^ TM, "    ");
			string2.swapper(" ", "\" \"");
			string2 = string2.quote();
		}
	} else if (mode0 == "UPPERCASE") {
		string2 = str0;
		string2.converter(LOWERCASE, UPPERCASE);
	} else if (mode0 == "LOWERCASE") {
		string2 = str0;
		string2.converter(UPPERCASE, LOWERCASE);
	} else if (mode0.substr(1, 5) == "PARSE") {

		var uppercase = mode0.index("UPPERCASE", 1);

		string2 = str0;

		// convert to uppercase
		var quoted = "";
		for (int ii = 1; ii <= 99999; ii++) {
			var tt = string2.substr(ii, 1);
			// BREAK;
			if (!(tt != ""))
				break;
			;
			if (tt == quoted) {
				quoted = "";
			} else {
				if (!quoted) {
					if ((DQ ^ "\'").index(tt, 1)) {
						quoted = tt;
					} else {
						if (tt == " ") {
							tt = FM;
							string2.splicer(ii, 1, tt);
						} else {
							if (uppercase) {
								tt.converter(LOWERCASE, UPPERCASE);
								string2.splicer(ii, 1, tt);
							}
						}
					}
				}
			}
		};	// ii;

		if (mode0.index("TRIM", 1)) {
			string2.converter(" " _FM_, _FM_ " ");
			string2 = string2.trim();
			string2.converter(" " _FM_, _FM_ " ");
		}
	}

	return string2;
}

var ExodusProgramBase::execute(const var& sentence) {

	var v1, v2, v3, v4;
	pushselect(v1, v2, v3, v4);

	var sentence2 = sentence.fieldstore(" ", 1, 1, sentence.field(" ", 1).lcase());

	var result = perform(sentence);

	popselect(v1, v2, v3, v4);

	return result;
}

void ExodusProgramBase::chain(const var& libraryname) {
	CHAIN = libraryname;
	var().stop();
}

var ExodusProgramBase::perform(const var& sentence) {
	// THISIS("var ExodusProgramBase::perform(const var& sentence)")
	// ISSTRING(sentence)

	// return ID^"*"^dictid;

	// wire up the the library linker to have the current mvenvironment
	// if (!perform_exodusfunctorbase_.mv_)
	//	perform_exodusfunctorbase_.mv_=this;

	// lowercase all library functions to aid in conversion from pickos
	// TODO remove after conversion complete

	perform_exodusfunctorbase_.mv_ = (&mv);

	// save some environment
	var savesentence;
	var savecommand;
	var saveoptions;
	var saverecur0;
	var saverecur1;
	var saverecur2;
	var saverecur3;
	var saverecur4;
	//
	var saveid;
	var saverecord;
	var savemv;
	var savedict;

	SENTENCE.transfer(savesentence);
	COMMAND.transfer(savecommand);
	OPTIONS.transfer(saveoptions);
	RECUR0.transfer(saverecur0);
	RECUR1.transfer(saverecur1);
	RECUR2.transfer(saverecur2);
	RECUR3.transfer(saverecur3);
	RECUR4.transfer(saverecur4);
	//
	ID.transfer(saveid);
	RECORD.transfer(saverecord);
	MV.transfer(savemv);
	DICT.transfer(savedict);

	//a lambda function to restore the environment
	auto restore_environment = [&]() {
		// restore some environment
		savesentence.transfer(SENTENCE);
		savecommand.transfer(COMMAND);
		saveoptions.transfer(OPTIONS);
		saverecur0.transfer(RECUR0);
		saverecur1.transfer(RECUR1);
		saverecur2.transfer(RECUR2);
		saverecur3.transfer(RECUR3);
		saverecur4.transfer(RECUR4);
		//
		saveid.transfer(ID);
		saverecord.transfer(RECORD);
		savemv.transfer(MV);
		savedict.transfer(DICT);
	};

	SENTENCE = sentence;
	while (SENTENCE) {

		// set new perform environment
		COMMAND = SENTENCE;
		OPTIONS = "";
		// similar code in exodus_main() and mvprogram.cpp:perform()
		var lastchar = COMMAND[-1];
		if (lastchar == ")") {
			OPTIONS = "(" ^ COMMAND.field2("(", -1);
		} else if (lastchar == "}")
			OPTIONS = "{" ^ COMMAND.field2("{", -1);
		if (OPTIONS)
			COMMAND.splicer(-(OPTIONS.length()), OPTIONS.length(), "");

		// load the shared library file
		var libid = SENTENCE.field(" ", 1).lcase();
		std::string str_libname = libid.toString();
		if (!perform_exodusfunctorbase_.initsmf(str_libname.c_str(),
												"exodusprogrambasecreatedelete_",
												true  // forcenew each perform/execute
												)) {
			USER4 ^= "perform() Cannot find shared library \"" + str_libname +
					 "\", or \"libraryexit()\" is not present in it.";
			// throw MVError(USER4);
			// return "";
			break;
		}

		// call the shared library exodus object's main function
		try {
			ANS = perform_exodusfunctorbase_.callsmf();
		} catch (const MVUndefined&) {
			// if return "" is missing then default ANS to ""
			ANS = "";
		} catch (const MVStop&) {
			// stop is normal way of stopping a perform
			// functions can call it to terminate the whole "program"
			// without needing to setup chains of returns
			// to exit from nested functions
			ANS = "";
		} catch (const MVAbort&) {
			// similar to stop for the time being
			// maybe it should set some error flag/messages
			ANS = "";
		} catch (const MVAbortAll&) {
			// similar to stop for the time being
			// maybe it should set some error flag/messages
			// and abort multiple levels of perform?
			ANS = "";
		}
		//TODO create a second version of this whole try/catch block
		//that omits catch (MVError) if EXO_DEBUG is set
		//so that gdb will catch the original error and allow backtracing there
		//Until then, use gdb "catch throw" as mentioned below.
		catch (const MVError&) {
			//restore environment in case MVError is caught
			//in caller and the program resumes processing
			restore_environment();

			//use gdb command "catch throw" to break at error line to get back traces there
			throw;
		}

		//dont catch MVLogoff in perform/execute. leave it for mvprogram top level exit
		//catch (const MVLogoff& e)
		//{
		//	// similar to stop for the time being
		//	// maybe it should set some error flag/messages
		//	// and abort multiple levels of perform?
		//	ANS = "";
		//}

		// chain is a kind of optional follow controlled by the library function
		// to perform another function after the first
		// go round again if anything in CHAIN
		CHAIN.transfer(SENTENCE);
	}

	// restore some environment
	restore_environment();

	return ANS;
}

var ExodusProgramBase::xlate(const var& filename, const var& key, const var& fieldno_or_name, const var& mode) {

	// TODO implement additional MV argument

	// key can be multivalued and multivalued result will be returned
	var results = "";
	var nkeys = key.dcount(VM);

	for (var keyn = 1; keyn <= nkeys; ++keyn) {

		var keyx = key.a(1, keyn);

		// handle non-numeric field_no ie dictionary field/column name
		if (not fieldno_or_name.isnum()) {

			// get the whole record
			var record = keyx.xlate(filename, "", mode);

			// TODO what if key is multivalued?

			// handle record not found and mode C
			if (mode == "C" && record == keyx) {
				results.r(keyn, key);
				continue;
			}

			// handle record not found and mode X
			if (not record.length()) {
				results.r(keyn, "");
				continue;
			}

			// use calculate()
			var result =
				calculate(fieldno_or_name, "dict_" ^ filename.a(1), keyx, record);
			if (nkeys > 1)
				result.lowerer();
			results.r(keyn, result);
			continue;
		}

		// ordinary numeric or "" fieldno
		results.r(keyn, keyx.xlate(filename, fieldno_or_name, mode));
	}

	if (nkeys > 1)
		results.converter(_FM_, _VM_);
	//else
	//    sep = _RM_;

	return results;
}

var ExodusProgramBase::calculate(const var& dictid, const var& dictfile, const var& id, const var& record, const var& mvno) {

	//dictid @ID/@id is hard coded to return ID
	//to avoid incessant lookup in main file dictionary and then defaulting to dict_voc
	//sadly this means that @ID cannot be customised per file
	//possibly amend the read cache to cache the dict_voc version for the main file
	//if (dictid == "@ID" || dictid == "@id")
	//	return ID;

	// TODO how is it possible to exchange when the variable is const?

	DICT.exchange(dictfile);
	ID.exchange(id);
	RECORD.exchange(record);
	MV.exchange(mvno);

	var result = calculate(dictid);

	DICT.exchange(dictfile);
	ID.exchange(id);
	RECORD.exchange(record);
	MV.exchange(mvno);

	return result;
}

var ExodusProgramBase::calculate(const var& dictid) {
	// THISIS("var ExodusProgramBase::calculate(const var& dictid)")
	// ISSTRING(dictid)

	// return ID^"*"^dictid;

	// get the dictionary record so we know how to extract the correct field or call the right
	// library
	bool newlibfunc;
	bool indictvoc = false;
	if (cache_dictid_ != (DICT.a(1) ^ " " ^ dictid)) {
		newlibfunc = true;
		if (not DICT)
			throw MVError("ExodusProgramBase::calculate(" ^ dictid ^
						  ") DICT file variable has not been set");
		if (not cache_dictrec_.reado(DICT, dictid)) {
			// try lower case
			if (not cache_dictrec_.reado(DICT, dictid.lcase())) {
				// try dict_voc
				var dictvoc;  // TODO implement mv.DICTVOC to avoid opening
				if (not dictvoc.open("dict_voc")) {
baddict:
					throw MVError("ExodusProgramBase::calculate(" ^ dictid ^
								  ") dictionary record not in DICT " ^
								  DICT.a(1).quote() ^ " nor in DICT_VOC");
				}
				if (not cache_dictrec_.reado(dictvoc, dictid)) {
					// try lower case
					if (not cache_dictrec_.reado(dictvoc, dictid.lcase())) {
						goto baddict;
					}
				}
				indictvoc = true;
			}
			// cache whichever dict record was found
			// regardless of file xxxxxxxx/voc and upper/lower case key
			// as if it was found in the initial file,key requested
			// this will save repeated drill down searching on every access.
			cache_dictrec_.r(16, indictvoc);
			cache_dictrec_.writeo(DICT, dictid);
		}
		cache_dictid_ = DICT.a(1) ^ " " ^ dictid;

		//detect from the cached record if it came from dict_voc
		//so we can choose the libdict_voc if so
		indictvoc = cache_dictrec_.a(16);

	} else
		newlibfunc = false;

	var dicttype = cache_dictrec_(1);
	bool ismv = cache_dictrec_(4)[1] == "M";

	// F type dictionaries
	if (dicttype == "F") {

		// check field number is numeric
		var fieldno = cache_dictrec_(2);
		if (!fieldno.isnum())
			return "";

		// field no > 0
		if (fieldno) {
			if (ismv)
				return RECORD(fieldno, MV);
			else
				return RECORD(fieldno);

			// field no 0
		} else {
			var keypart = cache_dictrec_(5);
			if (keypart && keypart.isnum())
				return ID.field("*", keypart);
			else
				return ID;
		}
	} else if (dicttype == "S") {
		// TODO deduplicate various exodusfunctorbase code spread around calculate mvipc*
		// etc
		if (newlibfunc) {

			std::string str_libname;
			if (indictvoc)
				str_libname = "dict_voc";
			else
				str_libname = DICT.a(1).lcase().convert(".", "_").toString();

			// get from cache
			std::string cachekey = dictid.lcase().toString() + "_" + str_libname;
			dict_exodusfunctorbase_ = dict_function_cache[cachekey];
			// if (dict_exodusfunctorbase_) {
			//	delete dict_exodusfunctorbase_;
			//	dict_exodusfunctorbase_=NULL;
			//}

			// if not in cache then create new one
			if (!dict_exodusfunctorbase_) {
				// var(cachekey).outputl("cachekey=");
				dict_exodusfunctorbase_ = new ExodusFunctorBase;
				dict_function_cache[cachekey] = dict_exodusfunctorbase_;
				// wire up the the library linker to have the current mvenvironment
				// if (!dict_exodusfunctorbase_.mv_)
				dict_exodusfunctorbase_->mv_ = (&mv);

				std::string str_funcname =
					("exodusprogrambasecreatedelete_" ^ dictid.lcase()).toString();
				if (!dict_exodusfunctorbase_->initsmf(str_libname.c_str(),
													  str_funcname.c_str()))
					throw MVError("ExodusProgramBase::calculate() Cannot find Library " +
								  str_libname + ", or function " +
								  dictid.lcase() + " is not present");
			}
		}

		//AREV does not do this.
		//in AREV, a multivalued dict may call a single valued dict that calls a multivalued dict.
		//and MV needs to be preserved over the set of calls.
		//Therefore all the following is cancelled.
		// for single valued fields, inform the called routine that MV is 0
		//int savedMV;
		//if (!ismv) {
		//	savedMV = MV;
		//	MV = 0;
		//}

		// return dict_exodusfunctorbase_.calldict();
		// return ANS;

		// call the shared library object main function with the right args (none for
		// dicts), returning a var std::cout<<"precal"<<std::endl;
		ANS = CALLMEMBERFUNCTION(*(dict_exodusfunctorbase_->pobject_),
								 ((pExodusProgramBaseMemberFunction)(
									 dict_exodusfunctorbase_->pmemberfunction_)))();
		// std::cout<<"postcal"<<std::endl;

		// restore the MV if necessary
		//if (!ismv)
		//	MV = savedMV;

		return ANS;
	}

	throw MVError("ExodusProgramBase::calculate(" ^ dictid ^ ") " ^ DICT ^ " Invalid dictionary type " ^
				  dicttype.quote());
	return "";
}

//unlock all
bool ExodusProgramBase::unlockrecord() const {
	var xx;
	return unlockrecord("", xx, "");
}

bool ExodusProgramBase::unlockrecord(const var& filename, var& file0, const var& key) const {
	var file;
	if (file0.unassigned())
		file = "";
	else
		file = file0;

	if (file == "") {
		var().unlockall();
		return 1;
	}

	// remove persistentlock
	// common /shadow.mfs/
	// shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3 done by
	// shadow.mfs

	// remove actual lock
	file.unlock(key);

	return 1;

	// evade warning: unused parameter
	if (filename) {
	}
	return 1;
}

void ExodusProgramBase::debug() const {

	var reply;
	std::clog << "debug():";
	if (OSSLASH == "/")
		asm("int $3");	//use gdb n commands to resume
	else
		var().debug();
	return;
}

bool ExodusProgramBase::fsmsg(const var& msg) const {
	mssg(msg ^ var().lasterror());
	return false;
}

var ExodusProgramBase::sysvar(const var& var1, const var& var2, const var& var3, const var& var4) {

	std::cout << "sysvar() do nothing:";
	//	var reply;
	//	cin>>reply;
	return "";

	// evade warning: unused parameter
	if (var1 || var2 || var3 || var4) {
	}
}

void ExodusProgramBase::setprivilege(const var& var1) {

	PRIVILEGE = var1;
	std::cout << "setprivilege(" << var1 << ") do nothing" << std::endl;
	//	var reply;
	//	cin>>reply;
	return;
}

var ExodusProgramBase::decide(const var& question, const var& options) const {
	var reply = "";
	return decide(question, options, reply, 1);
}

var ExodusProgramBase::decide(const var& questionx, const var& optionsx, var& reply, const int defaultreply) const {

	// If default reply is 0 then there is no default
	// and pressing Enter returns "" and reply is set to 0

	// question can be multiline
	var question = questionx;
	question.converter(STM ^ TM ^ SM ^ VM ^ "|" ^ FM, "\n\n\n\n\n\n");

	var interactive = !SYSTEM.a(33);
	if (interactive)
		std::cout << var("========================================") << std::endl;

	std::cout << question << std::endl;

	//	var noptions = options.count(FM) + (options != "");
	var options = optionsx;
	options.converter(VM ^ "|", FM ^ FM);
	var noptions = options.dcount(FM);
	for (int optionn = 1; optionn <= noptions; optionn++) {
		if (optionn == defaultreply)
			std::cout << "*";
		else
			std::cout << " ";
		std::cout << optionn << ". " << options.a(optionn) << std::endl;
	}

inp:
	if (defaultreply)
		std::cout << " Please enter 1 - " << noptions << " or Enter to default or 0 to cancel.";
	else
		std::cout << " Please enter 1 - " << noptions << " or Enter to cancel.";
	std::cout << std::endl;

	reply = defaultreply;

	if (interactive) {
		reply.input("? ");

		//entering ESC anywhere in the input causes "no response"
		if (reply.index("\x1B"))
			reply = 0;

		//reply must be numeric in range
		if (!reply.isnum())
			goto inp;
	}

	//no input means use default which might be zero
	if (reply == "")
		reply = defaultreply;

	//reply must be integer
	reply = int(reply);

	//zero or empty response
	if (!reply)
		return "";

	//reply must be 0 to noptions
	if (reply < 0 || reply > noptions) {
		if (interactive)
			goto inp;
		throw MVError(questionx ^ " Default reply must be 0-" ^ noptions);
	}

	return options.a(reply);
}

void ExodusProgramBase::savescreen(var& origscrn, var& origattr) const {
	std::cout << "ExodusProgramBase::savescreen not implemented" << std::endl;

	// evade warning: unused parameter
	if (origscrn || origattr) {
	}
}

var ExodusProgramBase::keypressed(int milliseconds) const {
	return var().hasinput(milliseconds);
}

bool ExodusProgramBase::esctoexit() const {
	if (not keypressed())
		return false;

	//std::cout << "Press Esc again to cancel or any other key to continue " << std::endl;
	std::cout << " Paused. Continue? (Y/n) ";
	std::cout << std::flush;

	//wait for ever
	var key;
	key.input();

	//key.outputl("key=");
	//key.oconv("HEX").outputl("key=");

	return key[-1].ucase() == "N";
}

var ExodusProgramBase::otherusers(const var& param) {
	std::cout << "ExodusProgramBase::otherusers not implemented yet";
	return var("");

	// evade warning: unused parameter
	if (param) {
	}
}

var ExodusProgramBase::otherdatasetusers(const var& param) {
	std::cout << "ExodusProgramBase::otherdatausers not implemented yet";
	return var("");

	// evade warning: unused parameter
	if (param) {
	}
}

bool ExodusProgramBase::lockrecord(const var& filename, var& file, const var& keyx) const {
	var record;
	return (bool)lockrecord(filename, file, keyx, record);
}

bool ExodusProgramBase::lockrecord(const var& filename, var& file, const var& keyx, const var& recordx, const int waitsecs0, const bool allowduplicate) const {

	// linemark
	// common /shadow.mfs/
	// shfilenames,shhandles,shadow.file,shsession.no,locks,spare1,spare2,spare3

	// nb case where we do not wish to wai
	// wait
	// 0 fail immediately (eg looking for next batch/nextkey)
	// 9 wait for 9 seconds
	//- wait for a default number of seconds
	// * wait infinite number of seconds
	// if index(file,'message',1) else de bug
	int waitsecs = waitsecs0;

	if (file.unassigned()) {
		if (not file.open(filename)) {
			call mssg(filename.quote() ^ " cannot be opened in LOCKRECORD " ^ keyx);
			var().abort();
		}
	}

lock:
	var locked = file.lock(keyx);
	if (locked || (allowduplicate && locked eq "")) {

		// fail if unexpired persistent lock exists in LOCKS file
		// on the same connection as file

		var locks;
		if (locks.open("LOCKS", file)) {

			var filename_for_locks = (filename.assigned() && filename) ? filename : file.a(1);
			var lockfilekey = filename_for_locks ^ "*" ^ keyx;

			var lockrec;
			if (lockrec.read(locks, lockfilekey)) {

				//current dos time
				//convert to Windows based date/time (ndays since 1/1/1900)
				//31/12/67 in rev date() format equals 24837 in windows date format
				var dostime = var().ostime();
				dostime = 24837 + var().date() + dostime / 24 / 3600;

				//remove lock if expired
				if (lockrec.a(1) <= dostime) {
					locks.deleterecord(lockfilekey);
					lockrec = "";
				}

				//or release any absolute lock
				//and return indication of lock failure
				else {
					if (locked)
						file.unlock(keyx);
					return 0;
				}
			}
		}

		return 1;
	} else {
		if (waitsecs) {
			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return false;
	}

	// evade warning: unused parameter
	(false || filename || recordx);

	return true;
}

var ExodusProgramBase::singular(const var& pluralnoun) {

	var temp = pluralnoun;

	if (temp.substr(-2, 2) == "ES") {

		// companies=company
		if (temp.substr(-3, 3) == "IES") {
			temp.splicer(-3, 3, "Y");

			// addresses=address
		} else if (temp.substr(-4, 4) == "SSES") {
			temp.splicer(-2, 2, "");
		} else if (temp.substr(-4, 4) == "SHES") {
			temp.splicer(-2, 2, "");
		} else if (temp.substr(-4, 4) == "CHES") {
			temp.splicer(-2, 2, "");
		} else if (1) {
			temp.splicer(-1, 1, "");
		}
	} else {

		if (temp[-1] == "S") {
			// analysis, dos
			if (temp.substr(-2, 2) != "IS" && temp.substr(-2, 2) != "OS")
				temp.splicer(-1, 1, "");
		}
	}

	return temp;
}

void ExodusProgramBase::flushindex(const var& /*filename*/) {
	//std::cout << "ExodusProgramBase::std::flushindex not implemented yet, " << filename
	//	  << std::endl;
	return;
}

var ExodusProgramBase::encrypt2(const var& encrypt0) const {

	var encrypt = encrypt0;
	var encryptkey = 1234567;

	// pass1
	while (true) {
		// BREAK;
		if (!(encrypt != ""))
			break;
		;
		encryptkey = (encryptkey % 390001) * (var(encrypt[1])).seq() + 1;
		encrypt.splicer(1, 1, "");
	}  // loop;

	// pass2
	while (true) {
		encrypt ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
		// BREAK;
		if (!encryptkey)
			break;
		;
	}  // loop;

	return encrypt;
}

var ExodusProgramBase::xmlquote(const var& string0) const {

	var string1;

	if (string0.unassigned()) {
		// de bug
		string1 = "UNASSIGNED";
	} else {
		string1 = string0;
	}

	string1.swapper("&", "&amp;");
	string1.swapper(DQ, "&quot;");
	string1.swapper("<", "&lt;");
	string1.swapper(">", "&gt;");

	string1.converter(DQ, "\'");
	string1.swapper(VM, "\" \"");
	string1.swapper(FM, "\" \"");
	return string1.quote();
}

var ExodusProgramBase::loginnet(const var& dataset, const var& username, var& cookie, var& msg) {

	// evade warning: unused parameter
	if (false && dataset) {
	}

	var menuid;
	var usern;
	var menun;
	var xx;

	// this is a custom login routine called from listen2
	cookie = "";
	var menus;
	if (!menus.open("ADMENUS")) {
		if (!menus.open("MENUS") && username != "EXODUS") {
			msg = "Error: Cannot open MENUS file";
			return false;
		}
	}

	// return allowable menus
	if (username == "EXODUS") {
		menuid = "ADAGENCY";
	} else {
		if (!(SECURITY.a(1).locate(username, usern))) {
			msg = "Error: " ^ username.quote() ^ " user is missing";
			return false;
		}
		menuid = SECURITY.a(3, usern);
	}

	var menu = "";
	if (!menu.read(menus, menuid)) {
		if (username == "EXODUS") {
			if (!menu.read(menus, "EXODUS")) {
				menu = FM ^ FM ^ FM ^ FM ^ FM ^
					   "MEDIA|ADPRODUCTION|ACCS|ANALMENU|TIMESHEETS|FILESMENU|"
					   "GENERAL|EXIT2";
				menu = menu.converter("|", VM);
			}
		}
	}
	if (!menu) {
		msg = "Error: " ^ menuid.quote() ^ " menu is missing";
		return false;
	}

	var menucodes = menu.a(6) ^ VM ^ "HELP";
	// remove local support menu
	if (!authorised("SUPPORT MENU ACCESS", msg, "LS")) {
		if (menucodes.a(1).locate("GENERA", menun))
			// menucodes.eraser(1, menun, 0);
			menucodes.remover(1, menun, 0);
	}
	menucodes.converter(VM ^ ".", ",_");

	// prepare session cookie
	cookie = "m=" ^ menucodes;

	return true;

	/* custom login per application
	 var compcode = "";

	 var temp;
	 if (!(temp.read(companies, "%RECORDS%"))) {
	 companies.select();
	 temp = "";
	 nextcomp:
	 var compcodex;
	 if (var("").readnext(compcodex)) {
	 temp.pickreplace(-1, 0, 0, compcodex);
	 goto nextcomp;
	 }
	 }

	 if (APPLICATION ne "ACCOUNTS") {
	 for (int ii = 1; ii <= 9999; ii++) {
	 compcode = temp.a(ii);
	 //until validcode('company',compcode)
	 //BREAK;
	 if (validcode2(compcode, "", "")) break;;
	 };//ii;
	 }else{
	 compcode = temp.a(1);
	 }

	 if (!compcode) {
	 msg = "Error: You are not authorised to access any companies";
	 return false;
	 }

	 var tempcompany;
	 if (!tempcompany.read(companies, compcode)) {
	 msg = "Error: " ^ compcode.quote() ^ " company code is missing";
	 return;
	 }

	 company = "";
	 initcompany(compcode);

	 force error here TODO: check trigraph following;
	 var defmarketcode = (company.a(30)) ? (company.a(30)) : (agp.a(37));
	 //if unassigned(markets) then markets=''

	 //markets is not open in finance only module
	 //readv maincurrcode from markets,defmarketcode,5 else maincurrcode=''
	 var maincurrcode = "";
	 if (FILES[0].locateusing(FM,"MARKETS", FM))
	 maincurrcode = defmarketcode.xlate("MARKETS", 5, "X");

	 if (maincurrcode.unassigned())
	 maincurrcode = "";
	 if (maincurrcode == "")
	 maincurrcode = basecurrency;

	 cookie ^= "&cc=" ^ compcode;
	 cookie ^= "&pd=" ^ currperiod ^ "/" ^ addcent(curryear);
	 cookie ^= "&bc=" ^ basecurrency;
	 cookie ^= "&bf=" ^ USER2;
	 cookie ^= "&mk=" ^ defmarketcode;
	 cookie ^= "&mc=" ^ maincurrcode;
	 temp = SYSTEM.a(23);
	 temp.swap("&", " and ");
	 cookie ^= "&db=" ^ temp;

	 backupreminder(dataset, msg);

	 changelogsubs("WHATSNEW" ^ FM ^ menucodes);
	 cookie ^= "&wn=" ^ ANS;

	 */
}

var ExodusProgramBase::AT(const int code) const {
	// should depend on terminal type
	return var().at(code);
}

var ExodusProgramBase::AT(const int x, const int y) const {
	// should depend on terminal type
	return var().at(x, y);
}

var ExodusProgramBase::handlefilename(const var& handle) {
	return handle.a(1);
}

var ExodusProgramBase::memspace([[maybe_unused]] const var& requiredmemory) {
	return 999999999;
}

var ExodusProgramBase::getuserdept(const var& usercode) {
	// locate the user in the list of users
	var usern;
	if (!(SECURITY.a(1).locate(usercode, usern))) {
		if (usercode == "EXODUS") {
			ANS = "EXODUS";
			return ANS;
		} else {
			ANS = "";
			return ANS;
		}
	}

	// locate divider, or usern+1
	var nusers = (SECURITY.a(1)).count(VM) + 1;
	var usernx;
	for (usernx = 1; usernx <= nusers; usernx++) {
		// BREAK;
		if (SECURITY.a(1, usernx) == "---")
			break;
		;
	};	// usern;

	// get the department code
	ANS = SECURITY.a(1, usernx - 1);

	return ANS;
}

var ExodusProgramBase::oconv(const var& input0, const var& conversion) {

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input0;

	//nothing in, nothing out
	//unfortutely conversions like L# do convert ""
	//if (result.length() == 0)
	//	return result;

	var ptr = 1;
	var delimiter;
	do {

		// var subconversion=conversion.remove(ptr,delimiter);
		var subconversion = conversion.substr2(ptr, delimiter);

		// EITHER call standard conversion methods
		if (subconversion[1] != "[") {
			result = result.oconv(subconversion);
		}

		else
		// OR call custom conversion routines
		// unlike pickos, custom routines are reponsible
		// for processing and returning multiple values
		{

			// var("*").logput();
			// return 1;

			//for speed dont call custom conversions on empty strings
			//nothing in, nothing out
			//Unfortunately [taghtml,td] produces output from nothing
			//if (result.length() == 0)
			//	continue;

			//remove brackets
			subconversion.substrer(2);
			if (subconversion[-1] == "]")
				subconversion.splicer(-1, 1, "");

			//determine the function name
			var functionname = subconversion.field(",").lcase();

			// extract any params
			var mode = subconversion.substr(functionname.length() + 2);

			var output;

			//custom function "[NUMBER]" actually has a built in function
			if (functionname == "number") {

				gosub number("OCONV", result, mode, output);

			} else {

				// set the function name
				ioconv_custom = functionname;

				// wire in the current environment
				ioconv_custom.mv_ = &mv;

				// and call it
				//call ioconv_custom("OCONV", result, mode, output);

				//call it once per field (any field mark RM-STM are preserved)
				var posn = 1;
				var ifield, ofield, delim;
				output = "";
				while (true) {

					ifield = result.substr2(posn, delim);

					call ioconv_custom("OCONV", ifield, mode, ofield);

					output ^= ofield;

					if (not delim)
						break;

					output ^= var().chr(RM.seq() + 1 - delim);
				}
			}
			result = output;
		}
	} while (delimiter);

	return result;
}

var ExodusProgramBase::iconv(const var& input, const var& conversion) {

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input;
	var ptr = 1;
	var delimiter;
	do {

		// var subconversion=conversion.remove(ptr,delimiter);
		var subconversion = conversion.substr2(ptr, delimiter);

		// or call standard conversion methods
		if (subconversion[1] != "[") {

			result = result.iconv(subconversion);

		// or call custom conversion routines
		} else {

			//determine the function name
			var functionname = subconversion.substr(2).field(",", 1).field("]", 1).lcase();;

			// extract any params
			var mode = subconversion.field(",", 2, 9999).field("]", 1);

			var output;

			//custom function "[NUMBER]" actually has a built in function
            if (functionname == "number") {
                gosub number("ICONV", result, mode, output);

            } else {

				// set the function name
				ioconv_custom = functionname;

				// wire up the current environment
				ioconv_custom.mv_ = (&mv);

				// and call it
				call ioconv_custom("ICONV", result, mode, output);
			}

			result = output;
		}
	} while (delimiter);

	return result;
}

var ExodusProgramBase::invertarray(const var& input, const var& force0 /*=0*/) {
	//c sys in,=(0)

	var force = force0.unassigned() ? var(0) : force0;

	var output = "";
	var nfs = input.count(FM) + (input ne "");
	//for force to work, the first field must have full number of vns
	var maxnvs = 0;
	for (var fn = 1; fn <= nfs; ++fn) {
		var fieldx = input.field(FM, fn);
		if (fieldx.length() or force) {
			var nvs = fieldx.count(VM) + 1;
			if (force) {
				if (nvs > maxnvs) {
					maxnvs = nvs;
				}
			} else {
				maxnvs = nvs;
			}
			for (var vn = 1; vn <= maxnvs; ++vn) {
				var cell = fieldx.field(VM, vn);
				if (cell.length() or force) {
					output.r(vn, fn, cell);
				}
			};	//vn;
		}
	};	//fn;

	return output;
}

//automatic upto date/time
var ExodusProgramBase::elapsedtimetext(const var& fromdate, const var& fromtime) {
	var uptodate, uptotime;
	return elapsedtimetext(fromdate, fromtime, uptodate, uptotime);
}

//given from and to
var ExodusProgramBase::elapsedtimetext(const var& fromdate, const var& fromtime, var& uptodate, var& uptotime) {
	//c sys in,in,io,io

	var text = "";

	if (uptodate.unassigned()) {
		uptodate = var().date();
	}
	if (uptotime.unassigned()) {
		uptotime = var().ostime();
	}
	//uptodate=date()
	//CALL DOSTIME(uptotime)

	//NSECS=INT(uptotime-fromTIME)
	var nsecs = uptotime - fromtime;
	//IF NSECS ELSE NSECS=1
	//uptodate=date()
	if (fromdate ne uptodate) {
		nsecs += (uptodate - fromdate) * 24 * 3600;
	}

	//cater for bug where start date isnt known and time has crossed midnight
	//so the 2nd time is less than the first
	if (nsecs < 0) {
		nsecs += 86400;
	}

	var weeks = (nsecs / 604800).floor();
	nsecs -= weeks * 604800;

	var days = (nsecs / 86400).floor();
	nsecs -= days * 86400;

	var hours = (nsecs / 3600).floor();
	nsecs -= hours * 3600;

	var minutes = (nsecs / 60).floor();
	nsecs -= minutes * 60;

	if (weeks) {
		text.r(-1, weeks ^ " week");
		if (weeks ne 1) {
			text ^= "s";
		}
	}
	if (days) {
		text.r(-1, days ^ " day");
		if (days ne 1) {
			text ^= "s";
		}
	}
	if (hours) {
		text.r(-1, hours ^ " hour");
		if (hours ne 1) {
			text ^= "s";
		}
	}
	if (minutes) {
		text.r(-1, minutes ^ " min");
		if (minutes ne 1) {
			text ^= "s";
		}
	}
	if (not(hours) and (minutes < 5)) {
		if (nsecs) {
			if (minutes or (nsecs - 10 > 0)) {
				nsecs = nsecs.oconv("MD00P");
			} else {
				nsecs = (nsecs.oconv("MD40P")) + 0;
				if (nsecs[1] == ".") {
					nsecs.splicer(1, 0, "0");
				}
			}
			if (nsecs) {
				text.r(-1, nsecs ^ " sec");
				if (nsecs ne 1) {
					text ^= "s";
				}
			} else if (not(minutes)) {
zero:
				text.r(-1, "< 1 msec");
			} else {
				text.r(-1, "exactly");
			}
		} else {
			if (not(minutes)) {
				goto zero;
			}
			text.r(3, "exactly");
		}
	}

	text.swapper(FM ^ FM ^ FM, FM);
	text.swapper(FM ^ FM, FM);
	text.swapper(FM, ", ");

	return text;
}

var ExodusProgramBase::number(const var& type, const var& input0, const var& ndecs0, var& output) {
	//function main(in type, in input0, in ndecs0, out output) {
	//c xxx in,in,in,out

	//WARNING IF YOU CHANGE THIS THEN ADECOM GOES TO MODULE xxx and libexodus
	////////////////////////////////////////////////////////////////////////
	//where exodus/service/src/getpickos will copy
	//~/pickos/xxx/number.cpp to ~/exodus/cli/src

	var fmtx;
	var input1;	 //num
	var delim;	 //num
	var output1;

	var ndecs = ndecs0;
	var input = input0;
	output = "";

	var zz = "";
	if (ndecs.index("Z")) {
		ndecs.converter("Z", "");
		zz = "Z";
	}

	if (type eq "ICONV") {
		var reciprocal = 0;
		if (input[1] eq "/") {
			reciprocal = 1;
			input.splicer(1, 1, "");
		} else {
			if (input.substr(1, 2) eq "1/") {
				reciprocal = 1;
				input.splicer(1, 2, "");
			}
		}

		output = input.trim();

		//first get into a pickos number with dots not commas
		if (BASEFMT.substr(1, 2) eq "MC") {
			output.converter(",", ".");
		} else {
			output.converter(",", "");
		}
		//nb [NUMBER,X] means no decimal place conversion to be done
		//if ndecs is given then convert to that number of decimals
		// if ndecs starts with a digit then use {NDECS} (use 2 if {NDECS}=null)
		if (ndecs eq "") {
			if (DICT) {
				ndecs = calculate("NDECS");
			} else {
				ndecs = BASEFMT[3];
			}
			if (ndecs eq "") {
				ndecs = 2;
			}
			if (not(ndecs.match("^\\d$"))) {
			}
			//FMTX='MD':NDECS:'0P'
			//OUTPUT=OUTPUT FMTX
		}
		if (ndecs eq "*" or ndecs eq "X") {
			ndecs = output.field(".", 2).length();
		}
		if (ndecs eq "BASE") {
			fmtx = "MD" ^ BASEFMT[3] ^ "0P";
		} else {
			fmtx = "MD" ^ ndecs ^ "0P";
		}
		output = oconv(output, fmtx);
		STATUS = "";
		if (output.isnum()) {
			if (reciprocal and output) {
				output = ((1 / output).oconv("MD90P")) + 0;
			}
		} else {
			STATUS = 2;
		}

		return 0;
	}

	//oconv
	//////

	var divx = ndecs.field(",", 2);
	if (divx.length()) {
		ndecs = ndecs.field(",", 1);
	}

	var posn = 1;
	while (true) {
		input1 = input.substr2(posn, delim);

		var perc = input1[-1];
		if (perc eq "%") {
			input1.splicer(-1, 1, "");
		} else {
			perc = "";
		}
		var plus = input1[1];
		if (plus eq "+") {
			input1.splicer(1, 1, "");
		} else {
			plus = "";
		}

		if (input.length()) {

			if (divx) {
				input1 = input1 / var(10).pwr(divx);
			}

			if (input1.index("E-")) {
				if (input1.isnum()) {
					input1 = input1.oconv("MD90P");
				}
			}

			var temp = input1;
			temp.converter("0123456789-.", "            ");
			var numlen = input1.length() - temp.trimf().length();
			var unitx = input1.substr(numlen + 1, 99);
			var numx = input1.substr(1, numlen);

			if (ndecs eq "BASE") {
				output1 = oconv(numx, BASEFMT ^ zz) ^ unitx;
			} else {
				//if ndecs='' then ndecs=len(field(numx,'.',2))
				//!ndecs could be X to mean no conversion at all!
				//FMTX=@USER2[1,2]:ndecs:'0P,':z
				if (ndecs eq "") {
					fmtx = BASEFMT.substr(1, 2) ^ numx.field(".", 2).length() ^ "0P," ^ zz;
				} else {
					fmtx = BASEFMT.substr(1, 2) ^ ndecs ^ "0P," ^ zz;
				}
				if (numx.isnum()) {
					numx += 0;
				}
				output1 = oconv(numx, fmtx) ^ unitx;

			}

			if (output1.length()) {
				if (var(".,").count(output1[1])) {
					output1.splicer(1, 0, "0");
				}
			}

			output ^= plus ^ output1 ^ perc;
		}

		///BREAK;
		if (not delim)
			break;
		//output:=char(256-delim)
		output ^= var().chr(RM.seq() + 1 - delim);
	}  //loop;

	return 0;
}

void ExodusProgramBase::sortarray(var& array, const var& fns, const var& orderby0) {
	//function main(io array, in fns=0, in orderby0="") {
	//c sys io,0,""

	//sorts one or more parallel mved fields in a record

	//array is the record which the field(s) to be sorted are present
	//fns is a mv array of parallel field numbers to be sorted
	//order is D or A (can be repeated to sort for multiple sorts per fn)
	//eg AA would sort the first two fns ascending
	//justification is L or R for left (text) or right (numberic)

	//MULTIPLE SORTS NOT SUPPORTED CURRENTLY
	//MULTIPLE FIELDS WILL BE SORTED IN PARALLEL ON THE FIRST FIELD ONLY

	var orderby;
	var fn;
	var success;
	var newvn;

	if (orderby0.unassigned()) {
		orderby = "AL";
	} else {
		orderby = orderby0;
	}

	var nfns = fns.count(VM) + 1;

	var sortfn = fns.a(1, 1);
	var unsorted = array.a(sortfn);
	var sorted = "";

	//insert into a new array without other fields for speed
	var newarray = "";
	var nv = unsorted.count(VM) + (unsorted ne "");
	for (var vn = 1; vn <= nv; ++vn) {
		var value = unsorted.a(1, vn);
		if (not(sorted.locateby(orderby, value, newvn))) {
			{
			}
		}
		sorted.inserter(1, newvn, value);

		//insert any parallel fields
		for (var fnn = 2; fnn <= nfns; ++fnn) {
			fn = fns.a(1, fnn);
			var othervalue = array.a(fn, vn);
			newarray.inserter(fn, newvn, othervalue);
		};	//fnn;

	};	//vn;

	array.r(sortfn, sorted);

	//put any parallel fields back into the original array
	for (var fnn = 2; fnn <= nfns; ++fnn) {
		fn = fns.a(1, fnn);
		array.r(fn, newarray.a(fn));
	}  //fnn;

	return;
}

}  // namespace exodus
