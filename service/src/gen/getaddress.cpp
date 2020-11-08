#include <exodus/library.h>
libraryinit()

#include <gen_common.h>

function main(in addrcodes0, io taxregno) {
	//c gen in,io

	//!!! CALLER MUST INITIALISE common variable address

	//1) receives and passes COMMON variable "address"
	//2) gets addresses if fm list of address codes provided and any rec exists
	//3) adds tax reg no if present
	//4) formats into html breaking between but not within lines

	#include <general_common.h>

	//try to get address and taxregno if codes provided

	//address is in general.common
	//address='' default address can be passed in and MUST be initialised in CALLER!
	taxregno = "";

	var addrcodes = addrcodes0;

nextaddrcode:
/////////////
	if (addrcodes) {

		var addrcode = addrcodes.a(1);
		addrcodes.remover(1);

		if (addrcode == "") {
			goto nextaddrcode;
		}

		var addrec;
		if (not(addrec.read(gen.addresses, addrcode))) {
			goto nextaddrcode;
		}

		//whoopie! got an address record

		//prefer the four lines of address if present on the address record
		//otherwise use the given address
		var tt = trim(trim(addrec.a(4), VM), TM);
		if (tt) {
			gen.address = tt;
		}

		//add country if present
		if (addrec.a(8)) {
			gen.address.r(1, -1, addrec.a(8));
		}

		//return Tax Reg No. if present
		taxregno = addrec.a(28);

	}

	//format the input or obtained address
	/////////////////////////////////////
	//garbagecollect;
	gen.address.swapper(VM, ",");
	gen.address.swapper(TM, ",");

	//allow break on commas ONLY
	if (gen.address.index(",")) {
		gen.address = "<nobr>" ^ gen.address ^ "</nobr>";
	}
	gen.address.swapper(", ", ",");
	gen.address.swapper(",", ",</nobr> <wbr/><nobr>");

	//IE11 ignores wbr but folds on UNICODE ZERO WIDTH SPACE
	//swap '<wbr/>' with '<wbr/>':'&#x200b;' in address

	return 0;
}

libraryexit()
