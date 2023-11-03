#include <exodus/exoprog.h>

#include <exodus/exoimpl.h>

// Allows and *requires* coding style like exodus application programming
// e.g. must use USERNAME not mv.USERNAME
#include <exodus/exomacros.h>

// Putting various member functions into all exodus programs allows access to the mv environment
// variable which is also available in all exodus programs.

//#include <unordered_map>

namespace exodus {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"

///////////////////////////
// Special Member Functions
///////////////////////////

// 2. Destructor
ExodusProgramBase::~ExodusProgramBase(){}

var ExodusProgramBase::libinfo(CVR command) {
	return var(perform_callable_.libfilepath(command.toString())).osfile();
}

/////////////////////
// Other Constructors
/////////////////////

// Constructor from an ExoEnv
// mv is a reference to allow mv.xxxxxx syntax instead of mv->xxxxxx
// However, being a reference, it must be provided at ExodusProgram construction time
// and cannot be changed thereafter.
ExodusProgramBase::ExodusProgramBase(ExoEnv& inmv)
	:
	perform_callable_(inmv),
	mv(inmv)
{
	cached_dictid_ = "";
	dict_callable_ = nullptr;
}

#pragma GCC diagnostic pop

//////////////////
// Other functions
//////////////////

// select
bool ExodusProgramBase::select(CVR sortselectclause_or_filehandle) {

	//TRACE(sortselectclause_or_filehandle)

	//simple select on filehandle
	if (sortselectclause_or_filehandle.contains(FM)) {
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
	var calc_fields = CURSOR.f(10);
	if (!calc_fields) {
		return true;
		////////////
	}

	//stage 2 "2stage"
	//////////////////

	CURSOR.convert(_ALL_FMS, _VISIBLE_FMS).logputl("2 Stage Select:");
	sortselectclause.logputl();

	//secondary sort/select on fields that could not be calculated by the database

	//calc_fields.oswrite("calc_fields=");

	//clear the list of calculated fields
	CURSOR.r(10, "");

	//vms to fms etc in calculated fields
	calc_fields.raiser();

	//ONLY TEST MATERIALS FOR NOW
	//if (!calc_fields.ucase().contains("MATERIALS"))
	//	return true;

	//debug
	//calc_fields.convert(FM^VM^SM,"   ").logputl("calc=");

	var dictfilename = calc_fields.f(5, 1);

	//debugging
	var calc_fields_file;
	if (!calc_fields_file.open("calc_fields"))
		calc_fields_file = "";

	//open the dictionary
	if (not dictfilename.lcase().starts("dict."))
		dictfilename = "dict." ^ dictfilename;
	if (!DICT.open(dictfilename)) {
		dictfilename = "dict.voc";
		if (!DICT.open(dictfilename)) {
			//throw VarDBException(dictfilename.quote() ^ " cannot be opened");
			[[unlikely]]
			throw VarError(dictfilename.quote() ^ " cannot be opened");
		}
	}

	//prepare to create a temporary sql table
	//DROP TABLE IF EXISTS SELECT_STAGE2;
	//CREATE TABLE SELECT_STAGE2(
	// KEY TEXT PRIMARY KEY,
	// EXECUTIVE_CODE TEXT)
	var temptablename = "SELECT_CURSOR_STAGE2_" ^ CURSOR.f(1);
	var createtablesql = "";
	createtablesql ^= "DROP TABLE IF EXISTS " ^ temptablename ^ ";\n";
	//createtablesql ^= "CREATE TABLE " ^ temptablename ^ "(\n";
	createtablesql ^= "CREATE TEMPORARY TABLE IF NOT EXISTS " ^ temptablename ^ "(\n";
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
	int nfields = calc_fields.f(1).fcount(VM);
	dim dictids(nfields);
	dim opnos(nfields);
	dim reqivalues(nfields);
	dim reqivalues2(nfields);
	dim ioconvs(nfields);
	dim sqltypes(nfields);

	for (int fieldn = 1; fieldn <= nfields; ++fieldn) {

		//dictids

		var dictid = calc_fields.f(1, fieldn);

		var dictrec;
		if (not dictrec.readc(DICT,dictid))
			dictrec="";
		ioconvs(fieldn) = dictrec.f(7);

		//add colons to the end of every calculated field in the sselect clause
		//so that 2nd stage select knows that these fields are available in the
		//temporary parallel file
		sortselectclause.regex_replacer("\\b" ^ dictid ^ "\\b", dictid ^ ":");

		dictid.converter(".", "_");
		dictids(fieldn) = dictid;
		var sqlcolid = dictid ^ "_calc";

		//ops

		var ovalue = calc_fields.f(3, fieldn).convert(SM, VM).unquote();

		var op = calc_fields.f(2, fieldn);

		//multivalued selections are not well supported from mvdbpostgresql. handle the obvious cases"
		if (ovalue.contains(VM)) {
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
			[[unlikely]]
			throw VarError(op.quote() ^ " unknown op in sql select");
		opnos(fieldn) = opno;

		//reqivalues
		if (op == "in" and ovalue.starts("(") and ovalue.ends(")")) {
			ovalue.cutter(1).popper();
			ovalue.replacer("', '", VM);
			ovalue.trimmerboth().unquoter();
			//ovalue.convert(VM,"]").logputl("ovalue=");
		}

		var ovalue2 = calc_fields.f(4, fieldn).unquote();

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
		//if (ioconv.starts("[DATETIME")) {
		//	sqltype = "DATE";//TODO
		//}
		if (ioconv.starts("[DATE")) {
			sqltype = "DATE";
            ioconvs(fieldn) = "D";
		}
		else if (ioconv.starts("[TIME")) {
			sqltype = "TIME";//TODO check if works
		}
		// Note that we aslo assume numeric if a filter value is present and is numeric (without quotes)
		else if (ioconv.starts("[NUMBER") or (ovalue.len() and ovalue.isnum())) {
			sqltype = "NUMERIC";
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
		//var("").logputl();

		//debug
		if (calc_fields_file && dictid != "AUTHORISED") {
			var key = dictfilename ^ "*" ^ dictid;
			var rec = sortselectclause ^ FM ^ op ^ FM ^ ovalue ^ FM ^ ovalue2;
			rec.write(calc_fields_file, key);
			key.logput("written to calc_fields ");
		}
	}

	if (baseinsertsql.ends(",")) {

		baseinsertsql.popper() ^= ")";
		baseinsertsql ^= " VALUES (";

		createtablesql.popper() ^= ")";
		//createtablesql.logputl();

		//clear the table in case we are reusing it
		createtablesql ^= ";DELETE FROM " ^ temptablename ^ ";\n";

		//create the temporary table
		if (!CURSOR.sqlexec(createtablesql))
			[[unlikely]]
			throw VarError("ExodusProgramBase::select " ^ var().lasterror());

	} else
		baseinsertsql = "";

	int maxnrecs = calc_fields.f(6);
	int recn = 0;

	//nextrecord:
	while (CURSOR.readnext(RECORD, ID, MV)) {

		bool allok = true;

		//var id2 = MV ? (ID ^ "*" ^ MV) : ID;
		var insertsql = baseinsertsql ^ ID.replacer("'", "''").squote() ^ ",";

		//some or all fields may have filters on them
		//some or all fields may not have filters, if 'order by' calculated fields
		for (int fieldn = 1; fieldn <= nfields; ++fieldn) {

			var ivalues = calculate(dictids(fieldn));
//			TRACE(ivalues)
//			TRACE(opnos.join())
//			TRACE(fieldn)
//			TRACE(opnos(fieldn))
			if (opnos(fieldn)) {
				//debug
				//ivalues.logputl(dictids(fieldn) ^ " ivalues=");

				var nextsep;
				var pos = 1;
				bool ok = false;

				while (true) {

					var ivalue = ivalues.substr2(pos,nextsep);
//					TRACE(ivalue)

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
							//ok = !!ivalue;
//							TRACE(!!ivalue)
							break;
						case 16:  // is false (isnt true)
							ok = !ivalue;
							break;
						case 17:  // STARTING ]
							ok = reqivalues(fieldn).starts(ivalue);
							break;
						case 18:  // ENDING [
							ok = reqivalues(fieldn).ends(ivalue);
							ok = !ivalue;
							break;
						case 19:  //  CONTAINING []
							ok = reqivalues(fieldn).contains(ivalue);
							break;
					}//switch

					//quit searching data values if found or no more
//					TRACE(ok)
					if (ok || !nextsep)
						break;

				}//for each ivalue

				//if a field fails test then skip and remaining fields and readnext candidate record
				if (!ok) {
					//debug
					//ivalue.logputl("FAILED=");
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
					//ovalues = "nullptr";
					ovalues = "null";
			}
			else {
				//ivalueS (41472, 'Practical PostgreSQL', 1212, 4);
				ovalues = ivalues.replace("'", "''").squoter();
			}

			insertsql ^= " " ^ ovalues ^ ",";

		}//fieldn

		//skip if failed to match
		if (!allok) {
//			ID.logputl("stage 1/2 failed ");
			continue;
		}

//		ID.logputl("stage 1/2 ok ");

		// Replace final comma with a closing bracket and additional SQL
		// Ignore any duplicates due to multivalues
		// TODO insert stage2 records with MV as par of the key?
		insertsql.popper() ^= ") ON CONFLICT (key) DO NOTHING";

		//insertsql.logputl();

		if (not CURSOR.sqlexec(insertsql)) {
			//tolerate failure to write in case multiple records returned (due to mv selection?)
			if (not CURSOR.lasterror().contains("duplicate key"))
				CURSOR.lasterror().errputl("Error inserting pass2 record:");
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

// savelist
bool ExodusProgramBase::savelist(CVR listname) {
	return CURSOR.savelist(listname.field(" ", 1));
}

// getlist
bool ExodusProgramBase::getlist(CVR listname) {
	return CURSOR.getlist(listname.field(" ", 1));
}

// formlist
bool ExodusProgramBase::formlist(CVR filename_or_command, CVR keys /*=""*/, const var fieldno /*=0*/) {
	//remove any options from the filename or command
	var filename2 = filename_or_command;
	if (filename2.ends(")")) {
		var options = filename2.field2(" ", -1);
		filename2.cutter(-options.len()).trimmerlast();
	}

	//optionally get keys from filename or command
	var keys2 = (keys == "") ? filename2.field(" ", 2, 999999999) : keys;

	//remove any keys from the filename
	filename2 = filename2.field(" ", 1);

	//open the file
	clearselect();
	if (not CURSOR.open(filename2))
		[[unlikely]]
		throw VarError(filename2.quote() ^ " file cannot be opened in formlist(" ^ keys ^ ")");

	return CURSOR.formlist(keys2, fieldno);
}

// makelist
bool ExodusProgramBase::makelist(CVR listname, CVR keys) {
	return CURSOR.makelist(listname.field(" ", 1), keys);
}

// deletelist
bool ExodusProgramBase::deletelist(CVR listname) {
	return CURSOR.deletelist(listname.field(" ", 1));
}

// clearselect
void ExodusProgramBase::clearselect() {
	CURSOR.clearselect();
}

// hasnext
bool ExodusProgramBase::hasnext() {
	return CURSOR.hasnext();
}

// readnext 1
bool ExodusProgramBase::readnext(VARREF key) {
	return CURSOR.readnext(key);
}

// readnext 2
bool ExodusProgramBase::readnext(VARREF key, VARREF valueno) {
	return CURSOR.readnext(key, valueno);
}

// readnext 3
bool ExodusProgramBase::readnext(VARREF record, VARREF key, VARREF valueno) {
	return CURSOR.readnext(record, key, valueno);
}

// deleterecord
bool ExodusProgramBase::deleterecord(CVR filename_or_handle_or_command, CVR key) {

	if (not filename_or_handle_or_command.assigned() || not key.assigned())
		//throw VarUnassigned("bool ExodusProgramBase::deleterecord(CVR filename_or_handle_or_command, CVR key)");
		[[unlikely]]
		throw VarError("bool ExodusProgramBase::deleterecord(CVR filename_or_handle_or_command, CVR key)");

	// Simple deleterecord
	//if (filename_or_handle_or_command.contains(" ") || key.len() == 0) {
	if (not filename_or_handle_or_command.contains(" ") and key.len() != 0)
		return filename_or_handle_or_command.deleterecord(key);

	// Complex deleterecord command

	var command = filename_or_handle_or_command.f(1);

	var filename = command.field(" ", 1);

	//if any keys provided (remove quotes if present)
	int nwords = command.fcount(" ");

	//find and skip final options like (S)
	bool silent = false;
	if (command.ends(")") || command.ends("}")) {
		silent = command.field2(" ", -1).contains("S");
		nwords--;
	}

	if (nwords >= 2) {
		for (int wordn = 2; wordn <= nwords; ++wordn) {
			var key = command.field(" ", wordn).unquote();
			if (filename.deleterecord(key)) {
				silent || key.quote().logputl("Deleted ");
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
				silent || key.quote().logputl("Deleted ");
			} else {
				silent || key.quote().errputl("NOT deleted ");
			}
		}
	}
	return true;
	//return filehandle.deleterecord(key);
}

// pushselect
bool ExodusProgramBase::pushselect([[maybe_unused]] CVR v1, VARREF v2, [[maybe_unused]] VARREF v3, [[maybe_unused]] VARREF v4) {

	// CURSOR.quote().logputl("CURSOR=");
	// CURSOR++;
	// CURSOR has connection number hidden in it, so it cannot be used as an ordinary variable
	// ATM
	CURSOR.move(v2);
	CURSOR = var().date() ^ "_" ^ (var().ostime().convert(".", "_"));
	return true;
}

// popselect
bool ExodusProgramBase::popselect([[maybe_unused]] CVR v1, VARREF v2, [[maybe_unused]] VARREF v3, [[maybe_unused]] VARREF v4) {
	// CURSOR.quote().logputl("CURSOR=");
	// CURSOR--;
	v2.move(CURSOR);
	return true;
}

// note 2
void ExodusProgramBase::note(CVR msg, CVR options) const {
	var buffer = "";
	mssg(msg, options, buffer);
}

// note 4
void ExodusProgramBase::note(CVR msg, CVR options, VARREF buffer, CVR params) const {
	mssg(msg, options, buffer, params);
}

// mssg 2
void ExodusProgramBase::mssg(CVR msg, CVR options) const {
	var buffer = "";
	mssg(msg, options, buffer);
}

// mssg 4
void ExodusProgramBase::mssg(CVR msg, CVR options, VARREF buffer, CVR params) const {

	var interactive = !SYSTEM.f(33);
	if (interactive)
		std::cout << var("----------------------------------------") << std::endl;

	//skip if just "downing" a previous "upped" message
	if (options.contains("D")) {
		return;
	}

	//we must pass the message unmodified into USER4 below
	//e.g. to pass ACCOUNTLIST back to client with FM/VM etc
	//var msg1 = msg.convert("|" ^ FM ^ VM ^ SM, "\n\n\n\n").trim("\n");
	var msg1 = msg;

	//replace %1, %2 etc with params
	for (var ii = 1; ii <= 9; ++ii)
		msg1.replacer("%" ^ ii, params.f(ii));

	msg1.converter(_FM _VM "|", "\n\n\n").trimmer("\n");

	std::cout << msg1 << std::endl;

	var origbuffer = buffer.assigned() ? buffer : "";

	//R=Reply required in buffer
	if (options.contains("R")) {
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
			if (options.contains("E") and (buffer == "" or buffer.contains("\x1B")))
				buffer = "\x1B";  //esc

			std::cout << std::endl;
		} else {

			//input=output if not interactive
			buffer = origbuffer;
		}

		//force upper case
		if (options.contains("C"))
			buffer.ucaser();

		return;
	}

	if (!options.contains("U")) {
		if (USER4.len() > 8000) {
			var msg2 = "Aborted MSG()>8000";
			if (not USER4.contains(msg2)) {
				std::cout << msg2 << std::endl;
				//std::cout << USER4 << std::endl;
				USER4 ^= FM ^ msg2;
			}
		} else {
			USER4.r(-1, msg1);
		}
	}
}

// authorised 1
var ExodusProgramBase::authorised(CVR task0) {
	var msg;
	return authorised(task0, msg);
}

// authorised 4
var ExodusProgramBase::authorised(CVR task0, VARREF msg, CVR defaultlock, CVR username0) {

	var username;
	var msgusername;
	var taskn;	// num
	var taskn2;
	var xx;
	var usern;

	var task = task0;
	if (username0.unassigned() or username0 == "") {
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

	if (task.starts(" ")) {
		call mssg(DQ ^ (task0 ^ DQ));
	}
	// Each task may have many "locks", each users may have many "keys"
	// A user must have keys to all the locks in order to pass

	if (not task) {
		return 1;
	}

	task.ucaser();
	task.converter(RM ^ FM ^ VM ^ SM, "\\\\\\");
	task.replacer(" FILE ", " ");
	task.replacer(" - ", " ");
	task.converter(".", " ");
	task.trimmer();

	msg = "";
	// **CALL note(' ':TASK)

	if (task.starts("..")) {
		// call note(task:'')
		return 1;
	}

	var noadd = task.starts("!");
	if (noadd) {
		task.cutter(1);
	}
	// if noadd else NOADD=((TASK[-1,1]='"') and (len(userprivs)<10000))
	if (not noadd) {
		var lenuserprivs = SECURITY.len();
		noadd = task.ends(DQ) or lenuserprivs > 48000;
	}
	var positive = task[1];
	if (positive == "#")
		task.cutter(1);
	else
		positive = "";

	//? as first character of task (after positive) means
	// security is being used as a configuration and user EXODUS has no special privs
	var isadmin;
	if (task.starts("?")) {
		isadmin = 0;
		task.cutter(1);
	} else
		isadmin = username == "EXODUS";

	var deleting = task.starts("%DELETE%");
	if (deleting) {
		task.cutter(8);
	}
	var updating = task.starts("%UPDATE%");
	if (updating) {
		task.cutter(8);
	}
	var renaming = task.starts("%RENAME%");
	if (renaming) {
		task.cutter(8);
	}

	// find the task
	if (SECURITY.f(10).locate(task, taskn)) {
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
			if (SECURITY.f(10).locate(defaultlock, taskn2)) {
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
			if (SECURITY.f(10).locate(defaultlock, taskn2)) {
				tt = SECURITY.f(11, taskn2);
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
			// if (SECURITY.len() < 65000) {
			if (true) {
				var x = var();
				if (not(SECURITY.f(10).locateby("A", task, taskn))) {
					var newlock = defaultlock;
					// get locks on default task if present otherwise new locks
					// are none
					if (newlock and SECURITY.f(10).locate(newlock)) {
						newlock = SECURITY.f(11, xx);
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
	var locks = SECURITY.f(11, taskn);
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
	if (not(SECURITY.f(1).locate(username, usern))) {
		if (username != "EXODUS" and username != APPLICATION) {
			gosub readuserprivs();
			usern = (SECURITY.f(1)).fcount(VM) + 1;
			if (SECURITY.len() < 65000) {
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
	var keys = SECURITY.f(2).field(VM, usern, 999);
	var temp = keys.index("---");
	if (temp) {
		keys.paster(temp - 1, 999, "");
	}
	// convert ',' to vm in keys
	// convert ',' to vm in locks
	keys.converter("," _VM "", "  ");
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
	}	// lockn;

	// ok:
	// CALL STATUP(2,3,TASK)
	return 1;
}

// readuserprivs
void ExodusProgramBase::readuserprivs() const {
	if (not DEFINITIONS or not(SECURITY.read(DEFINITIONS, "SECURITY"))) {
		SECURITY = "";
	}
	return;
}

// writeuserprivs
void ExodusProgramBase::writeuserprivs() const {
	SECURITY.r(9, "");
	if (DEFINITIONS) {
		SECURITY.write(DEFINITIONS, "SECURITY");
	}
	return;
}

// capitalise
var ExodusProgramBase::capitalise(CVR str0, CVR mode0, CVR wordseps0) const {

	var string2;

	if (not str0) {
		string2 = "";
		return string2;
	}

	if (mode0.unassigned() || !mode0.len() || mode0 == "CAPITALISE") {
		string2 = str0;
		// convert @upper.case to @lower.case in string2
		int nn = string2.len();
		var numx = var("1234567890").contains(string2[1]);
		var cap = 1;
		var wordseps;
		var inquotes = 0;
		// wordseps=' /-.()&'
		if (wordseps0.unassigned())
			wordseps = " .()&_" _RM _FM _VM _SM _TM _ST;
		else
			wordseps = wordseps0;
		for (int ii = 1; ii <= nn; ii++) {
			var tt = string2.b(ii, 1);

			if (inquotes) {
				inquotes = tt != inquotes;
			} else {
				//if (tt == DQ && (string2.count(DQ) > 1 || tt == "\'") &&
				//	string2.count("\'") > 1) {
				if ((tt == DQ and string2.count(DQ) gt 1) or ((tt == "'" and string2.count("'") gt 1))) {
					inquotes = tt;
				} else {
					if (wordseps.contains(tt)) {
						cap = 1;
						if (tt == " ")
							//numx = var("1234567890").contains(string2.b(ii + 1, 1), 1);
							numx = var("1234567890").contains(string2[ii + 1]);
					} else {
						if (cap || numx) {
							tt.ucaser();
							string2.paster(ii, 1, tt);
							cap = 0;
						} else {
							tt.lcaser();
							string2.paster(ii, 1, tt);
						}
					}
				}
			}

		}	// ii;

		string2.replacer("\'S ", "\'s ");
		if (string2.ends("\'S"))
			string2.paster(-2, 2, "\'s");
	} else if (mode0 == "QUOTE") {
		string2 = str0;
		if (string2 != "") {
			string2.converter(FM ^ VM ^ SM ^ TM, "    ");
			string2.replacer(" ", "\" \"");
			string2 = string2.quote();
		}
	} else if (mode0 == "UPPERCASE") {
		string2 = str0;
		string2.ucaser();
	} else if (mode0 == "LOWERCASE") {
		string2 = str0;
		string2.lcaser();
	} else if (mode0.starts("PARSE")) {

		var uppercase = mode0.contains("UPPERCASE");

		string2 = str0;

		// convert to uppercase
		var quoted = "";
		for (int ii = 1; ii <= 99999; ii++) {
			//var tt = string2.b(ii, 1);
			var tt = string2[ii];
			// BREAK;
			if (!(tt != ""))
				break;
			if (tt == quoted) {
				quoted = "";
			} else {
				if (!quoted) {
					if ((DQ ^ "\'").contains(tt)) {
						quoted = tt;
					} else {
						if (tt == " ") {
							tt = FM;
							string2.paster(ii, 1, tt);
						} else {
							if (uppercase) {
								tt.ucaser();
								string2.paster(ii, 1, tt);
							}
						}
					}
				}
			}
		}	// ii;

		if (mode0.contains("TRIM")) {
			string2.converter(" " _FM, _FM " ");
			string2 = string2.trim();
			string2.converter(" " _FM, _FM " ");
		}
	}

	return string2;
}

// execute
var ExodusProgramBase::execute(CVR sentence) {

	var v1, v2, v3, v4;
	pushselect(v1, v2, v3, v4);

	var sentence2 = sentence.fieldstore(" ", 1, 1, sentence.field(" ", 1).lcase());

	var result = perform(sentence);

	popselect(v1, v2, v3, v4);

	return result;
}

// chain
[[noreturn]]
void ExodusProgramBase::chain(CVR libraryname) {
	CHAIN = libraryname;
	this->stop();
}

// perform
var ExodusProgramBase::perform(CVR sentence) {
	// THISIS("var ExodusProgramBase::perform(CVR sentence)")
	// ISSTRING(sentence)

	// return ID^"*"^dictid;

	// wire up the the library linker to have the current exoenv
	// if (!perform_callable_.mv_)
	//	perform_callable_.mv_=this;

	// lowercase all library functions to aid in conversion from pickos
	// TODO remove after conversion complete

	//perform_callable_.mv_ = (&mv);

	// Save some environment
	////////////////////////

	var savesentence = "";
	var savecommand = "";
	var saveoptions = "";
	var saverecur0 = "";
	var saverecur1 = "";
	var saverecur2 = "";
	var saverecur3 = "";
	var saverecur4 = "";
	//
	var saveid = "";
	var saverecord = "";
	var savemv = "";
	var savedict = "";

	//SENTENCE.swap(savesentence);
	// Must take a *COPY* since sentence and SENTENCE might be the same variable
	// if called like 'perform(SENTENCE)'
	savesentence = SENTENCE;

	COMMAND.swap(savecommand);
	OPTIONS.swap(saveoptions);
	RECUR0.swap(saverecur0);
	RECUR1.swap(saverecur1);
	RECUR2.swap(saverecur2);
	RECUR3.swap(saverecur3);
	RECUR4.swap(saverecur4);
	LEVEL++;
	//
	ID.swap(saveid);
	RECORD.swap(saverecord);
	MV.swap(savemv);
	DICT.swap(savedict);

	// A function to restore the environment
	////////////////////////////////////////
	auto restore_environment = [&]() {
		// restore some environment
		savesentence.swap(SENTENCE);
		savecommand.swap(COMMAND);
		saveoptions.swap(OPTIONS);
		saverecur0.swap(RECUR0);
		saverecur1.swap(RECUR1);
		saverecur2.swap(RECUR2);
		saverecur3.swap(RECUR3);
		saverecur4.swap(RECUR4);
		LEVEL--;
		//
		saveid.swap(ID);
		saverecord.swap(RECORD);
		savemv.swap(MV);
		savedict.swap(DICT);
	};

	// Do the perform
	/////////////////

	SENTENCE = sentence;
	while (SENTENCE) {
		COMMAND = SENTENCE.parse(' ');

		// Cut off OPTIONS from end of COMMAND if present
		// *** SIMILAR code in
		// 1. exofuncs.cpp exodus_main()
		// 2. exoprog.cpp  perform()
		// OPTIONS are in either (AbC) or {AbC} in the last field of COMMAND
		OPTIONS = COMMAND.field2(FM, -1);
		if ((OPTIONS.starts("{") and OPTIONS.ends("}")) or (OPTIONS.starts("(") and OPTIONS.ends(")"))) {
			// Remove last field of COMMAND. TODO fpopper command or remover(-1)?
			COMMAND.cutter(-OPTIONS.len() - 1);
			// Remove first { or ( and last ) } chars of OPTIONS
			OPTIONS.cutter(1).popper();
		} else {
			OPTIONS = "";
		}


		// Load the shared library file
		var libid = SENTENCE.field(" ", 1).lcase();
		std::string libname = libid.toString();
		if (!perform_callable_.initsmf(
											mv,
											libname.c_str(),
											"exodusprogrambasecreatedelete_",
											true  // forcenew each perform/execute
											)) {
			USER4 ^= "perform() Cannot find shared library \"" + libname +
					 "\", or \"libraryexit()\" is not present in it.";
			// [[unlikely]] throw VarError(USER4);
			// return "";
			break;
		}

		// Call the shared library exodus object's main function
		try {

			//ANS = perform_callable_.callsmf();
			var* retval = new var;
			*retval = perform_callable_.callsmf();

			// Detect corrupt return if we are lucky
			// If we are unlucky the corrupt data will look good
			// OK: var_typ will have only bottom 5 bits set
			// KO: var_typ will have other bits set
			// By throwing without deleting the new head var, we will avoid
			// segfault: free(): invalid pointer" when the var's std:string is "destructed"
			// but the memory will leak.
			if (retval->unassigned()) {
				retval->dump().errputl();
				restore_environment();
				[[unlikely]]
				throw VarError("exoprog::perform corrupt result. perform only functions, not subroutines.");
			}

			// Since the returned var appears OK, use it.
			ANS = *retval;

			// Since the returned var appears OK, delete it.
			// If retval was not actually returned by callsmf then we may get a segfault: free(): invalid pointer
			// from the fake var's std::string destructor
			delete retval;

		}

		// TODO reimplement this
//		} catch (const VarUndefined&) {
//			// if return "" is missing then default ANS to ""
//			ANS = "";

		// Stop
		catch (const MVStop&) {
			// stop is normal way of stopping a perform
			// functions can call it to terminate the whole "program"
			// without needing to setup chains of returns
			// to exit from nested functions
			ANS = "";
		}

		// Abort
		catch (const MVAbort& e) {
			// similar to stop for the time being
			// maybe it should set some error flag/messages
			mssg(e.description);
			ANS = "";
		}

		// AbortAll
		catch (const MVAbortAll&) {
			// similar to stop for the time being
			// maybe it should set some error flag/messages
			// and abort multiple levels of perform?
			ANS = "";
		}

		//TODO Create a second version of this whole try/catch block
		//that omits catch (VarError) if EXO_DEBUG is set
		//so that gdb will catch the original error and allow backtracing there
		//Until then, use gdb "catch throw" as mentioned below.
		catch (const VarError&) {
			//restore environment in case VarError is caught
			//in caller and the program resumes processing
			restore_environment();

			// Use gdb command "catch throw" to break at error line to get back traces there
			[[unlikely]]
			throw;
		}

		// Dont catch MVLogoff in perform/execute. leave it for exoprog top level exit
		//catch (const MVLogoff& e)
		//{
		//	// similar to stop for the time being
		//	// maybe it should set some error flag/messages
		//	// and abort multiple levels of perform?
		//	ANS = "";
		//}

		// CHAIN is a kind of optional follow-on controlled by the library function
		// to perform another function after the first
		// Go round again if anything in CHAIN
		CHAIN.move(SENTENCE);
	}

	// restore various environment variables
	restore_environment();

//	// Futile attempt to recover from performing a subroutine which returns void instead of var
//	// This is atrocious undefined behaviour in C++
//	// TODO check that we only callsmf on libraries that provide FUNCTION main
//	// OR call libraries with SUBROUTINE main with new function maybe callsubroutine
//	//if (ANS.undefined() or ANS.unassigned()) {
//	// unassigned also checks undefined i.e. illegal bits in ANS.vartyp
//	TRACE("QWE1")
//	if (ANS.unassigned()) {
//		TRACE("QWE2")
//		ANS = "";
//	}
//	var bad;
//	ANS.swap(bad);
//	TRACE("QWE3")
//	ANS = "";
//	TRACE("QWE4")

	return ANS;
}

// xlate
var ExodusProgramBase::xlate(CVR filename, CVR key, CVR fieldno_or_name, const char* mode) {

	// TODO implement additional MV argument

	// Key can be multivalued and will return a multivalued result
	var results = "";
	var nkeys = key.fcount(VM);

	var is_fieldno = fieldno_or_name.isnum();

	var dictfile;
	if (not is_fieldno) {
		if (not dictfile.open("dict." ^ filename.f(1))) {
			[[unlikely]]
			throw VarError("ExodusProgramBase::xlate(filename:" ^ filename ^ ", key:" ^ key ^ ",field:" ^ fieldno_or_name ^ ") - dict." ^ filename ^ " does not exist.");
		}
	}

	for (var keyn = 1; keyn <= nkeys; ++keyn) {

		var keyx = key.f(1, keyn);

		// If ordinary numeric or "" fieldno
		if (is_fieldno) {
			results.r(keyn, keyx.xlate(filename, fieldno_or_name, mode));
		}

		// Otherwise handle non-numeric field_no ie dictionary field/column name
		else {

			// Get the whole record
			var record = keyx.xlate(filename, "", mode);

			// TODO what if key is multivalued?

			// Handle record not found and mode C
			if (*mode == 'C' && record == keyx) {
				results.r(keyn, key);
				continue;
			}

			// Handle record not found and mode X
			if (not record.len()) {
				results.r(keyn, "");
				continue;
			}

			// Use calculate() with four parameters
			var result =
				calculate(fieldno_or_name, dictfile, keyx, record);
			if (nkeys > 1)
				result.lowerer();
			results.r(keyn, result);
		}

	}

	if (nkeys > 1)
		results.converter(_FM, _VM);
	//else
	//    sep = _RM;

	return results;
}

// calculate 4
var ExodusProgramBase::calculate(CVR dictid, CVR dictfile, CVR id, CVR record, CVR mvno) {

	//dictid @ID/@id is hard coded to return ID
	//to avoid incessant lookup in main file dictionary and then defaulting to dict.voc
	//sadly this means that @ID cannot be customised per file
	//possibly amend the read cache to cache the dict.voc version for the main file
	//if (dictid == "@ID" || dictid == "@id")
	//	return ID;

	// How is it possible to swap when the argument variable is const?
	// Because libexodus provides a const swap that isnt const just for calculate()
	// to be able to work rapidly swapping globals in and out of place TEMPORARILY.
	// and all it to work with const arguments.
	// TODO replace with const_cast?

	// Save and reconfigure the relevent environment variables
	DICT.swap(dictfile);
	ID.swap(id);
	RECORD.swap(record);
	MV.swap(mvno);

	// Call the normal calculate function
	var result = calculate(dictid);

	// Restore the environment variables
	DICT.swap(dictfile);
	ID.swap(id);
	RECORD.swap(record);
	MV.swap(mvno);

	return result;
}

// calculate 1
var ExodusProgramBase::calculate(CVR dictid) {
	// THISIS("var ExodusProgramBase::calculate(CVR dictid)")
	// ISSTRING(dictid)

	// return ID^"*"^dictid;

	// Get the dictionary record so we know how to extract the correct field
	// or call the right library
	bool indictvoc = false;
	bool newlibfunc = false;
	std::string curr_dictid = std::string(dictid) + _RM + DICT.f(1).toString();
	if (cached_dictid_ != curr_dictid) {
		cached_dictid_ = "";
		newlibfunc = true;

		if (not DICT)
			[[unlikely]]
			throw VarError("ExodusProgramBase::calculate(" ^ dictid ^
						  ") DICT file variable has not been set");

		if (not cached_dictrec_.readc(DICT, dictid)) {

			// try lower case
			if (not cached_dictrec_.readc(DICT, dictid.lcase())) {

				// try dict.voc
				var dictvoc;  // TODO implement mv.DICTVOC to avoid opening
				if (not dictvoc.open("dict.voc")) {
baddict:
					[[unlikely]]
					throw VarError("ExodusProgramBase::calculate(" ^ dictid ^
								  ") dictionary record not in DICT " ^
								  DICT.f(1).quote() ^ " nor in dict.voc");
				}
				if (not cached_dictrec_.readc(dictvoc, dictid)) {
					// try lower case
					if (not cached_dictrec_.readc(dictvoc, dictid.lcase())) {
						goto baddict;
					}
				}
				indictvoc = true;
			}
			// cache whichever dict record was found
			// regardless of file xxxxxxxx/voc and upper/lower case key
			// as if it was found in the initial file,key requested
			// this will save repeated drill down searching on every access.
			cached_dictrec_.r(16, indictvoc);
			cached_dictrec_.writec(DICT, dictid);
		}
		cached_dictid_ = curr_dictid;

		//detect from the cached record if it came from dict.voc
		//so we can choose the libdict_voc if so
		indictvoc = cached_dictrec_.f(16);

	}

	var dicttype = cached_dictrec_.f(1);
	bool ismv = cached_dictrec_.f(4).starts("M");

	// F type dictionaries
	if (dicttype == "F") {

		// check field number is numeric
		var fieldno = cached_dictrec_.f(2);
		if (!fieldno.isnum())
			return "";

		// field no > 0
		if (fieldno) {
			if (ismv)
				return RECORD.f(fieldno, MV);
			else
				return RECORD.f(fieldno);

			// field no 0
		} else {
			var keypart = cached_dictrec_.f(5);
			if (keypart && keypart.isnum())
				return ID.field("*", keypart);
			else
				return ID;
		}
	} else if (dicttype == "S") {
		if (newlibfunc) {

			std::string libname;
			if (indictvoc)
				libname = "dict_voc";
			else
				libname = DICT.f(1).lcase().convert(".", "_").toString();

			// Try to get dict_callable from cache
			std::string callable_cachekey = dictid.lcase().toString() + "_" + libname;
			dict_callable_ = cached_dict_functions[callable_cachekey];

			// If not in cache
			if (!dict_callable_) {
				// var(cachekey).logputl("cachekey=");

				// Create a new dict_callable
				dict_callable_ = new Callable;

//				// Patch in the current exoenv
//				if (!dict_callable_->mv_)
//					dict_callable_->mv_ = (&mv);

				// Wire it up with the necessary ExoEnv mv
				// and locate the correct function object in the library
				std::string str_funcname =
					("exodusprogrambasecreatedelete_" ^ dictid.lcase()).toString();
				if (!dict_callable_->initsmf(
														mv,
														libname.c_str(),
														str_funcname.c_str())
													)
					[[unlikely]]
					throw VarError("ExodusProgramBase::calculate() Cannot find Library " +
								  libname + ", or function " +
								  dictid.lcase() + " is not present");

				// Cache it
				cached_dict_functions[callable_cachekey] = dict_callable_;

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

		// return dict_callable_.calldict();
		// return ANS;

		ANS = dict_callable_->callsmf();

		// restore the MV if necessary
		//if (!ismv)
		//	MV = savedMV;

		return ANS;
	}

	[[unlikely]]
	throw VarError("ExodusProgramBase::calculate(" ^ dictid ^ ") " ^ DICT ^ " Invalid dictionary type " ^
				  dicttype.quote());
	//std::unreachable();
	//return "";
}

//// debug
//void ExodusProgramBase::debug() const {
//
//	var reply;
//	std::clog << "debug():";
//	if (OSSLASH == "/")
//		asm("int $3");	//use gdb n commands to resume
//	else
//		var().debug();
//	return;
//}

// fsmg
//bool ExodusProgramBase::fsmsg(CVR msg) const {
//	mssg(msg ^ var().lasterror());
//	return false;
//}

// sysvar
var ExodusProgramBase::sysvar(CVR /*errmsg*/, CVR /*var2*/, CVR /*var3*/, CVR /*var4*/) {

	std::cout << "sysvar() do nothing:";
	//	var reply;
	//	cin>>reply;
	return "";
}

// setprivilege
void ExodusProgramBase::setprivilege(CVR errmsg) {

	PRIVILEGE = errmsg;
	std::cout << "setprivilege(" << errmsg << ") do nothing" << std::endl;
	//	var reply;
	//	cin>>reply;
	return;
}

// decide 2
var ExodusProgramBase::decide(CVR question, CVR options) const {
	var reply = "";
	return decide(question, options, reply, 0);
}

// decide 4
var ExodusProgramBase::decide(CVR questionx, CVR optionsx, VARREF reply, const int defaultreply) const {

	// If default reply is 0 then there is no default
	// and pressing Enter returns "" and reply is set to 0

	// question can be multiline
	var question = questionx;
	question.converter(_ALL_FMS "|", "\n\n\n\n\n\n\n");

	var interactive = !SYSTEM.f(33);
	if (interactive)
		std::cout << var("========================================") << std::endl;

	std::cout << question << std::endl;

	//	var noptions = options.fcount(FM);
	var options = optionsx;
	if (not options)
		options = "Yes" _VM "No";
	options.converter(VM ^ "|", FM ^ FM);
	var noptions = options.fcount(FM);
	for (int optionn = 1; optionn <= noptions; optionn++) {
		if (optionn == defaultreply)
			std::cout << "*";
		else
			std::cout << " ";
		std::cout << optionn << ". " << options.f(optionn) << std::endl;
	}

inp:
	if (defaultreply)
		std::cout << " Please enter 1 - " << noptions << " or Enter to default or 0 to cancel.";
	else
		std::cout << " Please enter 1 - " << noptions;
	std::cout << std::endl;

	reply = defaultreply;

	if (interactive) {
		reply.input("? ");

		//entering ESC anywhere in the input causes "no response"
		if (reply.contains("\x1B"))
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
		[[unlikely]]
		throw VarError(questionx ^ " Default reply must be 0-" ^ noptions);
	}

	return options.f(reply);
}

// savescreen
void ExodusProgramBase::savescreen(VARREF /*origscrn*/, VARREF /*origattr*/) const {
	std::cout << "ExodusProgramBase::savescreen not implemented" << std::endl;
}

// keypressed
var ExodusProgramBase::keypressed(int milliseconds) const {
	return var().hasinput(milliseconds);
}

// esctoexzit
bool ExodusProgramBase::esctoexit() const {
	if (not keypressed())
		return false;

	//std::cout << "Press Esc again to cancel or any other key to continue " << std::endl;
	std::cout << " Paused. Continue? (Y/n) ";
	std::cout << std::flush;

	//wait for ever
	var key;
	key.input();

	//key.logputl("key=");
	//key.oconv("HEX").logputl("key=");

	return key.at(-1).ucase() == "N";
}

// otherusers
var ExodusProgramBase::otherusers(CVR /*param*/) {
	std::cout << "ExodusProgramBase::otherusers not implemented yet";
	return var("");
}

// otherdatasetusers
var ExodusProgramBase::otherdatasetusers(CVR /*param*/) {
	std::cout << "ExodusProgramBase::otherdatausers not implemented yet";
	return var("");
}

// lockrecord 3
bool ExodusProgramBase::lockrecord(CVR filename, VARREF file, CVR keyx) const {
	var record;
	return lockrecord(filename, file, keyx, record);
}

// lockrecord 6
bool ExodusProgramBase::lockrecord(CVR filename, VARREF file, CVR keyx, CVR /*recordx*/, const int waitsecs0, const bool allowduplicate) const {

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
			this->abort();
		}
	}

lock:

	// Fail if global terminate or reload requested
	if (TERMINATE_req or RELOAD_req) {
		var("").errputl("Lock request for " ^ file.f(1).quote() ^ ", " ^ keyx.quote() ^ " cancelled because 'terminate' or 'reload' requested.");
		return false;
	}

	var locked = file.lock(keyx);
	if (locked || (allowduplicate && locked == "")) {

		// fail if unexpired persistent lock exists in LOCKS file
		// on the same connection as file

		var leaselocks;
		if (leaselocks.open("LOCKS", file)) {

			var filename_for_locks = (filename.assigned() && filename) ? filename : file.f(1);
			var lockfilekey = filename_for_locks ^ "*" ^ keyx;

			var lockrec;
			if (lockrec.read(leaselocks, lockfilekey)) {

				//current dos time
				//convert to Windows based date/time (ndays since 1/1/1900)
				//31/12/67 in rev date() format equals 24837 in windows date format
				var dostime = var().ostime();
				dostime = 24837 + var().date() + dostime / 24 / 3600;

				//remove lock if expired
				if (lockrec.f(1) <= dostime) {
					leaselocks.deleterecord(lockfilekey);
					lockrec = "";
				}

				//or release any absolute lock
				//and return indication of lock failure
				else {
					if (locked)
						file.unlock(keyx);

					// failure due to leaselock fails immediately regardless of waitsecs
					return 0;
				}
			}
		}

		return 1;
	} else {
		if (waitsecs) {

			if ((waitsecs0 - waitsecs) >  30)
	            var("").errputl("Locking file : " ^ file.f(1).quote() ^ " key : " ^ keyx.quote() ^ " secs : " ^ waitsecs ^ "/" ^ waitsecs0);

			var().ossleep(1000);
			waitsecs -= 1;
			goto lock;
		}
		return false;
	}

	//std::unreachable()
	//return true;
}

// unlockrecord 0 = unlock all
bool ExodusProgramBase::unlockrecord() const {
	var xx;
	return unlockrecord("", xx, "");
}

// unlockrecord
bool ExodusProgramBase::unlockrecord(CVR /*filename*/, VARREF file0, CVR key) const {
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
}

// flushindex
void ExodusProgramBase::flushindex(CVR /*filename*/) {
	//std::cout << "ExodusProgramBase::std::flushindex not implemented yet, " << filename
	//	  << std::endl;
	return;
}

// encrypt2
var ExodusProgramBase::encrypt2(CVR encrypt0) const {

	var encrypt = encrypt0;
	var encryptkey = 1234567;

	// pass1
	while (true) {
		// BREAK;
		if (!(encrypt != ""))
			break;

		encryptkey = (encryptkey % 390001) * (var(encrypt[1])).seq() + 1;
		encrypt.cutter(1);
	}  // loop;

	// pass2
	while (true) {
		encrypt ^= var().chr(65 + (encryptkey % 50));
		encryptkey = (encryptkey / 50).floor();
		// BREAK;
		if (!encryptkey)
			break;

	}  // loop;

	return encrypt;
}

// xmlquote
var ExodusProgramBase::xmlquote(CVR string0) const {

	var string1;

	if (string0.unassigned()) {
		// de bug
		string1 = "UNASSIGNED";
	} else {
		string1 = string0;
	}

	string1.replacer("&", "&amp;");
	string1.replacer(DQ, "&quot;");
	string1.replacer("<", "&lt;");
	string1.replacer(">", "&gt;");

	string1.converter(DQ, "\'");
	string1.replacer(VM, "\" \"");
	string1.replacer(FM, "\" \"");
	return string1.quote();
}

// loginnet
bool ExodusProgramBase::loginnet(CVR /*dataset*/, CVR username, VARREF cookie, VARREF msg) {

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
		if (!(SECURITY.f(1).locate(username, usern))) {
			msg = "Error: " ^ username.quote() ^ " user is missing";
			return false;
		}
		menuid = SECURITY.f(3, usern);
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

	var menucodes = menu.f(6) ^ VM ^ "HELP";
	// remove local support menu
	if (!authorised("SUPPORT MENU ACCESS", msg, "LS")) {
		if (menucodes.f(1).locate("GENERA", menun))
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
	 compcode = temp.f(ii);
	 //until validcode('company',compcode)
	 //BREAK;
	 if (validcode2(compcode, "", "")) break;
	 };//ii;
	 }else{
	 compcode = temp.f(1);
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
	 var defmarketcode = (company.f(30)) ? (company.f(30)) : (agp.f(37));
	 //if unassigned(markets) then markets=''

	 //markets is not open in finance only module
	 //readf maincurrcode from markets,defmarketcode,5 else maincurrcode=''
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
	 temp = SYSTEM.f(23);
	 temp.replace("&", " and ");
	 cookie ^= "&db=" ^ temp;

	 backupreminder(dataset, msg);

	 changelogsubs("WHATSNEW" ^ FM ^ menucodes);
	 cookie ^= "&wn=" ^ ANS;

	 */
}

// WARNING/ pickos column and row numbering is 0 based but
// in exodus we move to 1 based numbering to be consistent with
// c/c++/linux/terminal standards. hopefully not too inconvenient

var ExodusProgramBase::AT(const int columnno, const int rowno) const {
	// THISIS("var ExodusProgramBase::at(const int columnno,const int rowno) const")

	std::string tempstr = "\x1B[";
	tempstr += std::to_string(rowno);
	tempstr.push_back(';');
	tempstr += std::to_string(columnno);
	tempstr.push_back('H');
	return tempstr;
}

var ExodusProgramBase::AT(const int columnno) const {
	// THISIS("var ExodusProgramBase::at(const int columnno) const")

	// hard coded for xterm at the moment
	// http://www.xfree86.org/current/ctlseqs.html

	// move to columnno 0
	if (columnno == 0)
		// return "\x1b[G";
		return "\r";  // works on more terminals

	//return "";

	// move to columnno
	if (columnno > 0) {
		std::string tempstr = "\x1B[";
		tempstr += std::to_string(columnno);
		tempstr.push_back('G');
		return tempstr;
	}
	// clear the screen and home the cursor
	if (columnno == -1)
		return "\x1B[2J\x1B[H";
	// return "\x0c";//works on more terminals

	// move the cursor to top left home
	if (columnno == -2)
		return "\x1B[H";

	// clear from cursor to end of screen
	if (columnno == -3)
		return "\x1B[J";

	// clear from cursor to end of line
	if (columnno == -4)
		return "\x1B[0K";

	// move cursor to columnno 0 and clear to end of line
	if (columnno == -40)
		return "\r\x1B[K";

	return "";
}

// handlefilename
var ExodusProgramBase::handlefilename(CVR handle) {
	return handle.f(1);
}

//// memspace
//var ExodusProgramBase::memspace([[maybe_unused]] CVR requiredmemory) {
//	return 999999999;
//}

//getuserdept
var ExodusProgramBase::getuserdept(CVR usercode) {
	// locate the user in the list of users
	var usern;
	if (!(SECURITY.f(1).locate(usercode, usern))) {
		if (usercode == "EXODUS") {
			ANS = "EXODUS";
			return ANS;
		} else {
			ANS = "";
			return ANS;
		}
	}

	// locate divider, or usern+1
	var nusers = (SECURITY.f(1)).count(VM) + 1;
	var usernx;
	for (usernx = 1; usernx <= nusers; usernx++) {
		// BREAK;
		if (SECURITY.f(1, usernx) == "---")
			break;

	}	// usern;

	// get the department code
	ANS = SECURITY.f(1, usernx - 1);

	return ANS;
}

// oconv
var ExodusProgramBase::oconv(CVR input0, CVR conversion) {

	// call user conversion routine
	// almost identical code in var::oconv and var::iconv
	// ENSURE synchronised if you change it

	var result = input0;

	//nothing in, nothing out
	//unfortutely conversions like L# do convert ""
	//if (result.len() == 0)
	//	return result;

	var ptr = 1;
	var delimiter;
	do {

		// var subconversion=conversion.remove(ptr,delimiter);
		var subconversion = conversion.substr2(ptr, delimiter);

		// EITHER 1. Call standard conversion methods
		//if (subconversion[1] != "[") {
		if (!subconversion.starts("[")) {
			result = result.oconv(subconversion);
		}

		else
		// OR 2. Call custom conversion routines
		// unlike pickos, custom routines are reponsible
		// for processing and returning multiple values
		{

			// var("*").logput();
			// return 1;

			//for speed dont call custom conversions on empty strings
			//nothing in, nothing out
			//Unfortunately [taghtml,td] produces output from nothing
			//if (result.len() == 0)
			//	continue;

			//remove brackets
			subconversion.substrer(2);
			if (subconversion.ends("]"))
				subconversion.popper();

			//determine the function name
			var functionname = subconversion.field(",").lcase();

			// extract any params
			var mode = subconversion.b(functionname.len() + 2);

			var outx;

			//custom function "[NUMBER]" actually has a built in function
			if (functionname == "number") {
				gosub exoprog_number("OCONV", result, mode, outx);
			}
			//custom function "[DATE]" will use the standard exodus var oconv
			else if (functionname == "date") {
				gosub exoprog_date("OCONV", result, mode, outx);
			}
			//custom function "[CAPITALISE]" will use the standard exodus var oconv
			else if (functionname == "capitalise") {
				outx = capitalise(result, mode, "");
			} else {

				// set the function name
				ioconv_custom = functionname;

				// wire in the current environment
				//ioconv_custom.mv_ = &mv;

				// and call it
				//call ioconv_custom("OCONV", result, mode, output);

				//call it once per field (any field mark RM-ST are preserved)
				var posn = 1;
				var ifield, ofield, delim;
				outx = "";
				while (true) {

					ifield = result.substr2(posn, delim);

					call ioconv_custom("OCONV", ifield, mode, ofield);

					outx ^= ofield;

					if (not delim)
						break;

					outx ^= var().chr(RM.seq() + 1 - delim);
				}
			}
			result = outx;
		}
	} while (delimiter);

	return result;
}

// iconv
var ExodusProgramBase::iconv(CVR input, CVR conversion) {

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
			var functionname = subconversion.cut(1).field(",", 1).field("]", 1).lcase();

			// extract any params
			var mode = subconversion.field(",", 2, 9999).field("]", 1);

			var outx;

			//custom function "[NUMBER]" actually has a built in function
            if (functionname == "number") {
                gosub exoprog_number("ICONV", result, mode, outx);
			}
			//custom function "[DATE]" actually has a built in function
            else if (functionname == "date") {
                gosub exoprog_date("ICONV", result, mode, outx);
			}
			//custom function "[CAPITALISE]" actually has a built in function
            else if (functionname == "capitalise") {
                //gosub capitalise(result, mode, outx);
				//result = result;
				outx = result;
            } else {

				// set the function name
				ioconv_custom = functionname;

				// wire up the current environment
				//ioconv_custom.mv_ = (&mv);

				// and call it
				call ioconv_custom("ICONV", result, mode, outx);
			}

			result = outx;
		}
	} while (delimiter);

	return result;
}


void ExodusProgramBase::getdatetime(out localdate, out localtime, out sysdate, out systime, out utcdate, out utctime) {

	//see CHANGETZ which seems to go through all times (dates?) and changes them
	//this should be done ONCE to standardise on gmt/utc really
	//really need a DBTZ to determine datetime for storage in the database


	var tt;//num

	//ensure time and date are taken in the same day
	//by ensuring time is not less than time1
	//which could happen over midnight
	while (true) {
		var systime1 = var().time();
		sysdate = var().date();
		systime = var().time();
		///BREAK;
		if (systime ge systime1) break;
	}//loop;

	//no timezone info
	if (not SW) {
		localdate = sysdate;
		localtime = systime;
		//assume system is on gmt/utc or database contains non-gmt datetimes
		utcdate = sysdate;
		utctime = systime;
		goto exit;
	}

	//@sw<1> is the ADJUSTMENT to get user time from server time, therefore add
	localtime = systime;
	localdate = sysdate;
	tt = SW.f(1);
	if (tt) {
		localtime += tt;
		if (localtime ge 86400) {
			//assume offset cannot be more than 24 hours!
			localdate = sysdate + 1;
			localtime -= 86400;
		} else if (localtime lt 0) {
			//assume offset cannot be less than 24 hours!
			localdate = sysdate - 1;
			localtime += 86400;
		}
	}

	//@sw<2> is the difference from gmt/utc to server time, therefore subtract
	utctime = systime;
	utcdate = sysdate;

	//following is irrelevent until we support user tz when server tz ISNT gmt/utc
	//
	//remove server tz to get gmt/utc. if server is ahead of gmt then serv tz is +
	tt = SW.f(2);
	if (tt) {
		utctime -= tt;
		if (utctime ge 86400) {
			//assume tz cannot be more than 24 hours!
			utcdate = sysdate + 1;
			utctime -= 86400;
		} else if (utctime lt 0) {
			//assume tz cannot be less than 24 hours!
			utcdate = sysdate - 1;
			utctime += 86400;
		}
	}

/////
exit:
/////
	if (SENTENCE eq "GETDATETIME") {
		var msg = "User:   " ^ localdate.oconv("D");
		msg ^= " " ^ localtime.oconv("MTH");
		msg ^= FM ^ "Server: " ^ sysdate.oconv("D");
		msg ^= " " ^ systime.oconv("MTH");
		msg ^= FM ^ "GMT/UTC:" ^ utcdate.oconv("D");
		msg ^= " " ^ utctime.oconv("MTH");
		call note(msg);
	}

	return;
}

var ExodusProgramBase::timedate2(in localdate0, in localtime0, in glang) {

	//caserevised*

	var localdate;
	var localtime;
	var x3;
	var x4;
	var x5;
	var x6;

	//use parameters only if both are provided
	if (localtime0.unassigned()) {
		call getdatetime(localdate, localtime, x3, x4, x5, x6);
	} else {
		localdate = localdate0;
		localtime = localtime0;
	}

	var temp = "";
	if (not temp) {
		temp = "MTH";
	}
	temp = oconv(localtime, temp);
	if (temp.starts("0")) {
		temp.cutter(1);
	}
	if (glang) {
		if (temp.ends("AM")) {
			temp.paster(-2, 2, glang.f(16));
		}
		if (temp.ends("PM")) {
			temp.paster(-2, 2, glang.f(17));
		}
	}

	if (DATEFMT eq "") {
		DATEFMT = "D/E";
	}
	temp.prefixer(oconv(localdate, "[DATE,*4]") ^ " ");

	return temp;
}


// elapsedtimetext 1 - from program start/TIMESTAMP
var ExodusProgramBase::elapsedtimetext() const {
	return elapsedtimetext(TIMESTAMP, var().timestamp());
}

// elapsedtimetext 2 - from duration
var ExodusProgramBase::elapsedtimetext(CVR timestamp_difference) const {
	return elapsedtimetext(0, timestamp_difference);
}

// elapsedtimetext 3 - given two timestamps
var ExodusProgramBase::elapsedtimetext(CVR timestamp1, CVR timestamp2) const {

	var text = "";

	var secs = (timestamp2 - timestamp1) * 86'400;

	var weeks = (secs / 604800).floor();
	secs -= weeks * 604800;

	var days = (secs / 86400).floor();
	secs -= days * 86400;

	var hours = (secs / 3600).floor();
	secs -= hours * 3600;

	var minutes = (secs / 60).floor();
	secs -= minutes * 60;

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
		if (secs) {
			if (minutes or (secs - 10 > 0)) {
				secs = secs.round(0);
			} else {
				//secs = (secs.oconv("MD40P")) + 0;
				secs = secs.round(6) + 0;
				if (secs.starts(".")) {
					secs.prefixer("0");
				}
			}
			if (secs) {
				text.r(-1, secs ^ " sec");
				if (secs ne 1) {
					text ^= "s";
				}
			} else if (not minutes and not hours and not days and not weeks) {
zero:
				if (not text)
					//text.r(-1, "< 1 ms");
					text = "< 1 ms";
			} else {
				//text.r(-1, "exactly");
			}
		} else {
			if (not minutes and not hours and not days and not weeks) {
				goto zero;
			}
			//text.r(3, "exactly");
		}
	}

	text.replacer(FM ^ FM ^ FM, FM);
	text.replacer(FM ^ FM, FM);
	text.replacer(FM, ", ");

	return text;
}

// number
	//function main(in type, in input0, in ndecs0, out outx) {
	//c xxx in,in,in,out

var ExodusProgramBase::exoprog_date(CVR type, CVR in0, CVR mode0, VARREF outx) {
//var ExodusProgramBase::number(CVR type, CVR input0, CVR ndecs0, VARREF outx) {
	//c sys in,in,in,out,in

	//should really be sensitive to timezone in @SW

	var inx = in0;
	var mode = mode0;

	if (inx eq "") {
		outx = "";
		return 0;
	}

	var nospaces = mode.contains("*");
	if (nospaces) {
		mode.converter("*", "");
	}

	if (mode) {

		if (mode eq "4") {
			mode = DATEFMT;
			mode.paster(2, 1, "4");
		}

	} else {

		if (DATEFMT) {
			mode = DATEFMT;
		} else {
			mode = "D2/E";
		}

	}

	//status=0
	if (type eq "OCONV") {

		//dont oconv 1 or 2 digits as they are probably day of month being converted
		// to proper dates
		//IF len(inx) gt 2 and inx MATCHES '0N' OR inx MATCHES '"-"0N' OR inx MATCHES '0N.0N' THEN
		if (inx.len() gt 2 and inx.match("^\\d*$")) {
			goto ok;
		}

		if (inx.match("^-\\d*$") or inx.match("^\\d*\\.\\d*$")) {
ok:
			//language specific (date format could be a pattern in lang?)
			if (mode eq "L") {

//				var tt = inx.oconv("D4/E");
//				var mth = glang.f(2).field("|", tt.field("/", 2));
//				var day = tt.field("/", 1);
//				var year = tt.field("/", 3);
//				//if 1 then
//				outx = day ^ " " ^ mth ^ " " ^ year;
//				//end else
//				// outx=mth:' ':day:' ':year
//				// end
				outx = inx.oconv("D");

			} else {

				outx = oconv(inx, mode);

				if (outx.starts("0")) {
					outx.paster(1, 1, " ");
				}

				if (outx[4] eq "0") {
					outx.paster(4, 1, " ");
				}

				if (outx.b(4, 3).match("^[A-Za-z]{3}$")) {
					outx.paster(7, 1, "");
					outx.paster(3, 1, "");
				}

				if (nospaces) {
					outx.converter(" ", "");
				}

			}

		} else {
			outx = inx;
		}

	} else if (type eq "ICONV") {

		if (inx.match("^\\d*$") and inx le 31) {
			inx ^= var().date().oconv("D").b(4, 9);
		}

		outx = iconv(inx, mode);

	}

	return 0;
}

// number
var ExodusProgramBase::exoprog_number(CVR type, CVR input0, CVR ndecs0, VARREF outx) {
	//function main(in type, in input0, in ndecs0, out outx) {
	//c xxx in,in,in,out

	var fmtx;
	var input1;	 //num
	var delim;	 //num
	var output1;

	var ndecs = ndecs0;
	var input = input0;
	outx = "";

	var zz = "";
	if (ndecs.contains("Z")) {
		ndecs.converter("Z", "");
		zz = "Z";
	}

	if (type == "ICONV") {
		var reciprocal = 0;
		if (input.starts("/")) {
			reciprocal = 1;
			input.cutter(1);
		} else {
			if (input.starts("1/")) {
				reciprocal = 1;
				input.cutter(2);
			}
		}

		outx = input.trim();

		//first get into a pickos number with dots not commas
		if (BASEFMT.starts("MC")) {
			outx.converter(",", ".");
		} else {
			outx.converter(",", "");
		}
		//nb [NUMBER,X] means no decimal place conversion to be done
		//if ndecs is given then convert to that number of decimals
		// if ndecs starts with a digit then use {NDECS} (use 2 if {NDECS}=null)
		if (ndecs == "") {
			if (DICT) {
				ndecs = calculate("NDECS");
			} else {
				ndecs = BASEFMT[3];
			}
			if (ndecs == "") {
				ndecs = 2;
			}
			if (not(ndecs.match("^\\d$"))) {
			}
			//FMTX='MD':NDECS:'0P'
			//outx=outx FMTX
		}
		if (ndecs == "*" or ndecs == "X") {
			ndecs = outx.field(".", 2).len();
		}
		if (ndecs == "BASE") {
			//fmtx = "MD" ^ BASEFMT[3] ^ "0P";
			fmtx = "MD" ^ BASEFMT.at(3) ^ "0P";
		} else {
			fmtx = "MD" ^ ndecs ^ "0P";
		}
		outx = oconv(outx, fmtx);
		STATUS = "";
		if (outx.isnum()) {
			if (reciprocal and outx) {
				outx = ((1 / outx).oconv("MD90P")) + 0;
			}
		} else {
			STATUS = 2;
		}

		return 0;
	}

	//oconv
	//////

	var divx = ndecs.field(",", 2);
	if (divx.len()) {
		ndecs = ndecs.field(",", 1);
	}

	var posn = 1;
	while (true) {
		input1 = input.substr2(posn, delim);

		var perc = input1[-1];
		if (perc == "%") {
			input1.popper();
		} else {
			perc = "";
		}
		var plus = input1[1];
		if (plus == "+") {
			input1.cutter(1);
		} else {
			plus = "";
		}

		if (input.len()) {

			if (divx) {
				input1 = input1 / var(10).pwr(divx);
			}

			if (input1.contains("E-")) {
				if (input1.isnum()) {
					input1 = input1.oconv("MD90P");
				}
			}

			var temp = input1;
			temp.converter("0123456789-.", "            ");
			var numlen = input1.len() - temp.trimfirst().len();
			var unitx = input1.b(numlen + 1, 99);
			var numx = input1.first(numlen);

			if (ndecs == "BASE") {
				output1 = oconv(numx, BASEFMT ^ zz) ^ unitx;
			} else {
				//if ndecs='' then ndecs=len(field(numx,'.',2))
				//!ndecs could be X to mean no conversion at all!
				//FMTX=@USER2[1,2]:ndecs:'0P,':z
				if (ndecs == "") {
					fmtx = BASEFMT.first(2) ^ numx.field(".", 2).len() ^ "0P," ^ zz;
				} else {
					fmtx = BASEFMT.first(2) ^ ndecs ^ "0P," ^ zz;
				}
				if (numx.isnum()) {
					numx += 0;
				}
				output1 = oconv(numx, fmtx) ^ unitx;

			}

			if (output1.len()) {
				if (var(".,").count(output1[1])) {
					output1.prefixer("0");
				}
			}

			outx ^= plus ^ output1 ^ perc;
		}

		///BREAK;
		if (not delim)
			break;
		//outx:=char(256-delim)
		outx ^= var().chr(RM.seq() + 1 - delim);
	}  //loop;

	return 0;
}

// sortarray
void ExodusProgramBase::sortarray(VARREF array, CVR fns, CVR orderby0) {
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

	var sortfn = fns.f(1, 1);
	var unsorted = array.f(sortfn);
	var sorted = "";

	//insert into a new array without other fields for speed
	var newarray = "";
	var nv = unsorted.fcount(VM);
	for (var vn = 1; vn <= nv; ++vn) {
		var value = unsorted.f(1, vn);
		if (not(sorted.locateby(orderby, value, newvn))) {
			{
			}
		}
		sorted.inserter(1, newvn, value);

		//insert any parallel fields
		for (var fnn = 2; fnn <= nfns; ++fnn) {
			fn = fns.f(1, fnn);
			var othervalue = array.f(fn, vn);
			newarray.inserter(fn, newvn, othervalue);
		}	//fnn;

	}	//vn;

	array.r(sortfn, sorted);

	//put any parallel fields back into the original array
	for (var fnn = 2; fnn <= nfns; ++fnn) {
		fn = fns.f(1, fnn);
		array.r(fn, newarray.f(fn));
	}  //fnn;

	return;
}

// invertarray
var ExodusProgramBase::invertarray(CVR input, CVR force0 /*=0*/) {
	//c sys in,=(0)

	var force = force0.unassigned() ? var(0) : force0;

	var outx = "";
	var nfs = input.fcount(FM);
	//for force to work, the first field must have full number of vns
	var maxnvs = 0;
	for (var fn = 1; fn <= nfs; ++fn) {
		var fieldx = input.field(FM, fn);
		if (fieldx.len() or force) {
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
				if (cell.len() or force) {
					outx.r(vn, fn, cell);
				}
			}	//vn;
		}
	}	//fn;

	return outx;
}

// amountunit 1
var ExodusProgramBase::amountunit(in input0) {
	var unitcode;
	return amountunit(input0, unitcode);
}

// amountunit 2
var ExodusProgramBase::amountunit(in input0, out unitx) {
	//c sys in,out

	/*
	unitx = input0;
	unitx.converter(" -.0123456789", "");

	var inputx = input0;
	//convert ' ABCDEFGHIJKLMNOPQRSTUVWXYZ' to '' in inputx
	//patsalides db PT0303 base currency code contains $,
	//must be removed otherwise B16 in journal.subs4
	inputx.converter(" ABCDEFGHIJKLMNOPQRSTUVWXYZ$", "");

	return inputx;

	*/

	// TODO Consider runtime Non-numeric or other error if amount or unit is missing
	// TODO Consider runtime error (Non-numeric?) if amount is not numeric

	// New method based on finding the last digit. Doesnt allow digits in unit codes.

	// Input            Amount        Unit
	// -----            ------        ----
	// 1.23456E-6USD -> 1.23456E-6    USD
	// USD              ""            USD
	// 1                1             ""

	// 123.45e7USD
	// 123.45e+07USD
	// 123.45e-07USD
	// 123.45M3 cubic meters TODO
	// 123.45e10M3 cubic meters TODO
	// 123e10 means e+10 and no unit

	// Find the last digit
	std::string s = input0.toString();
	std::size_t pos = s.find_last_of("0123456789");

	// If no digits are found, then return amount as "" and all input as unit
	// TODO Non-numeric error?
	if (pos == std::string::npos) {
		unitx = input0;
		return "";
	}

	// If last char is digit, then return all as amount and unit is "
	// TODO Non-numeric error?
	if (pos + 1 == s.size()) {
		unitx = "";
		return input0;
	}

	// Everything after the last digit is the unit
	// example "1ABC" pos = 0 and unit -> ABC
	unitx = s.substr(pos + 1);

	// Everything up to and including the last digit is the number
	// example 1.23456E-6 -> 1.23456E-6
	// TODO Non-numeric error?
	return s.substr(0, pos + 1);

}

// clang-format off

MVStop     ::MVStop(CVR errmsg)     : description(errmsg) {}
MVAbort    ::MVAbort(CVR errmsg)    : description(errmsg) {}
MVAbortAll ::MVAbortAll(CVR errmsg) : description(errmsg) {}
MVLogoff   ::MVLogoff(CVR errmsg)   : description(errmsg) {}

[[noreturn]] bool ExodusProgramBase::stop(CVR errmsg)     const {throw MVStop(errmsg);}
[[noreturn]] bool ExodusProgramBase::abort(CVR errmsg)    const {throw MVAbort(errmsg);}
[[noreturn]] bool ExodusProgramBase::abortall(CVR errmsg) const {throw MVAbortAll(errmsg);}
[[noreturn]] bool ExodusProgramBase::logoff(CVR errmsg)   const {throw MVLogoff(errmsg);}

// clang-format on

}  // namespace exodus
