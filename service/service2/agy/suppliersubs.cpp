#include <exodus/library.h>
libraryinit()

#include <xrefsubs.h>
#include <generalsubs2.h>
#include <authorised.h>
#include <suppliersubs.h>
#include <flushindex.h>
#include <singular.h>

#include <gen.h>
#include <agy.h>
#include <win.h>

#include <window.hpp>

var msg;
var locklist;
var text;
var wsmsg;

function main(in mode) {
	//c agy
	var interactive = not SYSTEM.a(33);
	//jbase
	if (mode == "VAL.TYPE") {
		if (win.is == win.isorig) {
			return 0;
		}
		if (win.is == "P" and RECORD.a(8)) {
			msg = "AS THIS SUPPLIER HAS SOME MEDIA VEHICLES|IT CANNOT BE CONVERTED TO A PRODUCTION ONLY SUPPLIER|IF NECESSARY CONVERT IT TO \"MP\" IE MEDIA AND PRODUCTION";
			return invalid(msg);
		}

	} else if (mode == "BUILDSUPPLIERGROUPMEMBERS") {

		win.datafile = "SUPPLIERS";
		win.srcfile = agy.suppliers;
		call xrefsubs("BUILD", 17, win.datafile, win.srcfile, 18, locklist);

	} else if (mode == "PREWRITE") {

		//update old and new group supplier
		//if either cannot be updated due to lock
		//then neither are updated and fails
		call xrefsubs("WRITE", 17, win.datafile, win.srcfile, 18, locklist);
		if (not win.valid) {
			return 0;
		}

		//update version log
		call generalsubs2(mode);

	} else if (mode == "PREDELETE") {
		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

		if (win.orec.a(8)) {
			var().chr(7).output();
			msg = "This supplier has vehicles and cannot be deleted|(eg ";
			msg ^= win.orec.a(8).field(VM, 1, 3);
			if (win.orec.a(8, 4)) {
				msg ^= " etc.";
			}
			msg ^= ")";
			msg.swapper(VM, ", ");
			call mssg(msg);
			win.valid = 0;
			return 0;
		}

		//remove from group supplier. fails if group supplier is locked
		call xrefsubs("DELETE", 17, win.datafile, win.srcfile, 18, locklist);
		if (not win.valid) {
			return 0;
		}

		//update version log
		call generalsubs2(mode);

	} else if (mode == "POSTINIT") {
		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

	} else if (mode == "POSTREAD2") {

		//get supplier currency
		if (RECORD.a(9)) {
			var accounts;
			if (accounts.open("ACCOUNTS", "")) {
				var account;
				if (account.read(accounts, RECORD.a(9, 1, 2))) {
					if (account.a(4)) {
						RECORD.r(11, account.a(4));
					}
				}
			}
		}

		//optionally prevent update of stopped suppliers
		if (win.wlocked) {
			//tt=ucase(@record<1>:@record<15>)
			//if index(tt,'(STOP)',1) or index(tt,'<STOP>',1) then
			if (RECORD.a(15) or (RECORD.a(1)).index("(STOP)", 1) or (RECORD.a(1)).index("<STOP>", 1)) {
				//if security('SUPPLIER ACCESS STOPPED',msg) else
				if (not(authorised("SUPPLIER UPDATE TERMS", msg))) {
					win.wlocked = 0;
					//explain to the user why cannot update
					win.reset = -1;
					call mssg("This supplier is stopped and you are not authorised to update it" ^ FM ^ FM ^ msg);
				}
			}
		}

	} else if (mode == "POSTREAD") {

		//option to read previous versions
		call generalsubs2(mode);
		if (not win.valid) {
			return 0;
		}

		gosub security(mode);
		if (not win.valid) {
			return 0;
		}

		call suppliersubs("POSTREAD2");

		//type no longer required?
		//determine media, production or either
		//if security('SCHEDULE ACCESS',msg,'') then type='M'
		//if security('JOB ACCESS',msg,'') then
		// if type then
		// type=''
		// end else
		// type='P'
		// end
		// end

	} else if (mode == "IMPORT") {
		if (not text.osread("y")) {
			var().stop();
		}
		text.trimmer();
		var newtext = "";
		var nn = text.count(FM) + 1;
		for (var ii = 1; ii <= nn; ++ii) {
			var code = text.a(ii).field(" ", 1);
			var seqx = 999;
			if (code.substr(1,3) == "AD-") {
				code.splicer(1, 3, "");
				code.splicer(3, 1, "Z");
				seqx = 998;
			}
			var name = text.a(ii).field(" ", 2, 999);
			var rec;
			if (rec.read(agy.suppliers, code)) {
				printl(code , "\x09" , name , "\x09" , "DUPLICATE");
				newtext.r(-1, code ^ " " ^ name);
			}else{
				if (code.length() > 3) {
					printl(code , "\x09" , name , "\x09" , "GT3");
					newtext.r(-1, code ^ " " ^ name);
				}else{
					rec = name;
					rec.r(5, seqx);
					rec.write(agy.suppliers, code);
					//print rec
				}
			}
		};//ii;

	} else if (mode == "POSTWRITE" or mode == "POSTDELETE") {
		call flushindex("SUPPLIERS");

	}
//L976:

	return 0;

}


libraryexit()