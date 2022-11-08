#include <exodus/library.h>
libraryinit()

#include <authorised.h>
#include <collectixvals.h>
#include <esctoattr.h>
#include <exodus/htmllib2.h>
#include <getbackpars.h>
#include <listen4.h>
#include <listen5.h>
#include <openfile.h>
#include <otherusers.h>
#include <rtp57.h>
#include <scrnio.h>
#include <select2.h>
#include <singular.h>
#include <sysmsg.h>

#include <service_common.h>

#include <srv_common.h>
#include <req_common.h>

var initdir;
var request2;
var request3;
var request4;
var request5;
var request6;
//var filenamesx;
//var filename;
var processes;
var tt;
var xx;
var inpath;
var pattern;
var ageinsecs;	// num
//var ii;			// num
var patchfile;
var offset;
var firstblock;
var patchid;
var keyandrec;
var rec;
var versiondatetime;
var versionkey;
var patchdatetime;
var skipreason;
var skipemail;	// num
var lastpatchid;
var runoncekey;
var runonce;
var bottomline;
var stopn;
var locks;
var lockid;
var dostime;  // num
var yy;
var zz;
var serverend;
var bakpars;
var msg2;
var fileattributes;
var filetime;
var offset_zero;

function main(in request1, in request2in, in request3in, in request4in, in request5in, in request6in) {

	// $insert abp,common
	// $insert bp,agency.common

	// TODO share various files with LISTEN to prevent slowing down by opening?

	let tracing = 1;
	initdir		= "";

	// no output in arguments allowed since c++ doesnt allow
	// defaulting or value based arguments and setting them
	// and this means calling listen2 would require passing variables for all
	// in the few cases listen2 need to respond to caller, it sets @ans
	if (request2in.unassigned()) {
		request2 = "";
	} else {
		request2 = request2in;
	}
	if (request3in.unassigned()) {
		request3 = "";
	} else {
		request3 = request3in;
	}
	if (request4in.unassigned()) {
		request4 = "";
	} else {
		request4 = request4in;
	}
	if (request5in.unassigned()) {
		request5 = "";
	} else {
		request5 = request5in;
	}
	if (request6in.unassigned()) {
		request6 = "";
	} else {
		request6 = request6in;
	}
	var logx = request2;

	var isdevsys = var("exodus.id").osfile();

	// Auto start databases on request
	//////////////////////////////////
	if (request1 eq "RUNS") {

		// db start commands from xhttp.php
		let filenamesx = oslistf("*.run");

		// for (const var filen : range(1, 9999)) {
		for (var filename : filenamesx) {
			// filename = filenamesx.f(filen);
			// /BREAK;
			if (not filename)
				break;
			if (lockrecord("PROCESSES", processes, "START*" ^ filename)) {
				if (tt.osread(filename)) {
					tt.replacer("\r\n", _FM);
					tt.converter("\r\n", _FM _FM);
					// dont start if there is a database stop command
					if (not((tt.f(1).lcase() ^ ".end").osfile())) {
						if (tt.f(5)) {
							// var cmd = "CMD /C START EXODUS.JS /system " ^ tt.f(5) ^ " /database " ^ tt.f(1) ^ " /pid " ^ tt.f(6);

							// TRACE: tt="tamra2^EXODUS^^^ADAGENCY^/root/hosts/tamra/data/tamra2/~4025798^"
							// TRACE: osgetenv("EXO_SERVICE_CODE")="agy_live@tamra"

							// var application = tt.f(5).first(3).lcase();
							var app_code = osgetenv("EXO_SERVICE_CODE").field("_", 1);
							if (not app_code)
								app_code = osgetenv("APP_CODE");

							var database = tt.f(1);
							var mode	 = "live";
							if (database.ends("_test")) {
								mode = "test";
								database.cutter(-5);
							}

							let cmd = "systemctl start " ^ app_code ^ "_" ^ mode ^ "@" ^ database;
							// printl(AT(-40), time().oconv("MTS"), " ", tt);
							if (TERMINAL)
								print(AT(-40));
							printl(time().oconv("MTS"), " ", cmd);

							//cmd.osshell();
							if (not cmd.osshell())
								abort(lasterror());
						}
						//filename.osremove();
						if (filename and not filename.osremove())
							abort(lasterror());
					}
				}
				xx = unlockrecord("", processes, "START*" ^ filename);
			}
		}  // filen;

	} else if (request1 eq "DELETEOLDFILES2") {

		inpath = request3;
		tt	   = "\\/";
		inpath.converter(tt, _OSSLASH _OSSLASH);
		// delete old response and temp files every 1 minute
		call listen5("DELETEOLDFILES", "*.4", inpath);
		call listen5("DELETEOLDFILES", "*.5", inpath);
		// call listen5('DELETEOLDFILES','*.$2','./',tt,yy,zz)
		// shell2 cannot/does not delete its tempfiles due to wget remaining in background
		call listen5("DELETEOLDFILES", "vdm*.tmp", "./", 60);
		call listen5("DELETEOLDFILES", "*.$$*", "./", 60);

	} else if (request1 eq "DELETEOLDFILES") {

		pattern = request2;
		inpath	= request3;

		// delete files older than x
		ageinsecs = request4;
		if (not ageinsecs) {
			ageinsecs = SYSTEM.f(28);
		}
		if (ageinsecs eq "") {
			ageinsecs = 60 * 60;
		}

		gosub deleteoldfiles();

	} else if (request1 eq "CHECKRESTART") {

		// 		// check for corruption in system record
		// 		// if index(system,char(0),1) then
		// 		if (SYSTEM.contains(chr(0))) {
		// 			var(SYSTEM).oswrite("system.bad");
		// 			call sysmsg("Corrupt SYSTEM record in LISTEN - RESTARTING");
		// 			ANS = "CORRUPTSYSTEM";
		// 			SYSTEM.converter(chr(0), "");
		// 			ANS = "RESTART " ^ ANS;
		// 			return 0;
		// 		}

		// detect system parameter changes and restart
		// this has the effect of detecting corruption in system which inserts lines
		let s100	= SYSTEM.f(100);
		let exohome = osgetenv("EXO_HOME");
		var ospaths = "../../system.cfg,system.cfg";
		ospaths ^= "," ^ exohome ^ "/dat/";
		ospaths.converter("/", OSSLASH);
		let npaths = fcount(ospaths, ",");
		for (const var ii : range(1, npaths)) {
			let ospath = ospaths.field(",", ii);
			// order is significant
			let newtime = (ospath.ends(OSSLASH)) ? ospath.osdir().f(3) : ospath.osfile().f(3);
			let oldtime = s100.f(1, ii);
			if (newtime ne oldtime) {
				if (oldtime) {
					ANS = "RESTART " ^ ospath;
					return 0;
				} else {
					SYSTEM(100, ii) = newtime;
				}
			}
		}  // ii;

		// 		// check for upgrade to LISTEN
		// 		let gbp;
		// 		if (gbp.open("GBP", "")) {
		// 			let listen;
		// 			if (listen.read(gbp, "$LISTEN")) {
		// 				listen = field2(listen, FM, -1);
		// 				if (s100.f(1, 3)) {
		// 					if (s100.f(1, 3) ne listen) {
		// 						// this will only restart listen, not the whole process/thread
		// 						ANS = "RESTART $LISTEN";
		// 						return 0;
		// 					}
		// 						// comment to solve c++ decomp problem
		// 				} else {
		// 					SYSTEM(100, 3) = listen;
		// 				}
		// 			}
		// 		}

		ANS = "";
		return 0;

	} else if (request1 eq "PATCHANDRUNONCE") {

		// never patch and run on development systems (therefore can only test elsewhere)
		// OFF while developing this feature
		isdevsys = isdevsys and date() gt 19034;
		// if system<61> or isdevsys then
		// if (isdevsys or not(VOLUMES)) {
		if (isdevsys) {
			ANS = "";
			return 0;
		}

		let live  = request2;
		processes = request3;

		// //////////////////////////////////////////////////////////////////////////
		// Look for NEOPATCH.1 file in three locations
		// //////////////////////////////////////////////////////////////////////////
		// The file must have been created after the exodus version date
		// and after the last patch date if any
		// //////////////////////////////////////////////////////////////////////////
		// 1. DATA\CLIENTX\NEOPATCH.1
		//               - install in one EXODUS database
		// 2. DATA\NEOPATCH.1
		//               - install in all active EXODUS databases in one installation
		// 3. D:\NEOPATCH.1 or D:\HOSTS\NEOPATCH.1
		//               - install in all active EXODUS database and installations
		let patchcode = "NEOPATCH";
		var patchdirs = "../DATA/" ^ SYSTEM.f(17) ^ FM ^ "../DATA/" ^ FM ^ "../../";
		patchdirs.converter("/", OSSLASH);

		for (const var patchn : range(1, 3)) {

			// skip if no patch file or not dated today
			let patchfilename = patchdirs.f(patchn) ^ patchcode ^ ".1";
			let patchfileinfo = patchfilename.osfile();
			if (patchfileinfo.f(2) lt date()) {
				goto nextpatch;
			}

			// open patch file
			if (not(patchfile.osopen(patchfilename))) {
				goto nextpatch;
			}

			// ensure patch file is complete
			offset = patchfileinfo.f(1) - 18;
			//call osbread(tt, patchfile, offset, 18);
			if (not osbread(tt, patchfile, offset, 18))
				abort(lasterror());
			if (tt ne("!" ^ FM ^ "!END!OF!INSTALL!")) {
				goto nextpatch;
			}

			// verify correct file heading and determine patchid from the file
			offset_zero = 0;
			//call osbread(firstblock, patchfile, offset_zero, 65000);
			if (not osbread(firstblock, patchfile, offset_zero, 65000))
				abort(lasterror());
			patchid = firstblock.f(2).cut(5);
			if (firstblock.f(1) ne "00000DEFINITIONS" or patchid.first(8) ne "INSTALL*") {
				goto nextpatch;
			}

			// skip if patchid is older than 30 days
			// if field(patchid,'*',3) lt date()-30 then
			// goto nextpatch
			// end

			// extract DEFINITIONS install key and rec from first block
			// rec is "00000DEFINITIONS^00033INSTALL*X*19034*35287^...."
			// remove 00000DEFINITIONS^
			firstblock.remover(1);
			// remove 5 byte length field and cut out key+rec
			keyandrec = firstblock.b(6, firstblock.first(5));
			// remove key
			rec = keyandrec.remove(1, 0, 0);

			// installation wide lock on it
			if (not(lockrecord("", processes, patchid))) {
				goto nextpatch;
			}

			// skip if already installed in this database
			if (xx.read(DEFINITIONS, patchid)) {
				call unlockrecord("", processes, patchid);
				goto nextpatch;
			}

			// prevent from ever running this patch again on this database
			rec.write(DEFINITIONS, patchid);

			// get current EXODUS version timestamp
			versiondatetime = "";
			versionkey		= "general/version.dat";
			versionkey.converter("/", OSSLASH);
			if (tt.osread(versionkey)) {
				tt.trimmer();
				let vdate = tt.field(" ", 2, 3).iconv("D");
				let vtime = tt.field(" ", 1).iconv("MT");
				if (vdate and vtime) {
					versiondatetime = vdate ^ "." ^ vtime.oconv("R(0)#5");
				}
			}

			// get patch timestamp
			patchdatetime = patchid.field("*", 3);

			// skip patch if older than installation
			skipreason = "";
			skipemail  = "";
			if (versiondatetime and patchdatetime lt versiondatetime) {
				skipreason = "Patch is older than installation version - " ^ oconv(versiondatetime, "[DATETIME,4*]");
				skipemail  = 1;
			}

			// skip patch if older than last patch
			if (not(lastpatchid.read(DEFINITIONS, "INSTALL*LAST"))) {
				lastpatchid = "";
			}
			if (lastpatchid) {
				let lastpatchdatetime = lastpatchid.field("*", 3);
				if (patchdatetime lt lastpatchdatetime) {
					skipreason = "Patch is older than the last patch - " ^ oconv(lastpatchdatetime, "[DATETIME,4*]");
				}
			}

			// ensure that we only ever runonce something just loaded from a patch
			runoncekey = "$" ^ patchid.field("*", 2) ^ ".RUNONCE";
			DEFINITIONS.deleterecord(runoncekey);

			if (not skipemail) {

				// list the files and records that were installed
				let subject = rec.f(1) ^ " - " ^ patchid.field("*", 2) ^ " " ^ oconv(patchid.field("*", 3), "[DATETIME,4*]");
				var body	= subject ^ " " ^ patchfilename ^ FM;
				if (skipreason) {
					body(-1) = FM ^ "NOT PATCHED - " ^ skipreason ^ FM ^ FM;
				}
				let nfiles = rec.f(3).fcount(_VM);
				for (const var filen : range(1, nfiles)) {
					body(-1) = rec.f(3, filen) ^ " " ^ rec.f(4, filen) ^ "  " ^ rec.f(5, filen);
				}  // filen;

				// message EXODUS only
				call sysmsg(body, "NEOPATCH: " ^ subject, "EXODUS");
			}

			if (not skipreason) {

				// perform the installation
				let cmd = "INSTALL " ^ patchcode ^ " " ^ oscwd().first(2) ^ " (IO)";
				printl(cmd);
				perform(cmd);
			}

			// record success/failure before any autorun
			(date() ^ "." ^ time().oconv("R(0)#5")).writef(DEFINITIONS, patchid, 6);

			skipreason.writef(DEFINITIONS, patchid, 7);

			if (skipreason) {
				// release
				call unlockrecord("", processes, patchid);
				goto nextpatch;
			}

			// save the last patch info - used to prevent backward patching
			patchid.write(DEFINITIONS, "INSTALL*LAST");

			// post install runonce if installed
			// if $PATCH.RUNONCE or $datasetcode.RUNONCE appears in definitions
			// if the runonce record appears in the definitions then
			// run it, save it and delete it
			if (runonce.read(DEFINITIONS, runoncekey)) {
				perform("RUN DEFINITIONS " ^ runoncekey.cut(1));
				// leave it for inspection
				// delete definitions,runoncekey
			}

			// trigger other processes to restart by updating SYSTEM.CFG
			if (tt.osread("system.cfg")) {
				//var(tt).oswrite("system.cfg");
				if (not var(tt).oswrite("system.cfg"))
					abort(lasterror());
			}

			// release
			call unlockrecord("", processes, patchid);

			// indicate patches applied and may need restart
			ANS = 1;

			// dont check any other patchdirs
			return 0;

nextpatch:;
		}  // patchn;

		ANS = "";

	} else if (request1 eq "CONVLOG") {

		// assumes at least 0-31 and 128-255 encoded like $hh

		// reserve/use special characters for field separators
		logx.replacer("^", "%5E");
		logx.replacer("]", "%5D");
		logx.replacer("\\", "%5C");
		logx.replacer("[", "%5B");
		logx.replacer("%FE", "^");
		logx.replacer("%FD", "]");
		logx.replacer("%FC", "\\");
		logx.replacer("%FB", "[");
		logx.replacer(RM, "%FF");
		// swap fm with '^' in logx
		// swap vm with ']' in logx
		// swap sm with '\' in logx;*backslash
		// swap tm with "[" in logx
		logx.converter(_FM _VM _SM _TM, "^]\\[");
		// logx.converter(_ALL_FMS, _VISIBLE_FMS);
		logx.replacer(ST, "%FA");

		logx.replacer("%20", " ");
		// does not seem to format in XML
		// swap '%20' with '&nbsp;' in logx

		logx.replacer("&", "&amp;");
		logx.replacer(DQ, "&quot;");
		logx.replacer("<", "&lt;");
		logx.replacer(">", "&gt;");
		logx.replacer("%26", "&amp;");
		logx.replacer("%22", "&quot;");
		logx.replacer("%3C", "&lt;");
		logx.replacer("%3E", "&gt;");

	} else if (request1.starts("GETINDEX")) {

		data_		  = "";
		var filename  = request2;
		let fieldname = request3;
		let prefix	  = request4;
		var sortby	  = request5;
		if (not sortby) {
			sortby = "AL";
		}
		if (sortby) {
			if (not(var("AL,AR,DL,DR").locateusing(",", sortby, xx))) {
				response_ = "Invalid sortby " ^ (sortby.quote()) ^ " in LISTEN,GETINDEXVALUES";
				return 0;
			}
		}
		var active = request6;

		if (not(VOLUMES)) {
			if (filename eq "JOURNALS") {
				filename = "JOURNALS";
			}
		}

		if (filename eq "JOURNALS" or filename eq "JOURNALS") {
		} else {
			gosub fileaccesscheck(filename);
			if (response_) {
				return 0;
			}
		}

		// security check
		var temp = filename;

		// zzz
		if (temp eq "JOURNALS") {
			temp = "JOURNAL";
		}

		if (not(authorised(singular(temp) ^ " LIST", msg_, ""))) {
			response_ = msg_;
			return 0;
		}

getvalues:
		call collectixvals(filename, fieldname, prefix);
		PSEUDO.move(data_);
		if (data_.starts(FM)) {
			data_.cutter(1);
		}
		response_ = "OK";

		if (sortby and data_) {
			// convert fm to rm in iodat
			// iodat:=rm
			// call v119('S','',sortby[1,1],sortby[2,1],iodat,flag)
			// convert rm to fm in iodat
			// iodat[-1,1]=''
			// v119 C/ASM sort routine cannot be easily converted/reimplemented in c++
			data_.converter(_FM, _VM);
			call sortarray(data_, 1, sortby);
			data_.converter(_VM, _FM);
		}

		var execstoplist;
		if (execstoplist.read(DEFINITIONS, "INDEXVALUES*" ^ filename ^ "*" ^ fieldname)) {

			// execs will be in field 1
			// stopped reason will be in parallel in field 2
			// stopped execs will be at the end
			data_.converter(_FM, _VM);

			// remove or move any stopped execs to the end and add reason
			var nn	= data_.fcount(_VM);
			let nn2 = nn;
			for (int ii = 1; ii <= nn; ++ii) {
				let execcode = data_.f(1, ii);
				if (execstoplist.f(1).locate(execcode, stopn)) {
					let reason = execstoplist.f(2, stopn);
					if (reason) {
						data_.remover(1, ii);
						data_.remover(2, ii);
						if (not active) {
							data_(1, nn2) = execcode;
							data_(2, nn2) = reason;
						}
						ii -= 1;
						nn -= 1;
					}
				}
			}  // ii;

			// show inactive if none are active
			if (data_ eq "" and active) {
				active = "";
				goto getvalues;
			}

			// 1=force vm to ensure xml has empty not missing tags
			data_(2, 1) = data_.f(2, 1);
			data_		= invertarray(data_, 1);
		}

		if (request_.contains("RECORD")) {
			data_ = invertarray(data_, 1);

		} else if (request_.contains("XML")) {
			if (data_) {
				call htmllib2("STRIPTAGS", data_);
				data_.replacer(_FM, "</" ^ fieldname ^
										">"
										"</record>"
										"\r\n"
										"<record><" ^
										fieldname ^ ">");
				data_.prefixer("<record><" ^ fieldname ^ ">");
				data_ ^= "</" ^ fieldname ^
						 ">"
						 "</record>";
				if (data_.contains(_VM)) {
					data_.replacer("</" ^ fieldname ^ ">", "</STOPPED>");
					data_.replacer(_VM, "</" ^ fieldname ^
											">"
											"<STOPPED>");
				}
			}
			data_.prefixer("<records>");
			data_ ^= "</records>";
		} else {
			// convert fm to vm in iodat
		}

		// also called from financeproxy to return voucher details for popup
	} else if (request1 eq "SELECT") {

		var filename0 = request2;
		if (not(VOLUMES)) {
			filename0.converter(".", "_");
			filename0.replacer("MEDIA_TYPE", "JOB_TYPE");
		}
		let filename   = filename0.field(" ", 1);
		var sortselect = request3;
		let dictids	   = request4;
		let options	   = request5;
		let maxnrecs   = request6;

		if (not(sortselect.contains("%SELECTLIST%"))) {
			clearselect();
		}

		var file;
		if (not(file.open(filename, ""))) {
			response_ = filename.quote() ^ " file does not exist in LISTEN SELECT";
			return 0;
		}

		gosub fileaccesscheck(filename);
		if (response_) {
			return 0;
		}

		// get the current program stack
		// programstack=program.stack(nostack)

		// and data passed to SELECT is assumed to be a selectlist

		if (data_) {
			makelist("", data_);
			sortselect ^= "%SELECTLIST%";
			data_ = "";
		}

		call select2(filename0, SYSTEM.f(2), sortselect, dictids, options, data_, response_, "", "", "", maxnrecs);
		// restore the program stack although this is done in LISTEN per request
		// rev has a limit on 299 "programs" and dictionary entries count as 1 each!
		// call program.stack(programstack)

		if (msg_) {
			response_ = trim(msg_.f(1), FM);
		} else {
			data_ = "%DIRECTOUTPUT%";
			// response='OK'
		}

	} else if (request1 eq "LISTLOCKS") {

		gosub getdostime();

		if (not(openfile("LOCKS", locks))) {
			call mssg("LISTEN2 CANNOT OPEN LOCKS FILE");
			return 0;
		}

		select(locks);
		var select2data = "";
		var nlocks		= 0;
nextlock:
		if (readnext(lockid)) {
			var lockx;
			if (lockx.read(locks, lockid)) {
				if (lockx.f(1) lt dostime) {
					goto nextlock;
				}
				nlocks += 1;
				select2data(nlocks, 1) = lockx.f(4) ^ _VM ^ lockx.f(3) ^ _VM ^ lockid.field("*", 1) ^ _VM ^ lockid.field("*", 2, 999);
				goto nextlock;
			}
		}

		if (tracing) {
			printl();
		} else {
			print(AT(0, 1));
		}
		var nn		 = select2data.count(FM);
		let maxlines = 20;
		if (not tracing) {
			nn = maxlines;
			if (nn gt maxlines) {
				nn = maxlines;
			}
			printl("+------Active-------+");
		}
		if (tracing) {
			printl(var("User").oconv("L#19"), " ", var("Station").oconv("L#19"), " ", var("File").oconv("L#19"), " ", var("Record").oconv("L#19"));
		}
		for (const int ii : range(1, nn)) {
			// /BREAK;
			if (tracing and nn gt 20)
				break;
			let row = select2data.f(ii);
			if (not tracing) {
				print("|");
			}
			tt = row.f(1, 1).trim();
			if (not tracing) {
				tt ^= " " ^ row.f(1, 3);
			}
			print(tt.oconv("L#19"));
			if (not tracing) {
				printl("|");
			}
			if (tracing) {
				printl(" ", row.f(1, 2).oconv("L#19"), " ", row.f(1, 3).oconv("L#19"), " ", row.f(1, 4).oconv("L#19"));
			}
		}  // ii;

		if (not tracing) {
			printl("+-------------------+");
		}

		select2data			= "";
		let select2response = "";

	} else if (request1 eq "UNLOCKLONGPROCESS") {

		// IF revRELEASE()>= 2.1 THEN
		// lockmode=36
		// unlockmode=37
		// END ELSE
		let lockmode   = 23;
		let unlockmode = 24;
		// END

		// NB ZZZ will hang if is not locked eg via unlock all
		// unlock long process unlimited cpu time flag
		tt = SYSTEM.f(48);
		if (tt) {
			// unlock in request loop using lockkey stored in system<48> by giveway
			// lock first to avoid hanging if try to unlock when not locked
			xx = "";
			call rtp57(lockmode, "", xx, tt, "", yy, zz);
			call rtp57(unlockmode, "", xx, tt, "", yy, zz);
			SYSTEM(48) = "";
		}

	} else if (request1 eq "STOPDB") {

		// equ install.end to request3
		// equ server.end to request4

		// check authorised
		tt = request2;
		if (not tt) {
			tt = "STOP";
		}
		if (not(authorised("DATABASE " ^ tt, msg_, "LS"))) {
			response_ = "Error: " ^ msg_;
			return 0;
		}

		if (request3.osfile() or serverend.osfile()) {
			// response='Error: Database already stopped/stopping'
			call listen4(19, response_);

		} else {

			//call oswrite("", request3);
			if (not oswrite("", request3))
				abort(lasterror());

			// stop server
			if (request2.contains("ALL")) {
				//call oswrite("", request4);
				if (not oswrite("", request4))
					abort(lasterror());
			}

			// wait up to 30 seconds for other users to quit
			let timex = time();
			while (true) {
				// /BREAK;
				if (not(otherusers().f(1) and ((time() - timex).abs() lt 30)))
					break;
				call ossleep(1000 * 1);
			}  // loop;

			data_ = "";

			let otherusersx = otherusers();
			if (otherusersx) {
				// response='Error: Could not terminate ':otherusersx<1>:' processes|':otherusersx<2>
				call listen4(20, response_, otherusersx);
				//request3.osremove();
				if (request3 and not request3.osremove())
					abort(lasterror());
			} else {
				//osshell("NET STOP EXODUSSERVICE");
				if (not osshell("NET STOP EXODUSSERVICE"))
					abort(lasterror());

				if (request2.starts("RESTART")) {
					//request3.osremove();
					if (request3 and not request3.osremove())
						abort(lasterror());
					//osshell("NET START EXODUSSERVICE");
					if (not osshell("NET START EXODUSSERVICE"))
						abort(lasterror());

				}

				response_ = "OK";
			}

			if (request2.contains("ALL")) {
				//request4.osremove();
				if (request4 and not request4.osremove())
					abort(lasterror());
			}
		}

	} else if (request1 eq "BACKUP") {

		// similar code in LISTEN and LISTEN2

		// gosub getbakpars
		call getbackpars(bakpars);

		// backup may respond to user itself if it starts
		msg_ = "";
		perform("FILEMAN BACKUP " ^ SYSTEM.f(17) ^ " " ^ bakpars.f(7));

		// if backup has already responded to user
		// then quit and indicate to calling program that a backup has been done
		// user will be emailed
		if (SYSTEM.f(2) eq "") {
			PSEUDO = "BACKUP2 " ^ bakpars.f(7);
			if (msg_) {
				stop();
			}
		}

		response_ = msg_;
		response_.converter(_FM _VM, "\r\r");

		call sysmsg(response_, "EXODUS Backup");

		if (response_.ucase().contains("SUCCESS")) {
			response_.prefixer("OK ");
		}

		// note: if backup did not respond already then the requestexit will
		// respond as usual with the error message from backup
		data_ = "";

	} else {
		printl(request1.quote(), " invalid request in LISTEN5");
	}

	return 0;
}

subroutine getdostime() {
	dostime = ostime();
	// convert to Windows based date/time (ndays since 1/1/1900)
	// 31/12/67 in rev date() format equals 24837 in windows date format
	dostime = 24837 + date() + dostime / 24 / 3600;
	return;
}

subroutine fileaccesscheck(in filename) {
	response_ = "";

	var securityfilename = filename;
	if (filename eq "JOURNALS") {
		securityfilename = "JOURNALS";
	}

	// security check
	// dont check markets and companies because really must have access to some
	// and eliminates need for clumsy task COMPANY ACCESS PARTIAL task
	if (not(var("MENUS,ADMENUS").locateusing(",", filename, xx))) {
		var temp = securityfilename;
		temp.converter(".", " ");
		temp = singular(temp);
		if (not(authorised(temp ^ " ACCESS", msg_, ""))) {
			// we could use securityfilename LIST instead of securityfilename ACCESS PARTIAL clumsy
			// ie list without general access means there are some records
			// specifically allowed
			if (not(authorised("!#" ^ temp ^ " ACCESS PARTIAL", msg2, ""))) {
				// if there is an authorised dictionary item then leave it up to that
				if (not((var("AUTHORISED").xlate("DICT." ^ filename, 8, "X")).contains("ALLOWPARTIALACCESS"))) {
					response_ = msg_;
					return;
				}
			}
		}
		msg_ = "";
	}

	return;
}

subroutine deleteoldfiles() {

	let deletetime = date() * 24 * 60 * 60 + time() - ageinsecs;

	let filespec = (inpath ^ pattern).lcase();

	// failsafe - only allow delete .* in data folder
	if (filespec.ends(".*")) {
		var tdir = "/data/";
		tdir.converter("/", OSSLASH);
		if (not(filespec.contains(tdir))) {
			return;
		}
	}

	// for each suitable file
	var filenamesx = oslistf(filespec);

nextfiles:
	if (filenamesx) {

		// get the file time
		let filename0 = filenamesx.field(FM, 1);
		filenamesx.paster(1, filenamesx.field(FM, 1).len() + 1, "");

		var filename = inpath ^ filename0;

		// replaced by databasecode.SVR
		// if filename0='GLOBAL.SVR' then goto deleteit

		if (not(var(".jpg,.png,.gif,.svr,.cfg").locateusing(",", (filename.last(4)).lcase(), xx))) {

			// a file ending .4 is a request to delete the .2 and .3 files
			if (filename.ends(".4")) {
				//filename.osremove();
				if (filename and not filename.osremove())
					abort(lasterror());
				filename.paster(-1, 1, "2");
				//filename.osremove();
				if (filename and not filename.osremove())
					abort(lasterror());
				filename.paster(-1, 1, "3");
				//filename.osremove();
				if (filename and not filename.osremove())
					abort(lasterror());

			} else {
				if (filename.ends(".TMP")) {
					goto deleteit;
				}
				// and delete it if older than the cut off time
				// and has a file extension (ie leave PARAMS and PARAMS2)
				fileattributes = filename.osfile();
				filetime	   = fileattributes.f(2) * 24 * 60 * 60 + fileattributes.f(3);
				if (((filename.last(4)).contains(".")) and filetime le deletetime) {
deleteit:
					//filename.osremove();
					if (filename and not filename.osremove())
						abort(lasterror());
				} else {
				}
			}
		}

		goto nextfiles;
	}

	return;
}

libraryexit()
