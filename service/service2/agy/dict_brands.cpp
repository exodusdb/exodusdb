#include <exodus/library.h>

libraryinit(authorised)
#include <validcode2.h>
#include <validcode3.h>
function main()  {
	//also in client.subs
	//check the company, client and brand
	var ans;
	var msg;
	var xx;
	if (validcode2(RECORD.a(10), RECORD.a(1, 1), ID, xx, msg)) {
		ans = 1;
	}else{
		ans = 0;
	}
	//check the market
	if (ans) {
		if (not(validcode3(RECORD.a(13), "", "", xx, msg))) {
			ans = 0;
		}
	}
	return ans;
}
libraryexit(authorised)


libraryinit(brand_name)

function main()  {
	var ans = RECORD.a(2);
	var codes = RECORD.a(8);
	var code = ID;
	var nn = codes.count(VM) + 1;
	var alphanum = UPPERCASE ^ LOWERCASE ^ "0123456789";
	for (var ii = nn - 1; ii >= 1; --ii) {
		var newcode = codes.a(1, ii);
		if (newcode ne code) {
			code = newcode;
			if (var("!\"#$%^&*()_+-={}[]:@;\\<>,.").index(ans[1])) {
				ans.splicer(1, 0, " ");
			}else{
				ans.splicer(1, 0, " - ");
			}
			ans.splicer(1, 0, code.xlate("BRANDS", 2, "C"));
		}
	};//ii;
	return ans;
}
libraryexit(brand_name)


libraryinit(client_name)

function main()  {
	ANS = RECORD.a(3);
	if (not ANS) {
		ANS = calculate("CLIENT_CODE").xlate("CLIENTS", 1, "X");
	}
	return ANS;
}
libraryexit(client_name)


libraryinit(client_name2)

function main()  {
	return RECORD.a(1).xlate("CLIENTS", 1, "X");
}
libraryexit(client_name2)


libraryinit(name)

function main()  {
	ANS = RECORD(2);//"???" ^ ID;
	var clients;
	if (clients.open("", "CLIENTS")) {
		var client;
		if (client.read(clients, calculate("CLIENT_CODE"))) {
			var vn;
			if (client.locate(ID, vn, 2)) {
				ANS = client.a(3, vn);
			}
		}
	}
	return ANS;
}
libraryexit(name)


libraryinit(no_of_jobs)

function main()  {
	//call pushselect(0, v69, v70, v71);
	//call safeselect("SELECT JOBS WITH BRAND_CODE " ^ (DQ ^ (ID ^ DQ)) ^ " (S)");
	//ANS = RECCOUNT;
	//call popselect(0, v69, v70, v71);
	//return ANS;
	return 9999;
}
libraryexit(no_of_jobs)


libraryinit(no_of_schedules)

function main()  {
	//call pushselect(0, v69, v70, v71);
	//call safeselect("SELECT SCHEDULES WITH BRAND_CODE " ^ (DQ ^ (ID ^ DQ)) ^ " (S)");
	//ANS = RECCOUNT;
	//call popselect(0, v69, v70, v71);
	//return ANS;
	return 9999;
}
libraryexit(no_of_schedules)
libraryinit(rawname)

function main()  {
	ANS = RECORD.a(2);
	ANS.ucaser();
	ANS.converter("\'" " \".,/-", "");
	return ANS;
}
libraryexit(rawname)


libraryinit(sequence)

function main()  {
	ANS = RECORD.a(3) ^ " - " ^ RECORD.a(2) ^ " - " ^ ID;
	ANS.ucaser();
	return ANS;
}
libraryexit(sequence)


libraryinit(sequencexref)
#include <xref.h>
function main()  {
	call xref(calculate("SEQUENCE"), "\x20\x22\x20\x5F\x56\x4D\x5F\x20\x22\x22\x20\x5F\x53\x4D\x5F\x20\x22\x22\x20\x5F\x54\x4D\x5F\x20\x22\x2E\x2C\x2D\x2F\x28\x29\x5C\x5C\x2B\x24\x25\x26\x2A\x2B\x3D\x7B\x7D\x5B\x3C\x3E\x3F\x3B\x3A\x7C\x5C\x22\x5C\x27", "", "1");
	return ANS;
}
libraryexit(sequencexref)


libraryinit(stopped)

function main()  {
	return RECORD.a(1, 1).xlate("CLIENTS", 35, "X");
}
libraryexit(stopped)


libraryinit(stopped2)

function main()  {
	return RECORD.a(1, 1).xlate("CLIENTS", 35, "X") ne "";
}
libraryexit(stopped2)


libraryinit(year_period)

function main()  {
	return "";

}

libraryexit(year_period)


