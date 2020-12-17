#include <exodus/library.h>
libraryinit()

#include <whois.h>
#include <readhostsallow.h>
#include <shell2.h>

#include <gen_common.h>

var sentencex;
var ip12;
var ip1;
var ip2;
var allowedips;
var xx;
var cmd;
var exe;
var errors;

function main(in mode, in ipno, out text) {
	//c sys in,in,out

	#include <general_common.h>
	//global allowedips,ip1,ip2

	//prevent unused warning
	if (mode) {
		{}
	}

	if (SENTENCE.field(" ", 1) == "WHOIS") {
		SENTENCE.transfer(sentencex);
		call whois("", sentencex.field(" ", 2), text);
		call mssg(text);
		stop();
	}

	//skip standard LAN or EXODUS ip no and exit - no delay for these ips
	//medialine lan is 100.100.100.* iana "carrier grade NAT"
	//test in order of frequency installed at clients

	ip12 = ipno.field(".", 1, 2);
	if (ip12 == "192.168") {
		goto returnzero;
	}

	//skip 10.*.*.*
	ip1 = ipno.field(".", 1);
	if (ip1 == "10") {
		goto returnzero;
	}

	//skip 172.16-32.*.* and 100.64-127.*.*
	ip2 = ipno.field(".", 2);
	if (((ip1 == 172) and (ip2 >= 16)) and (ip2 <= 31)) {
		goto returnzero;
	}
	if (((ip1 == 100) and (ip2 >= 64)) and (ip2 <= 127)) {
		goto returnzero;
	}

	//skip 127.*.*.*
	if (ip1 == "127") {
		goto returnzero;
	}

	//skip EXODUS internet ip numbers and those in system configuration file
	//ZZZ should really detect net ranges like /24 and .*
	//allowedips=' ':xlate('GBP','$HOSTS.ALLOW','','X'):' '
	call readhostsallow(allowedips);
	allowedips.r(-1, SYSTEM.a(39));
	allowedips.converter(FM ^ VM ^ "," "\r\n", var(100).space());
	if (allowedips.locateusing(" ",ipno,xx)) {
returnzero:
		text = "0";
		return 0;
	}

	cmd = SYSTEM.a(50);

	//check cygwin whois present otherwise quit
	//WARNING TODO: check ternary op following;
	exe = oscwd().index(":") ? ".exe" : "";
	if (not((cmd ^ "whois" ^ exe).osfile())) {
		text = "whois command is not installed";
		return 0;
	}

	//build the command
	cmd ^= "whois " ^ ipno;

	//dont wait for a long time - in case internet access is blocked
	if ((cmd ^ "timeout.exe").osfile()) {
		cmd.splicer(1, 0, SYSTEM.a(50) ^ "timeout 5 ");
	}

	//capture the output of the whois command
	text = shell2(cmd, errors);

	if (errors) {
		text ^= errors;
	}

	text.swapper("\r\n", FM);
	text.swapper("\n", FM);

	return 0;
}

libraryexit()
