#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("test_mvmv says 'Hello World!'");

	////////////
	//fieldstore
	////////////

	//multibyte sep fieldstore
	var greek5x4  = "αβγδεαβγδεαβγδεαβγδε";
	var greek5x2  = "αβγδεαβγδε";
	var greekstr2 = fieldstore(greek5x2, "β", 2, 1, "xxx");
	assert(greekstr2       eq "αβxxxβγδε");
	assert(greekstr2.len() eq 15);
	//on a temporary
	greekstr2 = var(greek5x2).fieldstore("β", 2, 1, "xxx");
	assert(greekstr2       eq "αβxxxβγδε");
	assert(greekstr2.len() eq 15);

	// First *byte* of a var may not be valid utf8
	assert(var(greek5x4.toChar()).oconv("HEX").outputl() eq "CE");

	assert(textseq("") eq "");

	//fieldstorer
	//in place - oo method
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 2, 1, "xxx");
	assert(greekstr2       eq "αβxxxβγδε");
	assert(greekstr2.len() eq 15);
	//in place - procedural
	greekstr2 = greek5x2;
	fieldstorer(greekstr2, "β", 2, 1, "xxx");
	assert(greekstr2       eq "αβxxxβγδε");
	assert(greekstr2.len() eq 15);
	//empty separator character
	try {
		printl(fieldstore(greekstr2, "", 2, 1, "xxx"));
		assert(false && "empty separator character in fieldstore() should generate an error");
	} catch (VarError&) {
	}
	try {
		printl(field(greekstr2, "", 1));
		assert(false && "empty separator character in field() should generate an error");
	} catch (VarError&) {
	}
	//fieldstore after end of string
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 5, 1, "xxx");
	assert(greekstr2       eq "αβγδεαβγδεββxxx");
	//negative field number from back
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", -2, 1, "xxx");
	assert(greekstr2       eq "αβxxxβγδε");
	assert(greekstr2.len() eq 15);
	//positive number of fields indicates to replace fieldwise with the fields in the insertion. empty fields if the number of fields in the insertion value is insufficient
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 2, 2, "xxx");
	assert(greekstr2       eq "αβxxxβ");
	assert(greekstr2.len() eq 9);
	//positive number of fields indicates to replace fieldwise with the fields in the insertion.
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 2, 2, "xxxβyyy");
	assert(greekstr2       eq "αβxxxβyyy");
	assert(greekstr2.len() eq 12);
	//negative number of fields indicates to replace that positive number of fields with whatever is the insertion.
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 2, -2, "xxx");
	assert(greekstr2       eq "αβxxx");
	assert(greekstr2.len() eq 7);
	//replacing an empty field with something
	greekstr2 = "αββγδε";
	greekstr2.fieldstorer("β", 2, 1, "xxx");
	assert(greekstr2       eq "αβxxxβγδε");
	//replacing a field with nothing
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 2, 1, "");
	assert(greekstr2       eq "αββγδε");
	//replacing an empty field with nothing
	greekstr2 = "αββγδε";
	greekstr2.fieldstorer("β", 2, 1, "");
	//replacing 0 fields
	greekstr2 = greek5x2;
	greekstr2.fieldstorer("β", 2, 0, "xxx");
	printl(greekstr2);
	//assert(greekstr2 eq "αβγδεαβγδε");

	//insert into empty string
	greekstr2 = "";
	greekstr2.fieldstorer("β", 1, 3, "xxx");
	printl(greekstr2);
	assert(greekstr2 eq "xxxββ");
	//insert into empty string
	greekstr2 = "";
	greekstr2.fieldstorer("β", 0, 0, "");
	printl(greekstr2);
	assert(greekstr2 eq "");
	//insert into empty string
	greekstr2 = "";
	greekstr2.fieldstorer("β", 3, 3, "xxx");
	printl(greekstr2);
	assert(greekstr2 eq "ββxxxββ");

	//temporary
	greekstr2 = fieldstore(greek5x2, "β", 2, 1, "1β2β3");
	assert(greekstr2 eq "αβ1βγδε");

	greekstr2 = fieldstore(greek5x2, "β", 2, -1, "1β2β3");
	assert(greekstr2 eq "αβ1β2β3βγδε");

	//var greek5x4="αβγδεαβγδεαβγδεαβγδε";
	assert(fieldstore(greek5x4, "β", 2, 3, "ζ")  eq "αβζβββγδε");
	assert(fieldstore(greek5x4, "β", 2, -3, "ζ") eq "αβζβγδε");

	//more fieldstorer
	{
		var x = "a b c d";
		fieldstorer(x, " ", 2, 0, "yyy");
		printl(x);
		assert(x eq "a yyy b c d");

		x = "";
		fieldstorer(x, "|", 2, 0, "yyy");
		printl(x);
		assert(x eq "|yyy");

		x = "";
		fieldstorer(x, "β", 2, 2, "y");
		printl(x);
		assert(x eq "βyβ");

		x = "";
		fieldstorer(x, "β", 3, 1, "y");
		printl(x);
		assert(x eq "ββy");

		x = "";
		fieldstorer(x, "β", 3, 2, "y");
		printl(x);
		assert(x eq "ββyβ");

		x = "";
		fieldstorer(x, "β", 3, 2, "yyy");
		printl(x);
		assert(x eq "ββyyyβ");

		x = "";
		fieldstorer(x, "β", 3, 3, "yyy");
		printl(x);
		assert(x eq "ββyyyββ");

		x = "";
		fieldstorer(x, "β", 3, 4, "yyy");
		printl(x);
		assert(x eq "ββyyyβββ");
	}

	////////
	//locate
	////////

	var vn;

	// BY cannot be XX
	try {
		var x = "xyz"_var.locateby("XX", "xyz", MV);
		assert(false);
		//std::unreachable()
	} catch (VarError&) {
	}

	assert(var("f100" _FM "f210" _VM "" _VM "f230").locateby("AL", "", vn, 2));
	assert(vn eq 2);

	assert(var("f100" _FM "f210" _VM "" _VM "f230").locateby("AL", "", vn, 2, 2));
	assert(vn eq 1);

	assert(var("").locateby("AL", "", vn));
	assert(vn eq 1);

	//no orderby
	assert(var("XXX").locate("XXX"));
	assert(var("AAA" _VM "XXX").locate("XXX"));
	assert(var("AA" _VM "BB" _FM "22").locate("BB", vn, 1));
	assert(!var("AA" _FM "BB").locate("BB", vn, 1));
	assert(var("AA" _FM "BB1" _VM "BB2").locate("BB2", vn, 2));
	assert(vn eq 2);
	//
	assert(var("AA" _FM "f2v1" _VM "f2v2").locate("f2v2", vn, 2));
	assert(vn eq 2);
	assert(!var("AA" _FM "f2v1" _VM "f2v2" _SM "X").locate("f2v2", vn, 2));
	//assert (vn=2);
	assert(var("AA" _FM "f2v1" _VM "f2v2" _VM "X").locate("f2v2", vn, 2));
	assert(vn eq 2);
	assert(var("AA" _FM "f2v1" _VM "f2v2" _FM).locate("f2v2", vn, 2));
	assert(vn eq 2);
	// search for a subvalue
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2v2s2").locate("f2v2s2", vn, 2, 2));
	assert(vn eq 2);
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2v2s2" _SM "X").locate("f2v2s2", vn, 2, 2));
	assert(vn eq 2);
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2v2s2" _VM "X").locate("f2v2s2", vn, 2, 2));
	assert(vn eq 2);
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2v2s2" _FM "X").locate("f2v2s2", vn, 2, 2));
	assert(vn eq 2);
	//search in a subvalue using ","
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2").locateusing(",", "s2", vn, 2, 2, 2));
	// free function
	assert(locateusing(",", "s2", var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2"), vn, 2, 2, 2));
	assert(vn eq 3);
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2").locateusing(",", "s2", vn, 2, 2, -2));
	assert(vn eq 1);
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _SM "X").locateusing(",", "s2", vn, 2, 2, 2));
	assert(vn eq 3);
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _VM "X").locateusing(",", "s2", vn, 2, 2, 2));
	assert(vn eq 3);
	assert(var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _FM "X").locateusing(",", "s2", vn, 2, 2, 2));
	assert(vn eq 3);
	//search in a subvalue using "," where subvalue doesnt exist
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2").locateusing(",", "s2", vn, 2, 2, 3));
	assert(vn eq 1);
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2").locateusing(",", "s2", vn, 2, 2, -3));
	assert(vn eq 1);
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _SM "X").locateusing(",", "s2", vn, 2, 2, 3));
	assert(vn eq 2);
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _VM "X").locateusing(",", "s2", vn, 2, 2, 3));
	assert(vn eq 1);
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _FM "X").locateusing(",", "s2", vn, 2, 2, 3));
	assert(vn eq 1);
	//check SV search doesnt cross into next multivalue or field
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _VM _SM "X").locateusing(",", "s2", vn, 2, 2, 3));
	assert(vn eq 1);
	assert(!var("AA" _FM "f2v1" _VM "f2v2s1" _SM "f2,v2,s2" _FM _SM "X").locateusing(",", "s2", vn, 2, 2, 3));
	assert(vn eq 1);

	//check field number zero means search using FM
	var fn;
	assert(var("aa" _FM "bb" _FM "cc").locate("bb", fn, 0));
	assert(fn eq 2);

	var lbvn;
	//no fieldno/value given means using character VM
	assert(var("1" _VM "10" _VM "2" _VM "B").locateby("AL", "A", lbvn) || lbvn       eq 4);
	//fieldno given means search in that field using character VM
	assert(var("1" _VM "10" _VM "2" _VM "B").locateby("AL", "A", lbvn, 1) || lbvn    eq 4);
	//fieldno given and =0 means search whole string using character FM
	assert(var("1" _FM "10" _FM "2" _FM "B").locateby("AL", "A", lbvn, 0) || lbvn    eq 4);
	// free function
	assert(locateby("AL", "A", var("1" _FM "10" _FM "2" _FM "B"), lbvn, 0) || lbvn   eq 4);
	var order2 = "AL";
	assert(locateby(order2, "A", var("1" _FM "10" _FM "2" _FM "B"), lbvn, 0) || lbvn eq 4);

	//search values within a field

	assert(!var("").locate("X", MV, 2));
	assert(!var(_FM).locate("X", MV, 2));
	assert(!var(_FM _FM).locate("X", MV, 2));
	assert(var(_FM _FM).locate("", MV, 2));
	assert(!var(_FM "f2v1" _VM "f2v2" _FM).locate("X", MV, 2));
	assert(!var(_FM "f2v1" _VM "f2v2").locate("X", MV, 2));

	//search subvalues within a value within a field

	assert(!var("").locate("X", MV, 2, 2));
	assert(!var(_FM).locate("X", MV, 2, 2));
	assert(!var(_FM _VM).locate("X", MV, 2, 2));
	assert(!var(_FM _VM _VM).locate("X", MV, 2, 2));
	assert(var(_FM _VM _VM).locate("", MV, 2, 2));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _VM).locate("X", MV, 2, 2));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2").locate("X", MV, 2, 2));

	assert(!var("").locate("X", MV, 2, 3));
	assert(!var(_FM _FM).locate("X", MV, 2, 3));
	assert(!var(_FM _VM _FM).locate("X", MV, 2, 3));
	assert(!var(_FM _VM _VM _FM).locate("X", MV, 2, 3));
	assert(var(_FM _VM _VM _FM).locate("", MV, 2, 3));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _VM _FM).locate("X", MV, 2, 3));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _FM).locate("X", MV, 2, 3));

	//check searching for VMs does not stray over into the next field
	assert(!var("").locate("X", MV, 2, 3));
	assert(!var(_FM _FM _VM).locate("X", MV, 2, 3));
	assert(!var(_FM _VM _FM _VM).locate("X", MV, 2, 3));
	assert(!var(_FM _VM _VM _FM _VM).locate("X", MV, 2, 3));
	assert(var(_FM _VM _VM _FM _VM).locate("", MV, 2, 3));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _VM _FM _VM).locate("X", MV, 2, 3));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _FM _VM).locate("X", MV, 2, 3));

	//search subvalues within a value within a field (using a specfic separator)

	//check searching for SMs does not stray over into the next value
	assert(!var("").locateusing(",", "X", MV, 2, 3, -3));
	assert(!var("").locateusing(",", "X", MV, 2, 3, 3));
	assert(!var(_FM _FM _VM).locateusing(",", "X", MV, 2, 2, 2));
	assert(!var(_FM _VM _FM _VM).locateusing(",", "X", MV, 2, 2, 2));
	assert(!var(_FM _VM _VM _FM _VM).locateusing(",", "X", MV, 2, 2, 2));
	assert(var(_FM _VM _VM _FM _VM).locateusing(",", "", MV, 2, 2, 2));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _VM _FM _VM).locateusing(",", "X", MV, 2, 2, 2));
	assert(!var(_FM _VM "f2v2s1" _SM "f2v2s2" _FM _VM).locateusing(",", "X", MV, 2, 2, 2));

	//locate using multibyte unicode char separator and target
	//var greek5x4="αβγδεαβγδεαβγδεαβγδε";
	var setting;
	assert(greek5x4.locateusing("β", "γδεα", setting));
	assert(setting eq 2);

	var sort = "aa" _FM "bb" _FM "dd";
	var sortn;
	assert(!sort.locatebyusing(var("AL"), FM, "a", sortn));
	assert(sortn eq 1);
	assert(!sort.locatebyusing(var("AL"), FM, "b", sortn));
	assert(sortn eq 2);
	assert(!sort.locatebyusing("AL", FM, "a", sortn));
	assert(sortn eq 1);
	assert(sort.locatebyusing("AL", FM, "bb", sortn));
	assert(sortn eq 2);
	assert(!sort.locatebyusing("AL", FM, "cc", sortn));
	assert(sortn eq 3);
	assert(!sort.locatebyusing("AL", FM, "ee", sortn));
	assert(sortn eq 4);
	//usingchar is cstr
	assert(sort.locatebyusing("AL", _FM, "bb", sortn));
	assert(sortn eq 2);

	assert(sort.locateusing(FM, "aa", sortn));
	assert(sortn eq 1);
	assert(sort.locateusing(FM, "bb", sortn));
	assert(sortn eq 2);
	assert(!sort.locateusing(FM, "cc", sortn));
	assert(sortn eq 4);
	assert(!sort.locateusing(FM, "ee", sortn));
	assert(sortn eq 4);

	var locii;
	var ascints = "1 2 3 10 20 30 100 200 300";
	var revints = "300 200 100 30 20 10 3 2 1";
	converter(ascints, " ", VM);
	converter(revints, " ", VM);
	var ar	   = "AR";
	var locsep = ",";

	//negative field number or value number finds nothing
	assert(!ascints.locate(10, MV, -1));
	assert(!ascints.convert(VM, SM).locate(10, MV, 1, -1));

	assert(ascints.locate(10) eq 4);
	assert(ascints.locate(10));

	assert(ascints.locate(9) eq 0);
	assert(!ascints.locate(9));

	assert(ascints.locate(30, locii));
	assert(locii eq 6);

	assert(!ascints.locate(31, locii));
	assert(locii eq 10);

	assert(ascints.locate(30, locii, 1));
	assert(locii eq 6);

	assert(!ascints.locate(31, locii, 1));
	assert(locii eq 10);

	assert(!ascints.locate(31, locii, 2));
	assert(locii eq 1);

	assert(!ascints.locateby(ar, 21, locii));
	assert(locii eq 6);

	assert(!ascints.locateby(ar, 299, locii));
	assert(locii eq 9);

	assert(ascints.locateby(ar, 300, locii));
	assert(locii eq 9);

	assert(!revints.locateby("DL", 0, locii));
	assert(locii eq 10);

	assert(revints.locateby("DL", 1, locii));
	assert(locii eq 9);

	assert(!revints.locateby("DL", 1.1, locii));
	assert(locii eq 9);

	assert(!revints.locateby("DR", 0, locii));
	assert(locii eq 10);

	assert(revints.locateby("DR", 1, locii));
	assert(locii eq 9);

	assert(!revints.locateby("DR", 301, locii));
	assert(locii eq 1);

	assert(revints.locateby("DR", 300, locii));
	assert(locii eq 1);

	assert(!revints.locateby("DR", 299, locii));
	assert(locii eq 2);

	var revints2 = FM ^ revints ^ FM ^ FM ^ VM;

	assert(!revints2.locateby("DR", 0, locii, 2));
	assert(locii eq 10);

	assert(revints2.locateby("DR", 1, locii, 2));
	assert(locii eq 9);

	assert(!revints2.locateby("DR", 301, locii, 2));
	assert(locii eq 1);

	assert(revints2.locateby("DR", 300, locii, 2));
	assert(locii eq 1);

	assert(!revints2.locateby("DR", 299, locii, 2));
	assert(locii eq 2);

	var order = "AR";
	assert(ascints.convert(VM, SM).locateby(order, 20, locii, 1, 1));
	assert(locii eq 5);
	assert(ascints.convert(VM, SM).locateby("AR", 20, locii, 1, 1));
	assert(locii eq 5);

	assert(ascints.locateby("AR", 20, locii));
	assert(locii eq 5);

	assert(ascints.locateby("AL", 20, locii));
	assert(locii eq 5);

	assert(revints.locateby("DR", 20, locii));
	assert(locii eq 5);

	assert(revints.locateby("DL", 20, locii));
	assert(locii eq 5);

	assert(!ascints.locateby("AR", 21, locii));
	assert(locii eq 6);

	assert(!ascints.locateby("AL", 21, locii));
	assert(locii eq 3);

	assert(!revints.locateby("DR", 21, locii));
	assert(locii eq 5);

	//?
	assert(!revints.locateby("DL", 21, locii));
	TRACE(locii)
	assert(locii eq 2);

	assert(!ascints.locateby("AR", 321, locii));
	assert(locii eq 10);

	assert(!ascints.locateby("AL", 321, locii));
	assert(locii eq 10);

	assert(!revints.locateby("DR", 321, locii));
	assert(locii eq 1);

	assert(!revints.locateby("DL", 321, locii));
	assert(locii eq 1);

	converter(ascints, VM, ",");
	converter(revints, VM, ",");

	assert(ascints.locateusing(",", 30));
	assert(!ascints.locateusing(",", 31));

	assert(ascints.locateusing(",", 30, locii));
	assert(locii eq 6);

	assert(ascints.locateusing(locsep, 30, locii));
	assert(locii eq 6);

	converter(ascints, ",", VM);
	converter(revints, ",", VM);

	assert(locate(10, ascints) eq 4);

	assert(locate(30, ascints, locii));
	assert(locii eq 6);

	assert(not locate(31, ascints, locii));
	assert(locii eq 10);

	assert(locate(30, ascints, locii, 1));
	assert(locii eq 6);

	assert(not locate(31, ascints, locii, 1));
	assert(locii eq 10);

	assert(not locate(31, ascints, locii, 2));
	assert(locii eq 1);

	assert(not locateby(ar, 21, ascints, locii));
	assert(locii eq 6);

	assert(not locateby("AR", 21, ascints, locii));
	assert(locii eq 6);

	assert(not locateby("AL", 21, ascints, locii));
	assert(locii eq 3);

	assert(not locateby("DR", 21, revints, locii));
	assert(locii eq 5);

	assert(not locateby("DL", 21, revints, locii));
	assert(locii eq 2);

	converter(ascints, VM, ",");
	converter(revints, VM, ",");

	assert(locateusing(",", 30, ascints) eq 1);
	assert(locateusing(",", 31, ascints) eq 0);

	assert(locateusing(",", 30, ascints, locii));
	assert(locii eq 6);

	assert(locateusing(locsep, 30, ascints, locii));
	assert(locii eq 6);

	{

		// Check simple locate works with ANY of the field mark chars

		// Without setting
		//////////////////

		// Return field no if found
		assert(locate("X", "A^X^C"_var) == 2);
		assert("A^X^C"_var.locate("X") == 2);

		// Return 0 if not found
		assert(locate("X", "A^Q^C"_var) == 0);
		assert("A^Q^C"_var.locate("X") == 0);

		// With setting
		///////////////
		{
			var setting;
			// Return true if found
			assert(locate("X", "A|X|C"_var, setting) == 1);
			// and field no in setting
			assert(setting == 2);

			// Return false if not found
			assert(locate("X", "A|Q|C"_var, setting) == 0);
			// and returns next field no in setting
			assert(setting == 4);
		}

		{
			var setting;
			// Return true if found
			assert("A|X|C"_var.locate("X", setting) == 1);
			// and field no in setting
			assert(setting == 2);

			// Return false if not found
			assert("A|Q|C"_var.locate("X", setting) == 0);
			// and returns next field no in setting
			assert(setting == 4);
		}

		TRACE(_VISIBLE_FMS)
		TRACE(locate("abc~QQ`def"_var, "QQ"))

		{
			// Poor or undefined behaviour when target has any of the field mark characters

			assert(locate("^QQ^", "abc^QQ^def"_var) == 0); // the character BEFORE ^QQ^ is not a field mark so it is not "found"

			// Kind of works if target doesnt start or end with any of the field mark chars
			assert(locate( "11]22]33"_var, "abc^11]22]33^QQ^def"_var) == 2);
			// But it doesnt understand field mark heirarchy
			assert(locate( "11^22^33"_var, "abc]11^22^33]QQ]def"_var) == 2);
		}

		// Find in any mixture of field marks
		assert(locate("QQ", "abc|QQ^def"_var) == 2);
		assert(locate("QQ", "abc}QQ]def"_var) == 2);
		assert(locate("QQ", "abc]QQ}def"_var) == 2);
		assert(locate("QQ", "abc^QQ|def"_var) == 2);
		assert(locate("QQ", "abc`QQ~def"_var) == 2);

		assert(locate("abc", "abc`QQ`def"_var) == 1);
		assert(locate("abc", "abc^QQ^def"_var) == 1);
		assert(locate("abc", "abc]QQ]def"_var) == 1);
		assert(locate("abc", "abc}QQ}def"_var) == 1);
		assert(locate("abc", "abc|QQ|def"_var) == 1);
		assert(locate("abc", "abc~QQ~def"_var) == 1);

		assert(locate("QQ", "abc`QQ`def"_var) == 2);
		assert(locate("QQ", "abc^QQ^def"_var) == 2);
		assert(locate("QQ", "abc]QQ]def"_var) == 2);
		assert(locate("QQ", "abc}QQ}def"_var) == 2);
		assert(locate("QQ", "abc|QQ|def"_var) == 2);
		assert(locate("QQ", "abc~QQ~def"_var) == 2);

		assert(locate("def", "abc`QQ`def"_var) == 3);
		assert(locate("def", "abc^QQ^def"_var) == 3);
		assert(locate("def", "abc]QQ]def"_var) == 3);
		assert(locate("def", "abc}QQ}def"_var) == 3);
		assert(locate("def", "abc|QQ|def"_var) == 3);
		assert(locate("def", "abc~QQ~def"_var) == 3);

		// Find at the beginning, middle  or end
		assert(! locate("a", "abc]QQ]def"_var));
		assert(! locate("ab", "abc]QQ]def"_var));
		assert(! locate("b", "abc]QQ]def"_var));
		assert(! locate("bc", "abc]QQ]def"_var));
		assert(! locate("Q", "abc]QQ]def"_var));
		assert(! locate("QQQ", "abc]QQ]def"_var));
		assert(! locate("d", "abc]QQ]def"_var));
		assert(! locate("e", "abc]QQ]def"_var));
		assert(! locate("f", "abc]QQ]def"_var));
		assert(! locate("deff", "abc]QQ]def"_var));

		// Find at the beginning, middle  or end
		assert(locate("abc", "abc]QQ]def"_var));

		assert(locate("abc", "abc]QQ]def"_var) == 1);
		assert(locate("QQ", "abc]QQ]def"_var) == 2);
		assert(locate("def", "abc]QQ]def"_var) == 3);

		// Dont find at the beginning or end
		assert(! locate("ab", "abc]QQ]def"_var));
		assert(! locate("Q", "abc]QQ]def"_var));
		assert(! locate("de", "abc]QQ]def"_var));

		assert(! locate("", ""_var));
		assert(! locate("", "abc]QQ]def"_var));
		assert(! locate("000", ""_var));

		// Missing beginning, middle and end
		assert(! locate("Q", "]QQ]def"_var));
		assert(! locate("ef", "abc]]def"_var));
		assert(! locate("bc", "abc]QQ]"_var));

		// Missing beginning, middle and end
		assert(locate("QQ", "]QQ]def"_var) == 2);
		assert(locate("def", "abc]]def"_var) == 3);
		assert(locate("QQ", "abc]QQ]"_var) == 2);

		assert(locate("QQ", "abc]QQ]"_var) == 2);

	}

	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

//function to conveniently test sum function
function test_sum(in instr) {
	return sum(instr.convert("~^]}>|", _RM _FM _VM _SM _TM _ST)).convert(_RM _FM _VM _SM _TM _ST, "~^]}>|");
}

programexit()
