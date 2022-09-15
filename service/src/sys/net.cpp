#include <exodus/library.h>
libraryinit()

#include <log.h>
#include <sysmsg.h>
#include <getbackpars.h>
#include <sendmail.h>
#include <upgrade.h>

#include <sys_common.h>

var tt;//num
var bakpars;
var ver;

function main() {
	//c sys

	#include <system_common.h>

	//if @username='EXODUS' then break on
	//BREAK ON;

	var cmd = "LISTEN " ^ SENTENCE.field(" ", 2, 9999);

	PSEUDO = "";
	USER4 = "";
	PRIORITYINT(100) = "";
	var dbcode = SYSTEM.f(17);
	if (dbcode eq "") {
		dbcode = "DEFAULT";
	}

	if (not(DEFINITIONS.open("DEFINITIONS", ""))) {
		call fsmsg();
		stop();
	}

listen:

	var s33 = SYSTEM.f(33);

	//forces OFF in listen on esc
	if (cmd.field(" ",1) ne "LISTEN") {
		SYSTEM(33) = 1;
	}

	if (osgetenv("EXO_DEBUG"))
	 execute(cmd);
	else try {

	execute(cmd);

	}
	catch (VarError varerror) {
		// Similar code in net.cpp and listen.cpp
	    USER4 = varerror.description.unassigned("No error message") ^ FM ^ backtrace();
	}

	//unlock all
	var xx = unlockrecord();

	SYSTEM(33) = "";

	//trim off uninteresting back trace into listen
	//leave for now in case of errors in listen while converting to exodus
	//tt=index(@user4,fm:'listen.cpp',1)
	//if tt then
	// @user4[tt,99999]=''
	// end

	if (USER4.substr(1, 7) eq "RESTART") {

		if (USER4 eq "RESTART $LISTEN") {
			SYSTEM(100, 3) = "";
			SYSTEM(33) = s33;
			goto listen;
		}

		RELOAD_req = true;
		printl(THREADNO ^ ":", USER4);

		stop();
		////

	}

	if (USER4 eq "" or USER4 eq "TERMINATED OK") {
		stop();
		////
	}

	//stop if cant backup because another process is backing up or hung processes
	if (USER4.contains("FILEMAN-SHUTDOWN")) {
		perform("OFF");
		logoff();
	}

	//check lists and indexing files are not corrupted and zero them if they are
	//listen selects the locks file every 10 secs and may detect corruption
	s33 = SYSTEM.f(33);
	SYSTEM(33) = 1;
	//call checkfile"LISTS");
	//call checkfile"!INDEXING");
	//call checkfile"LOCKS");
	SYSTEM(33) = s33;

	//detect memory corruption?
	var halt = 0;
	if (USER4.contains("R18.6")) {
		halt = 1;
		USER4(-1) = "Corrupt temporary file. Restart Needed.";
		USER4(-1) = "exodus.net TERMINATED";
	}
	if (USER4.ucase().contains("NOT ENOUGH MEMORY")) {
		halt = 1;
	}
	if (USER4.ucase().contains("OUT OF MEMORY")) {
		halt = 1;
	}

	//convert error messages
	var normal = 0;
	if (USER4.contains("INDEX.REDUCER")) {
		normal = 1;
		USER4 = "Error: Please select fewer records";
	}

	if (not normal) {

		//determine subject
		var subject = "";
		var attachfilename = "";

		//either BACKUP or BACKUP2 followed by space and drive letter
		if (PSEUDO.substr(1, 6) eq "BACKUP") {

			subject = "EXODUS Backup " ^ dbcode;

			//add drive letter(s)
			tt = PSEUDO.field(" ", 2).substr(1, 2);
			var tt2 = PSEUDO.field(" ", 3).substr(1, 2);
			subject ^= " -> " ^ tt;
			if (tt2 and tt2 ne tt) {
				subject ^= "/" ^ tt2;
			}

			//note new or changed media
			tt = USER4.contains("Media: ");
			if (tt) {
				subject ^= " " ^ (USER4.substr(tt + 7, 9999)).f(1);
			}

			//add success, WARNING or FAILURE
			tt = "";
			tt2 = USER4.ucase();
			if (not(tt2.contains("SUCCESS")) or tt2.contains("FAIL")) {
				tt = "FAILURE";
			} else {
				if (tt2.contains("WARNING")) {
					tt = "WARNING";
				}
				attachfilename = SYSTEM.f(42);
			}
			//no need for specific success message. most succeed, can find FAIL or WARN
			//if tt='' then tt='Success'
			subject ^= " " ^ tt;

			//add exodus version for info
			//call osread(versionnote,'general\version.dat')
			//versiondate=iconv(field(trim(versionnote),' ',2,3),'D')
			//subject:=' Ver: ':versiondate 'D4/J'

			//log it
			var errormsg = USER4;
			call log(cmd, errormsg ^ "");

		} else {

			//subject='EXODUS Technical Message :'
			subject = "";
			if (VOLUMES) {
				printl(USER4);
			}
			var techmsg = USER4.f(1).substr(1, 256);
			call sysmsg(USER4, techmsg);

		}

		if (subject) {

			//get backup parameters
			call getbackpars(bakpars);

			//send email of error
			//readv address from definitions,'BACKUP',6 else address=''
			sys.address = bakpars.f(6);
			if (bakpars.f(10)) {
				if (sys.address) {
					sys.address ^= "/";
				}
				sys.address ^= bakpars.f(10);
			}
			if (sys.address eq "") {
				if (not(sys.address.readv(DEFINITIONS, "REPLICATION", 12))) {
					sys.address = "";
				}
			}
			if (sys.address eq "") {
				sys.address = "backups@neosys.com";
			}
			if (sys.address) {

				var body = "Server=" ^ SYSTEM.f(44).trim();
				body(-1) = "Client=" ^ STATION.trim();
				body(-1) = "User=" ^ USERNAME.trim();
				//osread ver from 'general\version.dat' then
				var verfilename = "general/version.dat";
				verfilename.converter("/", OSSLASH);
				if (ver.osread(verfilename)) {
					body(-1) = "EXODUS Ver:" ^ ver.f(1);
					}

				//too slow so ignore it
				//servername=getdrivepath(drive()[1,2])[3,'\']
				//if servername then body:=fm:'ServerName=':servername

				body ^= FM ^ FM ^ USER4;
				body.converter(FM ^ VM ^ SM ^ TM ^ ST ^ "|", "\r" "\r" "\r" "\r" "\r" "\r");
				body.replacer("\r", "\r\n");

				//sendmail - if it fails, there will be an entry in the log
				var address1 = sys.address.field("/", 1);
				var errormsg = "";
				if (address1) {
					call sendmail(address1, "", subject, body, "", "", errormsg);
				}

				//optionally email backup.zip
				if ((errormsg eq "" or errormsg.substr(1, 2) eq "OK") and attachfilename) {
					var address2 = sys.address.field("/", 2);
					//remove exodus from the backup.zip recipients
					if (address2.locateusing(";", "backups@neosys.com", xx)) {
						address2.converter(";", VM);
						address2.remover(1, xx);
						address2.converter(VM, ";");
					}
					if (address2) {
						call sendmail(address2, "", subject, body, attachfilename);
					}
				}

			}

		}

		//terminate after automatic backup in order to allow os filesystem backups
		//BACKUP2 means dont copydb/upgrade/quit - just resume
		if (PSEUDO.field(" ", 1) eq "BACKUP") {

			//before termination do any copy to testdata etc
			if (SYSTEM.f(58).locate(dbcode, tt)) {
				if (SYSTEM.f(62, tt)) {
					perform("COPYDB " ^ dbcode);
				}
			}

			//also before termination do any upgrade
			//why call not perform?
			call upgrade();

			if (SYSTEM.f(125)) {
				perform("OFF");
				logoff();
			}
		}

	}

	// Respond to web client e.g. program crash due to Variable not Assigned
	var linkfilename3 = PRIORITYINT.f(100);
	if (linkfilename3) {
		//cannot remove these since they may be codepage letters now
		USER4.replacer("|", "\r\n");
		USER4.replacer(FM, "\r\n");
		USER4.replacer(VM, "\r\n");
		call oswrite("Error: " ^ USER4, linkfilename3);
		//osclose linkfilename3
	}

	//restore the original settings
	//noninteractive avoids any further messages on OFF command
	SYSTEM(33) = s33;
	if (SYSTEM.f(43)) {
		USERNAME=(SYSTEM.f(43));
	}
	//if system<44> then call sysvar_109_110('SET',system<44>)
	if (SYSTEM.f(44)) {
		STATION=(SYSTEM.f(44));
	}

	if (halt) {
		perform("OFF");
		logoff();
	}

	if (cmd.field(" ",1) eq "LISTEN") {
		//if NET LISTEN LISTEN LISTEN - then terminate if too many errors
		if (cmd.len() gt 100) {
			printl(USER4);
			perform("OFF");
		}
		cmd.replacer(" INTRANET", "");
		chain("NET " ^ cmd);
	}

	return 0;
}

libraryexit()
