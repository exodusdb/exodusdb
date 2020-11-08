#include <exodus/library.h>

libraryinit(accname)
//------------------
function main() {
	return ("." ^ ID).xlate("ACCOUNTS", 1, "X");
}
libraryexit(accname)

libraryinit(address1)
//-------------------
function main() {
	return RECORD.a(4, 1);
}
libraryexit(address1)

libraryinit(address2)
//-------------------
function main() {
	return RECORD.a(4, 2);
}
libraryexit(address2)

libraryinit(address3)
//-------------------
function main() {
	return RECORD.a(4, 3);
}
libraryexit(address3)

libraryinit(address4)
//-------------------
function main() {
	return RECORD.a(4, 4);
}
libraryexit(address4)

libraryinit(address5)
//-------------------
function main() {
	return RECORD.a(4, 5);
}
libraryexit(address5)

libraryinit(address6)
//-------------------
function main() {
	return RECORD.a(4, 6);
}
libraryexit(address6)

libraryinit(address7)
//-------------------
function main() {
	return RECORD.a(4, 7);
}
libraryexit(address7)

libraryinit(address8)
//-------------------
function main() {
	return RECORD.a(4, 8);
}
libraryexit(address8)

libraryinit(address_text)
//-----------------------

function main() {
	ANS = trim(RECORD.a(4), VM);
	ANS.converter(VM, TM);
	return ANS;
}
libraryexit(address_text)

libraryinit(address_txt)
//----------------------
function main() {
	var ans = "";
	ans ^= VM ^ "" ^ calculate("ADDRESS_REF") ^ "," ^ calculate("FIRST_PERSON") ^ "," ^ calculate("PERSONAL_COMMENTS") ^ "," ^ calculate("COMPANY") ^ "," ^ calculate("ADDRESS1") ^ "," ^ calculate("ADDRESS2") ^ "," ^ calculate("ADDRESS3") ^ "," ^ calculate("ADDRESS4") ^ "," ^ calculate("ADDRESS5") ^ "," ^ calculate("COUNTRY") ^ "," ^ calculate("TEL") ^ "," ^ calculate("TLX") ^ "," ^ calculate("FAX") ^ "," ^ calculate("COMMENTS") ^ "," ^ calculate("CATEGORY");

	ANS = ans;
	ANS.swapper(VM, "\r\n");
	ANS.splicer(1, 2, "");
	return ANS;
}
libraryexit(address_txt)

libraryinit(comments2)
//--------------------
function main() {
	var ans = calculate("COMMENTS");
	var temp = calculate("DATE_ENTERED");
	if (temp) {
		ans.r(1, -1, "Entered:" ^ oconv(temp, DATEFMT));
	}
	temp = calculate("DATE_UPDATED");
	if (temp) {
		ans.r(1, -1, "Updated:" ^ oconv(temp, DATEFMT));
	}
	return ans;
}
libraryexit(comments2)

libraryinit(company_xref)
//-----------------------
#include <xref.h>

function main() {
	call xref(calculate("COMPANY"), "\x20\x22\x20\x5F\x56\x4D\x5F\x20\x5F\x53\x4D\x5F\x20\x5F\x54\x4D\x5F\x20\x22\x2E\x2C\x2D\x2F\x28\x29\x5C\x5C\x2B\x24\x25\x26\x2A\x2B\x3D\x7B\x7D\x5B\x3C\x3E\x3F\x3B\x3A\x7C\x5C\x22\x5C\x27", "", "1");
	return ANS;
}
libraryexit(company_xref)

libraryinit(company_part1)
//------------------------
function main() {
	return RECORD.a(1).oconv("T#30").substr(1,30);
}
libraryexit(company_part1)

libraryinit(company_part2)
//------------------------
function main() {
	return RECORD.a(1).oconv("T#30").substr(32,30);
}
libraryexit(company_part2)

libraryinit(default_first_person_title)
//-------------------------------------
function main() {
	ANS = RECORD.a(3, 1);
	if (not ANS) {
		ANS = "The Marketing Director(Middle East)";
	}
	return ANS;
}
libraryexit(default_first_person_title)

libraryinit(details)
//------------------
function main() {
	var ans = calculate("COUNTRY");
	var temp = (calculate("RELATIONSHIP") ^ " " ^ calculate("CURRENTLY")).trim();
	if (temp) {
		if (ans) {
			ans ^= ", ";
		}
		ans ^= temp;
	}
	temp = calculate("CATEGORY");
	if (temp) {
		temp.lcaser();
		temp.swapper(VM, ", ");
		ans.r(1, -1, temp);
	}
	temp = calculate("PRODUCT_CATEGORIES");
	if (temp) {
		temp.lcaser();
		temp.swapper(VM, ", ");
		ans.r(1, -1, temp);
	}
	ANS = ans.oconv("T#40");
	ANS.converter(TM, VM);
	return ANS;
}
libraryexit(details)

libraryinit(dhl)
//--------------
function main() {
	var ans = "";
	ans ^= VM ^ ".pl33";
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ " (D.H.L. FORM)";
	ans ^= VM;
	ans ^= VM ^ " 119210011     [MAILREF]" ^ calculate("ADDRESS_REF");
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ " A.M.E.R.                        " ^ calculate("COMPANY");
	ans ^= VM;
	ans ^= VM ^ "                                 " ^ calculate("FIRST_PERSON");
	ans ^= VM ^ "                                 " ^ calculate("ADDRESS1");
	ans ^= VM ^ "                                 " ^ calculate("ADDRESS2");
	ans ^= VM ^ " P O BOX 3962                    " ^ calculate("ADDRESS3");
	ans ^= VM ^ " NICOSIA                         " ^ calculate("ADDRESS4");
	ans ^= VM ^ " CYPRUS                          " ^ calculate("ADDRESS5");
	ans ^= VM ^ "                                                     " ^ calculate("COUNTRY");
	ans ^= VM;

	ANS = ans;
	ANS.swapper(VM, "\r\n");
	ANS.splicer(1, 2, "");
	return ANS;
}
libraryexit(dhl)

libraryinit(first_person)
//-----------------------
function main() {
	return RECORD.a(2, 1);
}
libraryexit(first_person)

libraryinit(full_product_name2)
//-----------------------------
function main() {
	var prodcodes = calculate("PRODUCT_CODE2");
	var nprods = prodcodes.count(VM) + (prodcodes ne "");
	var fullprodnames = "";
	for (var prodn = 1; prodn <= nprods; ++prodn) {
		var prodcode = prodcodes.a(1, prodn);
		var fullproductname = "";
		var max = prodcode.count("*") + (prodcode ne "");
		for (var ii = 1; ii <= max; ++ii) {
			fullproductname ^= " - " ^ prodcode.field("*", 1, ii).xlate("PRODUCTS", 1, "C");
		};//ii;
		fullprodnames.r(1, prodn, fullproductname.substr(4,9999));
	};//prodn;
	return fullprodnames;
}
libraryexit(full_product_name2)

libraryinit(label)
//----------------
function main() {
	var ans = "";
	ans ^= VM ^ "" ^ calculate("FIRST_PERSON");
	ans ^= VM ^ "" ^ calculate("COMPANY_PART1");
	ans ^= VM ^ "" ^ calculate("COMPANY_PART2");
	ans ^= VM ^ "" ^ calculate("ADDRESS");
	ans ^= VM ^ "" ^ calculate("COUNTRY");

	ANS = ans;
	ANS.swapper(VM, "\r\n");
	ANS.splicer(1, 2, "");
	return ANS;
}
libraryexit(label)

libraryinit(ledger_code)
//----------------------
function main() {
	return ID.xlate("ADDRESSES", 2, "X");
}
libraryexit(ledger_code)

libraryinit(letter)
//-----------------
function main() {
	var ans = "";
	ans ^= VM ^ "" ^ calculate("FIRST_PERSON");
	ans ^= VM ^ "" ^ calculate("COMPANY");
	ans ^= VM ^ "" ^ calculate("ADDRESS");
	ans ^= VM ^ "" ^ calculate("COUNTRY");
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ "Dear " ^ calculate("FIRST_PERSON");
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ "RE:";
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ "   ... letter ...";
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ "          Yours faithfully,";

	ANS = ans;
	ANS.swapper(VM, "\r\n");
	ANS.splicer(1, 2, "");
	return ANS;
}
libraryexit(letter)

libraryinit(nameaddr)
//-------------------
function main() {
	var ans = calculate("COMPANY");
	var persons = calculate("PERSON");
	var npersons = persons.count(VM) + (persons ne "");
	//PERSONS=PERSONS:::STR(', ':@VM,NPERSONS):::{PERSONAL_COMMENTS}
	persons.swapper(", " ^ VM, VM);
	persons.splicer(-1, 1, "");
	if (persons) {
		ans.r(1, -1, persons);
	}
	if (calculate("ADDRESS")) {
		ans.r(1, -1, calculate("ADDRESS"));
		}
	//ANS<1,-1>=TRIM({TOWN}:' ':{POST})
	ANS = ans.oconv("T#40");
	ANS.converter(TM, VM);
	return ANS;
}
libraryexit(nameaddr)

libraryinit(numbers)
//------------------
function main() {
	var ans = "";
	if (calculate("TEL")) {
		ans = "Tel: " ^ calculate("TEL");
	}
	if (calculate("TLX")) {
		ans.r(1, -1, "Tlx: " ^ calculate("TLX"));
	}
	if (calculate("FAX")) {
		ans.r(1, -1, "Fax: " ^ calculate("FAX"));
		}
	ans.r(1, -1, "Ref: " ^ ID);
	ANS = ans.oconv("T#25");
	ANS.converter(TM, VM);
	return ANS;
}
libraryexit(numbers)

libraryinit(person_xref)
//----------------------
#include <xref.h>

function main() {
	call xref(calculate("PERSON"), "\x20\x22\x20\x5F\x56\x4D\x5F\x20\x5F\x53\x4D\x5F\x20\x5F\x54\x4D\x5F\x20\x22\x2E\x2C\x2D\x2F\x28\x29\x5C\x5C\x2B\x24\x25\x26\x2A\x2B\x3D\x7B\x7D\x5B\x3C\x3E\x3F\x3B\x3A\x7C\x5C\x22\x5C\x27", "", "1");
	return ANS;
}
libraryexit(person_xref)

libraryinit(person_and_comment)
//-----------------------------
function main() {
	var ans = "";
	var temp = calculate("COMMENTS");
	if (temp) {
		ans ^= temp;
	}
	ANS = ans.oconv("T#40");
	ANS.converter(TM, VM);
	return ANS;
}
libraryexit(person_and_comment)

libraryinit(product_name)
//-----------------------
function main() {
	return calculate("PRODUCT_CODE").xlate("PRODUCTS", 1, "X");
}
libraryexit(product_name)

libraryinit(product_name2)
//------------------------
function main() {
	return calculate("PRODUCT_CODE2").xlate("PRODUCTS", 1, "X");
}
libraryexit(product_name2)

libraryinit(reminder)
//-------------------
function main() {
	var ans = "";
	ans ^= VM ^ "" ^ calculate("ADDRESS");
	ans ^= VM;
	ans ^= VM ^ "Dear " ^ calculate("FIRST_PERSON");
	ans ^= VM;
	ans ^= VM ^ " This is to remind you the following policies are expiring within the";
	ans ^= VM ^ " near future.";
	ans ^= VM;
	ans ^= VM ^ "Yours sincerely,";
	ans ^= VM;
	ans ^= VM;
	ans ^= VM;
	ans ^= VM ^ "Pantelis Souglides.";

	ANS = ans;
	ANS.swapper(VM, "\r\n");
	ANS.splicer(1, 2, "");
	return ANS;
}
libraryexit(reminder)

libraryinit(stevelabel)
//---------------------
function main() {
	var ans = "";
	ans ^= VM ^ "" ^ calculate("DEFAULT_FIRST_PERSON_TITLE");
	ans ^= VM ^ "" ^ calculate("FIRST_PERSON");
	ans ^= VM ^ "" ^ calculate("COMPANY");
	ans ^= VM ^ "" ^ calculate("ADDRESS");
	ans ^= VM ^ "" ^ calculate("COUNTRY");

	ANS = ans;
	ANS.swapper(VM, "\r\n");
	ANS.splicer(1, 2, "");
	return ANS;
}
libraryexit(stevelabel)

libraryinit(underline)
//--------------------
function main() {
	printl(var("-").str(128));
	return ANS;
}
libraryexit(underline)
