#include <exodus/library.h>
libraryinit()

#include <getenv.h>
#include <sysmsg.h>
#include <shell2.h>

#include <gen.h>

var osusername;
var ID;
var errors;

function main(io mode) {

	//jbase

	//$insert gbp,arev.common2
	//clear common
	//valid=1
	USER4 = "";

	//test -f ~/.ssh/id_dsa || \
	// ssh-keygen -t dsa -b 1024 && \
	// chmod 600 ~/.ssh/id_dsa

	if (SENTENCE.field(" ", 1) == "SSH") {
		mode = SENTENCE.field(" ", 2);
		if (not mode) {
			call mssg("Syntax is SSH init/start/stop");
			var().stop();
		}

		SENTENCE = "";
		USER3 = "";
		USER4 = "";
		call ssh(mode);

		if (USER3) {
			USER3 = USER3.oconv("T#60");
			USER3.converter(TM, FM);
			USER3.trimmer();
			call mssg(USER3);
		}

		var().stop();

		goto 663;
	}
	if (mode == "INIT") {

getssh:

		call getenv("USERNAME", osusername);
		var idpath = SYSTEM.a(50) ^ "..\\home\\" ^ osusername ^ "\\ssh~1\\ID_DSA.PUB";
		var triedmakingit = 0;
getpubliccert:
		call osread(ID, idpath);
		if (ID) {
			if (triedmakingit) {
				call sysmsg(idpath ^ FM ^ ID);
			}
			USER3 = "OK " ^ ID;
			return 0;
		}

		if (triedmakingit) {
			USER3 = "ERROR: Cant find " ^ idpath;
			USER3.r(-1, "ssh-keygen doesnt seem to have worked");
		}else{

			var cmd = SYSTEM.a(50) ^ "bash --login -i -c ";
			cmd ^= "\"test -f ~/.ssh/id_dsa";
			cmd ^= " || ssh-keygen -t dsa -b 1024 -f ~/.ssh/id_dsa -N \\\"\\\"";
			cmd ^= " && chmod 600 ~/.ssh/id_dsa\"";
			call shell2(cmd, errors);
			if (not errors) {
				triedmakingit = 1;
				goto getpubliccert;
			}

			errors.transfer(USER3);
		}

		goto 663;
	}
	if (mode == "START") {

		gosub stopssh();

		gosub getssh();
		if (USER3.substr(1, 2) ne "OK") {
			return 0;
		}

		//construct a random remote port from '4' and the first 4 digits of the cid
		//49999
		var rport1 = 4 ^ (SYSTEM.a(111)).substr(1, 4);
		var rport2 = rport1 + 1;
		var lport1 = 19580;
		var lport2 = 3389;
		var USER = "neosystunnel";
		var host = "tun.neosys.com";
		var sshport = 19580;

		var cmd = "ssh -v -p " ^ sshport;
		var tt = "*:";
		//tt=''
		cmd ^= " -R " ^ tt ^ rport1 ^ ":localhost:" ^ lport1;
		cmd ^= " -R " ^ tt ^ rport2 ^ ":localhost:" ^ lport2;
		cmd ^= " " ^ USER ^ "@" ^ host;
		cmd ^= " -N";
		cmd ^= " -o StrictHostKeyChecking=no";
		//needed in server sshd_config to listen on any iface instead of just 127.0.0.1
		if (tt) {
			cmd ^= " -o GatewayPorts=yes";
		}
		var cmd2 = SYSTEM.a(50) ^ cmd;
		//omit the following to get any ssh error messages but process hangs if ok
		cmd2.splicer(1, 0, "start \"NEOSYS_SSH\" /MIN cmd /c ");

		cout << endl;
		cout << cmd2;
		cout << " ";
		call shell2(cmd2, errors);
		if (errors) {
			USER3 = "ERROR: " ^ errors;
			var().stop();
		}
		USER3 = "OK " ^ cmd;

		goto 663;
	}
	if (mode == "STOP") {
}

subroutine stopssh() {
		var cmd = "TASKKILL /T /F /FI \"WINDOWTITLE eq NEOSYS_SSH\"";
		call shell2(cmd);

		goto 663;
	}
	if (1) {
		USER3 = DQ ^ (mode ^ DQ) ^ " invalid mode in SSH()";
	}
L663:
	return;

}


libraryexit()