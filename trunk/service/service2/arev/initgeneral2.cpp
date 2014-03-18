#include <exodus/library.h>
libraryinit()

#include <log2.h>
#include <getenvstr.h>

#include <gen.h>

var logtime;
var vn;

function main(io mode) {

	if (mode == "UPDATECONNECTIONS") {
		var gbp;
		if (not(gbp.open("GBP", ""))) {
			return 0;
		}
		var hosts;
		if (not(hosts.read(gbp, "$HOSTS.ALLOW"))) {
			return 0;
		}

		//convert hosts.allow format to sv list of valid ip numbers or partial ip nos
		hosts.swapper("sshd:", "");
		hosts.converter(" ", "");
		var nn = hosts.count(FM) + 1;
		for (var ln = nn; ln >= 1; --ln) {
			hosts.r(ln, hosts.a(ln).field("#", 1));
		};//ln;

		//remove blank lines and convert fm to sm
		hosts.converter(FM, " ");
		hosts.trimmer();
		hosts.converter(" ", SVM);

		//remove any trailing . from 10. etc which is valid syntax for hosts.allow
		hosts.swapper("." ^ SVM, SVM);
		if (hosts.substr(-1, 1) == ".") {
			hosts.splicer(-1, 1, "");
		}

		//neosys can login from
		//1) 127 always
		//2) NEOSYS office/vpn static ips (hardcoded in each neosys version)

		//neosys can also login from System Configuration ip ranges and numbers
		//default system configuration ips is standard lan ips "192.168 10 172"
		//3) standard LAN ips 192.168, 10, 172 RANGES BUT NOT FULLY FORMED IPS
		//4) Config File Fully formed WAN ips BUT NOT RANGES
		//5) NB the above 2 rules mean "*" for all is NOT allowed even if present

		//WARNING to disallow NEOSYS login from outside the office via NAT router
		//(which makes the connection appear like a LAN access)
		//you MUST put the FULLY FORMED LAN IP of the router eg 192.168.1.1
		//in the System Configuration file (even if 192.168 is config for true LAN)
		//then NEOSYS LOGINS FROM ANY *LISTED FULLY FORMED LAN IPS* WILL BE BLOCKED

		var configips = SYSTEM.a(39);
		if (configips == "") {
			configips = "192.168 10 172";
		}
		configips.converter(" ", SVM);
		configips.swapper(".*", "");
		nn = configips.count(SVM) + (configips ne "");
		for (var ii = nn; ii <= 1; ++ii) {
			var ipno = configips.a(1, 1, ii);
			if (ipno.field(".", 1) == "10") {
				goto 344;
			}
			if (ipno.field(".", 1) == "172") {
				goto 344;
			}
			if (ipno.field(".", 1, 2) == "192.168") {
			}else{
				//delete all WAN (non-LAN) ranges and allow only fully specced WAN ips
				if (ipno.count(SVM) ne 3) {
					configips.eraser(1, 1, ii);
				}
			}
		};//ii;

		//allow neosys login from 127.* and any default ips
		hosts.inserter(1, 1, 1, "127.0.0.1");
		hosts.inserter(1, 1, 2, "127");
		hosts.inserter(1, 1, 3, configips);

		hosts.write(gen._definitions, "IPNOS*NEOSYS");

		goto 743;
	}
	if (mode == "GETENV") {
		call log2("*get environment", logtime);
		//nb will NOT overwrite any manual entries in SYSTEM.CFG
		//environment=shell2('SET','')
		var environment = getenvstr();
		environment.converter("\r\n", FM);
		var nenv = environment.count(FM) + 1;
		for (var ii = 1; ii <= nenv; ++ii) {
			var enventry = environment.a(ii);
			if (enventry) {
				var tt = enventry.field("=", 1);
				if (not(SYSTEM.a(12).locateusing(tt, VM, vn))) {
					SYSTEM.r(12, vn, tt);
					SYSTEM.r(13, vn, enventry.field("=", 2, 99999));
				}
			}
		};//ii;

		goto 743;
	}
	if (mode == "FIXURLS") {

		call log2("*condition the http links");
		//remove any obvious page addresses
		//ensure ends in slash
		var baselinks = SYSTEM.a(114);
		var baselinkdescs = SYSTEM.a(115);
		if (not baselinks) {
			var baselink = "System Configuration File";
		}
		if (not baselinkdescs) {
			baselinkdescs = "Pending Configuration";
		}
		var nlinks = baselinks.count(VM) + (baselinks ne "");
		for (var linkn = 1; linkn <= nlinks; ++linkn) {
			var tt = baselinks.a(1, linkn);
			if (tt) {
				var tt2 = field2(tt.ucase("/", -1));
				if (tt2.index(".HTM", 1)) {
					tt.splicer(-tt2.length(), tt2.length(), "");
				}
				if (not(var("\\/").index(tt.substr(-1, 1), 1))) {
					tt ^= "/";
				}
				baselinks.r(1, linkn, tt);
			}
		};//linkn;
		SYSTEM.r(114, baselinks);
		SYSTEM.r(115, baselinkdescs);

		goto 743;
	}
	if (1) {
		//call msg(quote(mode):' is invalid in init.general2')
	}
L743:
	return 0;

}


libraryexit()