#include <exodus/library.h>
libraryinit()

#include <cid.h>
#include <secid.h>


var config;
var authnusers;//num

function main() {
	//compile without line numbers - why?
	//j b a s e

	if (not(config.osread("NE" "O" "SYS" "." "C" "F" "G"))) {
		config = "";
	}

	//if already checked today then skip
	//IF INVERT(HASH(DATE()[-1,-4],PRIKEY,0)) EQ CONFIG<3> THEN RETURN CONFIG

	//get c number/required authorisation number
	var cidx = cid();
	//diskette=@ans
	var secidx = secid(cidx, "1000" "100");
	var msecid = secid("m" ^ cidx, "1000" "100");

	//check authorisation number is correct otherwise ask for the number
	//sjb nov98 IF INVERT(CONFIG<2>) NE secidx and invert(config<2>)ne msecid THEN
	//if index(config<2>,invert(secidx),1) or index(config<2>,invert(msecid),1) else
	if ((config.a(2)).index(msecid.invert(), 1)) {
		authnusers = 999;
		goto 158;
	}
	if ((config.a(2)).index(secidx.invert(), 1)) {
		authnusers = 1;
		goto 158;
	}
	if (1) {
		authnusers = 0;
	}
L158:
	if (not authnusers) {
		var reply = "";
getcompauth:
		var().chr(7).output();
		var msg = "This computer is number " ^ cidx ^ FM;
		msg.r(-1, "What is the authorisation number ?");
		msg.r(-1, "Please contact NEOSYS and quote the computer");
		msg.r(-1, "number above to get an authorisation number." ^ FM);

		if (SYSTEM.a(33, 4)) {
			reply = SYSTEM.a(33, 4);
		}else{
			call note2(msg, "RC", reply, "");
		}

		if (not reply) {
			return "";
		}

		if (reply == secidx) {
			authnusers = 1;
		}
		if (reply == msecid) {
			authnusers = 999;
		}

		//IF REPLY NE secidx and reply ne msecid THEN
		if (not authnusers) {
			var().chr(7).output();
			call mssg(DQ ^ (reply ^ DQ) ^ " IS NOT THE CORRECT|AUTHORISATION NUMBER - TRY AGAIN");
			if (SYSTEM.a(33)) {
				return "";
			}
			goto getcompauth;
		}
		config.r(2, -1, reply.invert());
		config.r(4, -1, cidx.invert());
		//config<6>=diskette
	}

exit:
	config.r(3, (((var().date()).substr(-1, -4)).hash("1000" "100", 0)).invert());
	call oswrite(config, "NEO" "SYS" "." "C" "F" "G");

	return authnusers;

}


libraryexit()