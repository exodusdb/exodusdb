/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/
#include "Definition.h"

namespace exodus
{

//TODO:#include <taxsubs.h>
//TODO:#include <vouchertypesubs.h>

var Definition::operator() (const var& mode)
{
	var msg;

/*TODO:
	if (env.ID == "TAXES") {
		taxsubs(mode);
		return;
	}

	if (env.ID == "ALL") {
		vouchertypesubs(mode);
		return;
		}
*/
	if (mode == "POSTREAD") {

		//nb also called from postwrite

		if (env.ID == "SECURITY") {
			//template now provided by readenvironment in client
			//either security or hourlyrates
			//template='security'
			win.securitysubs("SETUP");
			return true;
		}

		//default cheque format
		if (win.wlocked and env.ID.field("*", 1, 1) == "CHEQUEDESIGN") {

			//prevent update
			if (!(env.authorised("CHEQUE DESIGN", msg, "UA"))) {
				win.srcfile.unlock( env.ID);
				win.wlocked = "";
			}

			if (!(env.RECORD.extract(1))) {

				var temp;
				if (temp.read(win.srcfile, "CHEQUEDESIGN*DEFAULT")) {

					if (!(env.RECORD.read(win.srcfile, "CHEQUEDESIGN*" ^ temp.extract(1, 1))))
						goto nochequeformat;

				}else{

nochequeformat:
					if (!(temp.read(win.srcfile, "CHEQUEDESIGN*ZZZ999"))) {
						if (temp.open("ALANGUAGE")) {
							if (!(env.RECORD.read(temp, "VOUCHERS**CHEQUE")))
								env.RECORD = "";
						}
					}

				}

				//flag a new record despite being copied
				env.RECORD.replacer(10, 0, 0, "");
				env.RECORD.replacer(11, 0, 0, "");
				env.RECORD.replacer(14, 0, 0, "");

			}

			return 1;
		}

		if (env.ID.field("*", 2, 1) == "ANALDESIGN") {

			//check allowed access
 			if (!(env.authorised("BILLING REPORT ACCESS", msg, ""))) {
				win.reset = 5;
				var xx = env.unlockrecord("DEFINITIONS", win.srcfile, env.ID);

                //
				return 0;

			}

			//always allowed to update or delete own records
			if (env.RECORD.extract(8) not_eq env.USERNAME) {
				win.security2(mode, "BILLING REPORT");
			}

			//prevent update or delete of neosys definitions
			if (win.wlocked and (env.RECORD.extract(8)).index("NEOSYS", 1) and not env.USERNAME.index("NEOSYS", 1)) {
preventupdate:
				win.wlocked = 0;
				var xx = env.unlockrecord("DEFINITIONS", win.srcfile, env.ID);
			}

			return 0;
		}

		if (env.ID == "AGENCY.PARAMS") {

			//configuration is locked to neosys initially
			if (!(env.authorised("CONFIGURATION UPDATE", msg, "AUTHORISATION UPDATE")))
				goto preventupdate;

		}

		gosub_postreadfix();

	}else if (mode == "PREWRITE") {

		if (env.ID == "SECURITY") {
			//no difference between security update and hourlyrate update now
			win.templatex = "SECURITY";
			win.securitysubs("SAVE");
			return 1;
		}

		//save cheque design for neosys
		if (env.ID.field("*", 1, 1) == "CHEQUEDESIGN") {

			//prevent update
			if (!(env.authorised("CHEQUE DESIGN", msg, "UA")))

                //
                return 0;

			if (env.RECORD.extract(14)) {

				//remove old default
				var temp;
				if (temp.read(win.srcfile, "CHEQUEDESIGN*DEFAULT")) {
					var temp2;
					if (temp2.read(win.srcfile, "CHEQUEDESIGN*" ^ temp.extract(1, 1))) {
						temp2.replacer(14, 0, 0, "");
						temp2.write(win.srcfile, "CHEQUEDESIGN*" ^ temp.extract(1, 1));
					}
				}

				//set new default
				env.ID.field("*", 2, 1).write(win.srcfile, "CHEQUEDESIGN*DEFAULT");

				//if neosys then save the default as neosys programs default
				if (env.USERNAME == "NEOSYS") {
					if (temp.open("ALANGUAGE"))
						env.RECORD.write(temp, "VOUCHERS**CHEQUE");
				}

			}else{

				//remove as default (assume is this account)
				if (win.orec.extract(14)) {
					win.srcfile.deleterecord("CHEQUEDESIGN*DEFAULT");
				}

			}

		}

		//update neosys standard (in case doing this on the programming system)
		//the programming standard is installed into all clients
		//on first login after upgrade
		//this is also done in copygbp perhaps could be removed from there
		//almost identical code in definition.subs and get.subs (for documents)
		//field 10 in documents and definitions xxx*analdesign means the same
		if (env.ID.field("*", 2, 1) == "ANALDESIGN" and env.USERNAME == "NEOSYS" and env.RECORD.extract(10)) {
			var reports;
			if (reports.open("REPORTS")) {
				var key = env.ID;
				key.swapper("*", "%2A");
				key = "DEFINITIONS*" ^ key;
				env.RECORD.write(reports, key);
			}
		}

		if (env.ID == "AGENCY.PARAMS") {

			//prevent changing "invno by company" after data is entered
			if (env.RECORD.extract(48) not_eq win.orec.extract(48)) {
//TODO:				anydata("", anydata);
                var anydata="";
				if (anydata) {
					env.RECORD.replacer(48, 0, 0, win.orec.extract(48));
					msg = "You cannot change Invoice Number Sequence after data is entered";
					msg.replacer(-1, 0, 0, "That change has been ignored.");

                    //
                    return 0;
				}
			}

			//encrypt the mark
			//env.RECORD.replacer(1, 0, 0, (env.RECORD.extract(1)).invert());

		}

	}else if (mode == "POSTWRITE") {

		//have to pass back the inverted, partial record
		if (env.ID == "SECURITY") {
			win.templatex = "SECURITY";

			//to get lastest userprivs
			win.securitysubs("POSTAPP");

			//to the get the filtered record again
            Definition definition;
            definition(var("POSTREAD"));

		}

		gosub_postreadfix();

	}else if (mode == "PREDELETE") {

		if (env.ID == "SECURITY") {
			msg = "DELETE SECURITY not allowed in DEFINITION.SUBS";

            //
            return 0;
		}

		if (env.ID.field("*", 2, 1) == "ANALDESIGN") {

			//prevent update
			if (!(env.authorised("CHEQUE DESIGN", msg, "UA")))

                //
                return 0;

			//always allowed to delete own records
			if (env.RECORD.extract(8) not_eq env.USERNAME) {
				win.security2(mode, "BILLING REPORT");
			}

		}

		//remove default cheque design
		if (env.ID.field("*", 1, 1) == "CHEQUEDESIGN") {
			if (env.RECORD.extract(14)) {
				win.srcfile.deleterecord("CHEQUEDESIGN*DEFAULT");

            }
		}

		//update neosys standard (in case doing this on the programming system)
		//%deleted% ensures that deleted neosys documents get deleted
		//on upgrading clients
		if (env.ID.field("*", 2, 1) == "ANALDESIGN" and env.USERNAME == "NEOSYS" and env.RECORD.extract(10)) {
			var reports;
			if (reports.open("REPORTS")) {
				var key = env.ID;
				key.swapper("*", "%2A");
				key = "DEFINITIONS*" ^ key;
				var xx;
				if (xx.read(reports, key))
					var("%DELETED%").write(reports, key);
				//delete reports,key
			}
		}

	}else if (mode == "POSTDELETE") {

	}else if (1) {
		msg = mode.quote() ^ " invalid mode in definition.subs";
		win.valid=0;

        //
        return 0;
	}
	return 1;
}

void Definition::gosub_postreadfix()
{
	////////////
	if (env.ID == "AGENCY.PARAMS") {

		//env.RECORD.replacer(1, 0, 0, (env.RECORD.extract(1)).invert());

		if (env.RECORD.extract(55) == "")
			env.RECORD.replacer(55, 0, 0, "Budget");
		if (env.RECORD.extract(56) == "")
			env.RECORD.replacer(56, 0, 0, "F/cast");
		if (env.RECORD.extract(72) == "")
			env.RECORD.replacer(72, 0, 0, "Media Plan");
		if (env.RECORD.extract(73) == "")
			env.RECORD.replacer(73, 0, 0, "Media Schedule");
		if (env.RECORD.extract(74) == "")
			env.RECORD.replacer(74, 0, 0, "Estimate");

		if (env.RECORD.extract(49) == "")
			env.RECORD.replacer(49, 0, 0, "<NUMBER>");
		if (env.RECORD.extract(49, 1, 2) == "")
			env.RECORD.replacer(49, 1, 2, "2000.01");
		if (env.RECORD.extract(50) == "")
			env.RECORD.replacer(50, 0, 0, "<NUMBER>");
		if (env.RECORD.extract(50, 1, 2) == "")
			env.RECORD.replacer(50, 1, 2, "2000.01");

		//done in agency.subs getnextid
		if (env.RECORD.extract(53) == "")
			env.RECORD.replacer(53, 0, 0, "<NUMBER>");
		if (env.RECORD.extract(63) == "")
			env.RECORD.replacer(63, 0, 0, "<NUMBER>");
		if (env.RECORD.extract(69) == "")
			env.RECORD.replacer(69, 0, 0, "<NUMBER>");
		if (env.RECORD.extract(70) == "")
			env.RECORD.replacer(70, 0, 0, "<NUMBER>");
		if (env.RECORD.extract(71) == "")
			env.RECORD.replacer(71, 0, 0, "<NUMBER>");

		if (env.RECORD.extract(25) == "")
			env.RECORD.replacer(25, 0, 0, "ACC<YEAR>");
		if (env.RECORD.extract(26) == "")
			env.RECORD.replacer(26, 0, 0, "WIP<YEAR>");

		//copy schedule footer to plan footer
		if (!(env.RECORD.extract(34)))
			env.RECORD.replacer(34, 0, 0, env.RECORD.extract(11));

	}

	return;

} // /:~

}
