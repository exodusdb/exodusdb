#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

	function main() {

	/////////
	//replace
	/////////

	//.r() is field/value/subvalue in place like x<1,2.3>='abc'
	// but we use pickreplace which returns a temporary for convenience in testing
	// note that replace() function in exodus is regular expression replace a with b

	var rec = "1" _FM "21" _VM "22" _FM "311" _SM "312" _VM "32" _FM "4";

	{
		printl("Check that () extraction on a constant var produces an ordinary var that can use .oconv(...) etc");
		let rec2 = rec;
		assert(rec2(2, 2).convert("2", "Q") eq "QQ");

		var rec3 = rec;
		// will not compile since () extraction on non-cost var produces a proxy, not a proper var
		//assert(rec3(2,2).convert("2","Q") eq "QQ");
		//
		// This WILL compile and work
		assert(convert(rec3(2, 2) from "2" to "Q") eq "QQ");
	}

	{
		assert(pickreplace(rec, 1, "1r") eq "1r" _FM "21" _VM "22" _FM "311" _SM "312" _VM "32" _FM "4");
		printl(pickreplace(rec, 2, "2r"));	//1^2r^311}312]32^4
		assert(pickreplace(rec, 2, "2r") eq "1" _FM "2r" _FM "311" _SM "312" _VM "32" _FM "4");

		assert(pickreplace(rec, -1, "5-1r")        eq "1" _FM "21" _VM "22" _FM "311" _SM "312" _VM "32" _FM "4" _FM "5-1r");
		assert(pickreplace(rec, 2, -1, "2-1r")     eq "1" _FM "21" _VM "22" _VM "2-1r" _FM "311" _SM "312" _VM "32" _FM "4");
		assert(pickreplace(rec, 3, 2, -1, "32-1r") eq "1" _FM "21" _VM "22" _FM "311" _SM "312" _VM "32" _SM "32-1r" _FM "4");

		assert(pickreplace(rec, 2, 1, "21r") eq "1" _FM "21r" _VM "22" _FM "311" _SM "312" _VM "32" _FM "4");
		assert(pickreplace(rec, 2, 4, "24r") eq "1" _FM "21" _VM "22" _VM _VM "24r" _FM "311" _SM "312" _VM "32" _FM "4");
	}

	//.r()

	//plain replace subvalue in the middle of subvalues
	var v123 = "f100" _FM "f210" _VM "f221" _SM "f222" _SM;
	assert(v123.r(2, 2, 2, "n222")                                               eq "f100" _FM "f210" _VM "f221" _SM "n222" _SM);
	v123(2, 2, 2) = "n222";
	assert(v123                                                                  eq "f100" _FM "f210" _VM "f221" _SM "n222" _SM);
	assert(var("f100" _FM "f210" _VM "f221" _SM "f222" _SM).r(2, 2, 2, "n222")   eq "f100" _FM "f210" _VM "f221" _SM "n222" _SM);
	//plain replace subvalue in the middle of subvalues with null, smaller and larger
	assert(var("f100" _FM "f210" _VM "f221" _SM "f222" _SM).r(2, 2, 2, "")       eq "f100" _FM "f210" _VM "f221" _SM "" _SM);
	assert(var("f100" _FM "f210" _VM "f221" _SM "f222" _SM).r(2, 2, 2, "x")      eq "f100" _FM "f210" _VM "f221" _SM "x" _SM);
	assert(var("f100" _FM "f210" _VM "f221" _SM "f222" _SM).r(2, 2, 2, "xyz123") eq "f100" _FM "f210" _VM "f221" _SM "xyz123" _SM);

	assert(var("f100" _FM "f210" _VM "f221" _SM "f222" _SM).r(2, "xyz123") eq "f100" _FM "xyz123");

	//replace field zero
	assert(pickreplace(rec, 0, "f000")       eq "f000");
	assert(pickreplace(rec, 0, 0, "f000")    eq "f000");
	assert(pickreplace(rec, 0, 0, 0, "f000") eq "f000");

	var da2 = "aa" _FM "b1" _VM "b2" _SM "b22" _FM "cc";

	//this shouldnt compile without a warning since it has no effect
	//pickreplace(da2,3,"x");//or this
	//pickreplace(da2,3,3,"x");//or this
	//pickreplace(da2,3,3,3,"x");//or this

	//replacement
	//da2(2)="x";//this will not compile because da2(2) is a temporary;

	da2 = "f1" _FM "f2" _FM "f3";

	//replace field 2 with "R2"
	da2 = "";
	assert(pickreplacer(da2, 2, "R2") eq(_FM "R2"));

	//replace field 2, value 3 with "R22"
	da2 = "";
	assert(pickreplacer(da2, 2, 3, "R23") eq(_FM _VM _VM "R23"));

	//replace field 2, value 3, subvalue 4 with "R234"
	da2 = "";
	assert(pickreplacer(da2, 2, 3, 4, "R234") eq(_FM _VM _VM _SM _SM _SM "R234"));

	/////////
	//extract
	/////////

	//extraction
	var da1 = "aa" _FM "b1" _VM "b2" _SM "b22" _FM "cc";
	assert(da1.f(2) eq extract(da1, 2));			  //this extracts field 2
	assert(da1.f(2, 2) eq extract(da1, 2, 2));		  //this extracts field 2, value 2
	assert(da1.f(2, 2, 2) eq extract(da1, 2, 2, 2));  //this extracts field 2, value 2, subvalue 2

	//extract or a() or plain ()
	var xx = "11" _SM "111" _VM "12" _FM "22" _FM "31" _VM "321" _SM "322" _FM;
	assert(extract(xx, 3, 2, 2)                                    eq "322");
	assert(xx.extract(3, 2, 2)                                     eq "322");
	//extract beyond end of string
	assert(extract(var("f100" _FM "f200"), 3)                      eq "");
	assert(extract(var("f100" _FM "f210" _VM), 2, 3)               eq "");
	assert(extract(var("f100" _FM "f210" _VM "f221" _SM), 2, 2, 2) eq "");
	assert(extract(var("f100" _FM "f200"), 3)                      eq "");
	assert(extract(var("f100" _FM "f210" _VM), 2, 3)               eq "");
	assert(extract(var("f100" _FM "f211" _SM), 2, 2, 2)            eq "");

	//plain extract subvalue from middle of subvalues
	assert(extract(var("100" _FM "210" _VM "221" _SM "222" _SM), 2, 2, 2) eq "222");

	//another way
	assert(xx.f(3, 2, 2) eq "322");
	//yet another way
	assert(xx.f(3, 2, 2) eq "322");

	//strange requests
	assert(xx.f(-1)       eq "");
	assert(xx.f(-2)       eq "");
	assert(xx.f(1, -1)    eq "");
	assert(xx.f(1, 1, -1) eq "");
	assert(xx.f(0, 0, 0)  eq xx);
	assert(xx.f(0, 2, 0)  eq "12");
	assert(xx.f(0, 0, 2)  eq "111");

	//missing fields
	assert(xx.f(4, 0, 0) eq "");
	assert(xx.f(1, 4, 0) eq "");
	assert(xx.f(1, 1, 4) eq "");
	assert(xx.f(4, 1, 0) eq "");
	assert(xx.f(4, 1, 0) eq "");
	assert(xx.f(4, 1, 2) eq "");

	///////
	//field
	///////

	//arev/pick convention is that field numbers and field counts less than 1 are treated as 1
	//this is probably not a logical or even useful convention
	assert(field("aa bb cc", " ", 0) eq "aa");	//arev
	assert(field("aa bb cc", " ", -1)    eq "aa");
	assert(field("aa bb cc", " ", -2)    eq "aa");
	assert(field("aa bb cc", " ", -3)    eq "aa");
	assert(field("aa bb cc", " ", -4) eq "aa");	 //?
	assert(field("aa bb cc", " ", 1)     eq "aa");
	assert(field("aa bb cc", " ", 2)     eq "bb");
	assert(field("aa bb cc", " ", 3)     eq "cc");
	assert(field("aa bb cc", " ", 4)     eq "");
	//
	assert(field("aa bb cc", " ", 0, 2)  eq "aa bb");
	assert(field("aa bb cc", " ", -1, 2) eq "aa bb");
	assert(field("aa bb cc", " ", -2, 2) eq "aa bb");
	assert(field("aa bb cc", " ", -3, 2) eq "aa bb");
	assert(field("aa bb cc", " ", -4, 2) eq "aa bb");
	assert(field("aa bb cc", " ", 1, 2)  eq "aa bb");
	assert(field("aa bb cc", " ", 2, 2)  eq "bb cc");
	assert(field("aa bb cc", " ", 3, 2)  eq "cc");
	assert(field("aa bb cc", " ", 4, 2)  eq "");

	//field2() is SB invention being very useful to extract the last field of a string
	//it should perhaps also work like [x,y]/substr character extraction rules
	//instead of treating any references before the first field as field 1
	//and negative field counts should work backwards perhaps
	assert(field2("aa bb cc", " ", 0) eq "aa");	 //arev but perhaps should return whole string
	assert(field2("aa bb cc", " ", -1) eq "cc");
	assert(field2("aa bb cc", " ", -2) eq "bb");
	assert(field2("aa bb cc", " ", -3) eq "aa");
	assert(field2("aa bb cc", " ", -4) eq "aa");  //?
	assert(field2("aa bb cc", " ", 1)  eq "aa");
	assert(field2("aa bb cc", " ", 2)  eq "bb");
	assert(field2("aa bb cc", " ", 3)  eq "cc");
	assert(field2("aa bb cc", " ", 4)  eq "");

	assert(field2("aa bb cc", " ", 0, 2) eq "aa bb");  //aren
	assert(field2("aa bb cc", " ", -1, 2) eq "cc");
	assert(field2("aa bb cc", " ", -2, 2) eq "bb cc");
	assert(field2("aa bb cc", " ", -3, 2) eq "aa bb");
	assert(field2("aa bb cc", " ", -4, 2) eq "aa bb");	//?
	assert(field2("aa bb cc", " ", 1, 2)  eq "aa bb");
	assert(field2("aa bb cc", " ", 2, 2)  eq "bb cc");
	assert(field2("aa bb cc", " ", 3, 2)  eq "cc");
	assert(field2("aa bb cc", " ", 4, 2)  eq "");

	//////////////////////////////////////////////////////////////
	//var& var::substr(const int startindex1, const int length) &&
	//////////////////////////////////////////////////////////////

	//negative length reverses the characters

	var ss = "abcd";
	assert(ss.b(2, 2) eq "bc");

	assert(substr(var(""), 0)    eq "");
	assert(substr(var(""), 0, 0) eq "");
	assert(substr(var(""), 0, 0) eq "");
	assert(substr(var(""), -2)   eq "");
	assert(substr(var(""), -1)   eq "");
	assert(substr(var(""), 0)    eq "");
	assert(substr(var(""), 1)    eq "");
	assert(substr(var(""), 2)    eq "");

	//single index extracts up to the end the string. 2nd parameter defaults to length of string
	assert(substr(var("ab"), -3) eq "ab");	//start before beginning
	assert(substr(var("ab"), -2) eq "ab");	//last but one char
	assert(substr(var("ab"), -1) eq "b");	//last char
	assert(substr(var("ab"), 0) eq "ab");	//start at 0 = start at 1
	assert(substr(var("ab"), 1) eq "ab");	//from 1st
	assert(substr(var("ab"), 2) eq "b");	//from 2nd
	assert(substr(var("ab"), 3) eq "");		//starting after the end of the string
	//
	//backwards 3 bytes from (including) starting character
	assert(substr(var("ab"), -3, -3) eq "");	//start before beginning
	assert(substr(var("ab"), -2, -3) eq "a");	//last but one char
	assert(substr(var("ab"), -1, -3) eq "ba");	//last char
	//assert(substr(var("ab"),0,-3) eq "a");//start at 0 = start at 1
	assert(substr(var("ab"), 0, -3) eq "");	   //start at 0 = start before beginning if negative length
	assert(substr(var("ab"), 1, -3) eq "a");   //from 1st
	assert(substr(var("ab"), 2, -3) eq "ba");  //from 2nd
	assert(substr(var("ab"), 3, -3) eq "ba");  //from one after the end of the string
	//
	//backwards 2 bytes from (including) starting character
	assert(substr(var("ab"), -3, -2) eq "");	//start before beginning
	assert(substr(var("ab"), -2, -2) eq "a");	//last but one char
	assert(substr(var("ab"), -1, -2) eq "ba");	//last char
	//assert(substr(var("ab"),0,-2) eq "a");//start at 0 = start at
	assert(substr(var("ab"), 0, -2) eq "");	   //start at 0 = start at
	assert(substr(var("ab"), 1, -2) eq "a");   //from 1st
	assert(substr(var("ab"), 2, -2) eq "ba");  //from 2nd
	//
	//backwards 2 bytes from (excluding) starting character
	assert(substr(var("ab"), -3, -1) eq "");   //start before beginning
	assert(substr(var("ab"), -2, -1) eq "a");  //last but one char ??
	assert(substr(var("ab"), -1, -1) eq "b");  //last char
	//assert(substr(var("ab"),0,-1) eq "a");//start at 0 = start at
	assert(substr(var("ab"), 0, -1) eq "");	  //start at 0 = start at
	assert(substr(var("ab"), 1, -1) eq "a");  //from 1st
	assert(substr(var("ab"), 2, -1) eq "b");  //from 2nd
	assert(substr(var("ab"), 3, -1) eq "b");  //from one after the end of the string ??
	//
	//length 0 bytes
	assert(substr(var("ab"), -3, 0) eq "");	 //start before beginning
	assert(substr(var("ab"), -2, 0) eq "");	 //last but one char ??
	assert(substr(var("ab"), -1, 0) eq "");	 //last char
	assert(substr(var("ab"), 0, 0) eq "");	 //start at 0 = start at
	assert(substr(var("ab"), 1, 0) eq "");	 //from 1st
	assert(substr(var("ab"), 2, 0) eq "");	 //from 2nd
	assert(substr(var("ab"), 3, 0) eq "");	 //from one after the end of the string ??
	//
	//length 1 bytes
	assert(substr(var("ab"), -3, 1) eq "a");  //start before beginning ??
	assert(substr(var("ab"), -2, 1) eq "a");  //last but one char ??
	assert(substr(var("ab"), -1, 1) eq "b");  //last char
	assert(substr(var("ab"), 0, 1) eq "a");	  //start at 0 = start at
	assert(substr(var("ab"), 1, 1) eq "a");	  //from 1st
	assert(substr(var("ab"), 2, 1) eq "b");	  //from 2nd
	assert(substr(var("ab"), 3, 1) eq "");	  //from one after the end of the string ??
	//
	//length 2 bytes
	assert(substr(var("ab"), -3, 2) eq "ab");  //start before beginning
	assert(substr(var("ab"), -2, 2) eq "ab");  //last but one char
	assert(substr(var("ab"), -1, 2) eq "b");   //last char
	assert(substr(var("ab"), 0, 2) eq "ab");   //start at 0 = start at
	assert(substr(var("ab"), 1, 2) eq "ab");   //from 1st
	assert(substr(var("ab"), 2, 2) eq "b");	   //from 2nd
	assert(substr(var("ab"), 3, 2) eq "");	   //from one after the end of the string ??
	//
	//length 3 bytes (same as length 2 bytes since the source string is only 2 bytes)
	assert(substr(var("ab"), -3, 3) eq "ab");  //start before beginning
	assert(substr(var("ab"), -2, 3) eq "ab");  //last but one char
	assert(substr(var("ab"), -1, 3) eq "b");   //last char
	assert(substr(var("ab"), 0, 3) eq "ab");   //start at 0 = start at
	assert(substr(var("ab"), 1, 3) eq "ab");   //from 1st
	assert(substr(var("ab"), 2, 3) eq "b");	   //from 2nd
	assert(substr(var("ab"), 3, 3) eq "");	   //from one after the end of the string ??

	//
	assert(substr(var(""), -2, -2) eq "");
	assert(substr(var(""), -2, -1) eq "");
	assert(substr(var(""), -2, 0)  eq "");
	assert(substr(var(""), -2, 1)  eq "");
	assert(substr(var(""), -2, 2)  eq "");
	//
	assert(substr(var(""), -1, -2) eq "");
	assert(substr(var(""), -1, -1) eq "");
	assert(substr(var(""), -1, 0)  eq "");
	assert(substr(var(""), -1, 1)  eq "");
	assert(substr(var(""), -1, 2)  eq "");
	//
	assert(substr(var(""), 0, -2)  eq "");
	assert(substr(var(""), 0, -1)  eq "");
	assert(substr(var(""), 0, 0)   eq "");
	assert(substr(var(""), 0, 1)   eq "");
	assert(substr(var(""), 0, 2)   eq "");
	//
	assert(substr(var(""), 1, -2)  eq "");
	assert(substr(var(""), 1, -1)  eq "");
	assert(substr(var(""), 1, 0)   eq "");
	assert(substr(var(""), 1, 1)   eq "");
	assert(substr(var(""), 1, 2)   eq "");
	//
	assert(substr(var(""), 2, -2)  eq "");
	assert(substr(var(""), 2, -1)  eq "");
	assert(substr(var(""), 2, 0)   eq "");
	assert(substr(var(""), 2, 1)   eq "");
	assert(substr(var(""), 2, 2)   eq "");

	////////////////////////////////////////////////////////////////////////////////////////
	//var var::substr(const int startindex1, const var& delimiterchars, int& endindex) const
	////////////////////////////////////////////////////////////////////////////////////////

	int pos;
	//from beginning of string (char 1) or zero or negative the same
	assert(var("abc/:cde").b(-1, ":/", pos) eq "abc");
	assert(var("abc/:cde").b(0, ":/", pos)  eq "abc");
	assert(var("abc/:cde").b(1, ":/", pos)  eq "abc");

	//starting after end of string
	assert(var("abc/:cde").b(10, ":/", pos) eq "");
	assert(pos                                   eq 9);

	//no delimiter found
	//start from middle
	assert(var("abc/:cde").b(3, "xyz", pos) eq "c/:cde");
	assert(pos                                   eq 9);

	//note multibyte delimiters and indexing dont work properly due to both being bytewise currently
	//var greek5x4b="αβγδεαβγδεαβγδεαβγδε";
	//assert(greek5x4b.b(3,"β",pos) eq "γδεα");

	//var var::insert(const int fieldno, const int valueno, const int subvalueno, const var& insertion) const&
	// given only field and value numbers
	//var var::insert(const int fieldno, const int valueno, const var& insertion) const&
	// given only field number
	//var var::insert(const int fieldno, const var& insertion) const&
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	//this shouldnt compile without a warning
	//insert(da1,3,"x");//or this
	//insert(da1,3,3,"x");//or this
	//insert(da1,3,3,3,"x");//or this

	var t1 = "aa";

	assert(insert(t1, -2, "xyz").convert(_FM _VM _SM, "^]}") eq "aa^xyz");	//-2 is the same as -1 i.e append
	assert(insert(t1, -1, "xyz").convert(_FM _VM _SM, "^]}")   eq "aa^xyz");
	assert(insert(t1, 0, "xyz").convert(_FM _VM _SM, "^]}")    eq "xyz^aa");
	assert(insert(t1, 2, "xyz").convert(_FM _VM _SM, "^]}")    eq "aa^xyz");
	assert(insert(t1, 3, "xyz").convert(_FM _VM _SM, "^]}")    eq "aa^^xyz");
	assert(insert(t1, 1, 1, "xyz").convert(_FM _VM _SM, "^]}") eq "xyz]aa");
	assert(insert(t1, 2, 1, "xyz").convert(_FM _VM _SM, "^]}") eq "aa^xyz");
	assert(insert(t1, 2, 2, "xyz").convert(_FM _VM _SM, "^]}") eq "aa^]xyz");

	assert(var("f110" _VM "f120" _VM "f130" _VM "f140" _FM "f211" _VM "f212").inserter(1, 3, "xyz").convert(_FM _VM _SM, "^]}") eq "f110]f120]xyz]f130]f140^f211]f212");
	assert(var(t1).inserter(1, "xyz").convert(_FM _VM _SM, "^]}")                                                               eq "xyz^aa");

	// free function
	{
		var x = "f110" _VM "f120" _VM "f130" _VM "f140" _FM "f211" _VM "f212";
		inserter(x, 2, "xyz");
		assert(x.convert(_FM _VM _SM, "^]}") eq "f110]f120]f130]f140^xyz^f211]f212");
	}
	{
		var x = "f110" _VM "f120" _VM "f130" _VM "f140" _FM "f211" _VM "f212";
		inserter(x, 1, 3, "xyz");
		assert(x.convert(_FM _VM _SM, "^]}") eq "f110]f120]xyz]f130]f140^f211]f212");
	}
	{
		var x = "f110" _VM "f120" _VM "f130" _VM "f140" _FM "f211" _VM "f212";
		inserter(x, 1, 3, 1, "xyz").dump();
		assert(x.convert(_FM _VM _SM, "^]}") eq "f110]f120]xyz}f130]f140^f211]f212");
	}

	assert(insert(t1, 2, -1, "xyz").convert(_FM _VM _SM, "^]}")                                                       eq "aa^xyz");
	assert(var("aa" _FM _VM).insert(1, -1, "xyz").convert(_FM _VM _SM, "^]}")                                         eq "aa]xyz^]");
	assert(var("aa" _FM _SM).insert(1, 1, -1, "xyz").convert(_FM _VM _SM, "^]}")                                      eq "aa}xyz^}");
	assert(var("aa" _FM _SM).insert(1, 2, -1, "xyz").convert(_FM _VM _SM, "^]}")                                      eq "aa]xyz^}");
	assert(var("aa" _FM _SM).insert(1, 3, -1, "xyz").convert(_FM _VM _SM, "^]}")                                      eq "aa]]xyz^}");
	assert(var("aa" _FM _SM).insert(1, 3, 4, "xyz").convert(_FM _VM _SM, "^]}")                                       eq "aa]]}}}xyz^}");
	assert(var("f111" _SM "f112" _SM "f113" _FM "f211" _SM "f212").insert(1, 1, 2, "xyz").convert(_FM _VM _SM, "^]}") eq "f111}xyz}f112}f113^f211}f212");

	assert(insert(t1, 2, 2, -2, "xyz").convert(_FM _VM _SM, "^]}") eq "aa^]xyz");  //-2 same as -1
	assert(insert(t1, 2, 2, -1, "xyz").convert(_FM _VM _SM, "^]}") eq "aa^]xyz");
	assert(insert(t1, 2, 2, 0, "xyz").convert(_FM _VM _SM, "^]}")  eq "aa^]xyz");
	assert(insert(t1, 2, 2, 1, "xyz").convert(_FM _VM _SM, "^]}")  eq "aa^]xyz");
	assert(insert(t1, 2, 2, 2, "xyz").convert(_FM _VM _SM, "^]}")  eq "aa^]}xyz");

	assert(insert(t1, 1, 2, -1, "xyz").convert(_FM _VM _SM, "^]}") eq "aa]xyz");
	assert(insert(t1, 1, 2, -1, "xyz").convert(_FM _VM _SM, "^]}") eq "aa]xyz");

	//on a temporary
	assert(var(t1).insert(-1, "xyz").convert(_FM _VM _SM, "^]}")      eq "aa^xyz");
	assert(var(t1).insert(2, 2, "xyz").convert(_FM _VM _SM, "^]}")    eq "aa^]xyz");
	assert(var(t1).insert(2, 2, 2, "xyz").convert(_FM _VM _SM, "^]}") eq "aa^]}xyz");

	t1 = "";
	assert(insert(t1, -1, "xyz").convert(_FM _VM _SM, "^]}")   eq "xyz");
	assert(insert(t1, 0, "xyz").convert(_FM _VM _SM, "^]}")    eq "xyz");
	assert(insert(t1, 1, "xyz").convert(_FM _VM _SM, "^]}")    eq "xyz");
	assert(insert(t1, 2, "xyz").convert(_FM _VM _SM, "^]}")    eq "^xyz");
	assert(insert(t1, 3, "xyz").convert(_FM _VM _SM, "^]}")    eq "^^xyz");
	assert(insert(t1, 1, 1, "xyz").convert(_FM _VM _SM, "^]}") eq "xyz");
	assert(insert(t1, 2, 1, "xyz").convert(_FM _VM _SM, "^]}") eq "^xyz");
	assert(insert(t1, 2, 2, "xyz").convert(_FM _VM _SM, "^]}") eq "^]xyz");

	//insert "I2" at field 2
	da1 = "f1" _FM "f2";
	assert(insert(da1, 2, "I2") eq("f1" _FM "I2" _FM "f2"));
	assert(insert(da1, 2, 1, "I21") eq("f1" _FM "I21" _VM "f2"));
	assert(insert(da1, 2, 1, 1, "I211") eq("f1" _FM "I211" _SM "f2"));

	////////////////////////
	//remove (pickos delete)
	////////////////////////

	//remove field 1
	da1 = "f1" _FM "f2";
	assert(remove(da1, 1) eq("f2"));
	assert(remover(da1, 1) eq("f2"));
	assert(da1 eq "f2");

	//remove field 1, value 2
	da1 = "f1" _VM "f1v2" _VM "f1v3" _FM "f2";
	assert(remove(da1, 1, 2) eq("f1" _VM "f1v3" _FM "f2"));

	//remove field 1, value 2, subvalue 2
	da1 = "f1" _VM "f1v2s1" _SM "f1v2s2" _SM "f1v2s3" _VM "f1v3" _FM "f2";
	assert(remove(da1, 1, 2, 2) eq("f1" _VM "f1v2s1" _SM "f1v2s3" _VM "f1v3" _FM "f2"));

	//remove field 2, value 1, subvalue 1
	assert(remove(da1, 2, 1, 1) eq("f1" _VM "f1v2s1" _SM "f1v2s2" _SM "f1v2s3" _VM "f1v3" _FM));
	assert(remove(da1, 2, 0, 0) eq("f1" _VM "f1v2s1" _SM "f1v2s2" _SM "f1v2s3" _VM "f1v3"));
	assert(remove(da1, 3, 0, 0) eq da1);

	da1 = "1^2^31]32]331|332|333]34^4";
	assert(convert(da1, "^]|", _FM _VM _SM).remove(3, 3, 0) eq var("1^2^31]32]34^4").convert("^]|", _FM _VM _SM));

	da1 = "1^2^311|312]32]331|332|333]34^4";
	assert(convert(da1, "^]|", _FM _VM _SM).remove(3, 1, 0) eq var("1^2^32]331|332|333]34^4").convert("^]|", _FM _VM _SM));

	da1 = "1^2^311|312]32]331|332|333]34^4";
	assert(convert(da1, "^]|", _FM _VM _SM).remove(3, 1, 1) eq var("1^2^312]32]331|332|333]34^4").convert("^]|", _FM _VM _SM));

	//remove 0, 0, 0
	assert(remove(da1, 0, 0, 0) eq "");

	/////
	//SUM
	/////

	//sum with defined separator
	assert(var("2245000900.76" _VM "102768099.9" _VM "-2347769000.66" _VM).sum(VM)             eq 0);
	//using free function
	assert(sum(var("2245000900.76" _VM "102768099.9" _VM "-2347769000.66" _VM), VM)            eq 0);
	//multilevel
	assert(var("2245000900.76" _VM "102760000" _SM "8099.9" _VM "-2347769000.66" _VM).sum(VM)  eq 0);
	assert(var("2245000900.76").sum(VM)                                                        eq 2245000900.76);
	assert(var("").sum(VM)                                                                     eq 0);
	//sumall
	assert(var("2245000900.76" _VM "102760000" _SM "8099.9" _VM "-2347769000.66" _VM).sumall() eq 0);

	//test sum rounds result to no more than the input decimals input
	printl(sum("2245000900.76" _VM "102768099.9" _VM "-2347769000.66" _VM));
	assert(sum("2245000900.76" _VM "102768099.9" _VM "-2347769000.66" _VM) eq 0);
	assert(sum("2245000900.76")                                            eq 2245000900.76);
	assert(sum("")                                                         eq 0);

	printl(test_sum("1}2]3}4"));
	assert(test_sum("1]2^3]4")         eq "3^7");
	assert(test_sum("1}2]3}4^9")       eq "3]7^9");
	assert(test_sum("1}2}3}4}9")       eq "19");
	assert(test_sum("1}2]3}4~9")       eq "3]7~9");
	assert(test_sum("1~2^3]5}6|7")     eq "1~2^3]5}13");
	assert(test_sum("1~2>2>2~3~4}5}6") eq "1~6~3~4}5}6");
	assert(test_sum("1~2|2|2~3~4}5}6") eq "1~6~3~4}5}6");
	assert(test_sum("1~2>2|2~3~4}5}6") eq "1~2>4~3~4}5}6");

	assert(test_sum("1~2~3~4~5~6") eq "21");  //rm
	assert(test_sum("1^2^3^4^5^6") eq "21");  //fm
	assert(test_sum("1]2]3]4]5]6") eq "21");  //vm
	assert(test_sum("1}2}3}4}5}6") eq "21");  //sm
	assert(test_sum("1>2>3>4>5>6") eq "21");  //tm
	assert(test_sum("1|2|3|4|5|6") eq "21");  //stm

	//check max decimal places respected
	assert(test_sum("1.2345|2|3|4|5|6") eq "21.2345");

	//check trailing zeros are eliminated
	assert(test_sum("1.2345|2|3|4|5|6|-.2345") eq "21");

	//check trailing zeros are not eliminated
	assert(test_sum("100|200") eq "300");

	//test strange combinations
	assert(test_sum("^-1]1").outputl()      eq "^0");
	assert(test_sum("]^-1]1").outputl()     eq "0^0");
	assert(test_sum("]7^-1]1").outputl()    eq "7^0");
	assert(test_sum("]7.10^-1]1").outputl() eq "7.1^0");
	assert(test_sum("]7.00^-1]1").outputl() eq "7^0");

	/////////////////////////
	//.mv() +++ --- *** /// :
	/////////////////////////

	assert((1 ^ FM ^ 2).mv("*", 10 ^ FM ^ 20) eq "10" _FM "40");

	var aaa = "11" ^ VM ^ VM ^ "13" ^ VM ^ FM ^ "21";
	var bbb = "1011" ^ VM ^ VM ^ "1013";

	assert(convert(aaa.mv(":", bbb), _VM _FM, "]^") eq "111011]]131013]^21");
	assert(convert(aaa.mv("+", bbb), _VM _FM, "]^") eq "1022]0]1026]0^21");
	assert(convert(aaa.mv("-", bbb), _VM _FM, "]^") eq "-1000]0]-1000]0^21");
	assert(convert(aaa.mv("*", bbb), _VM _FM, "]^") eq "11121]0]13169]0^0");

	assert(convert(bbb.mv(":", aaa), _VM _FM, "]^") eq "101111]]101313]^21");
	assert(convert(bbb.mv("+", aaa), _VM _FM, "]^") eq "1022]0]1026]0^21");
	assert(convert(bbb.mv("-", aaa), _VM _FM, "]^") eq "1000]0]1000]0^-21");
	assert(convert(bbb.mv("*", aaa), _VM _FM, "]^") eq "11121]0]13169]0^0");

	aaa = "11" ^ VM ^ VM ^ "13" ^ VM ^ FM ^ "21";
	bbb = "1011" ^ VM ^ "0" ^ VM ^ "1013" ^ VM ^ FM ^ "2011";
	assert(oconv(aaa.mv("/", bbb), "MD90P").convert(_VM _FM, "]^") eq "0.010880317]0.000000000]0.012833169]0.000000000^0.010442566");

	aaa = "" ^ VM ^ "" ^ VM ^ "0" ^ VM ^ "0" ^ VM;
	bbb = "" ^ VM ^ "0" ^ VM ^ "" ^ VM ^ "0" ^ VM ^ "" ^ VM ^ "0" ^ VM ^ "" ^ VM ^ "0";
	assert(aaa.mv("/", bbb).convert(_VM _FM, "]^") eq "0]0]0]0]0]0]0]0");

	//testing .mv(+ - * / :)
	var m1						= "1" _VM "2" _VM _VM "4";
	var						 m2 = "100" _VM "200" _VM "300";

	m1.convert(VM, "]").outputl("m1=");
	m2.convert(VM, "]").outputl("m2=");

	m1.mv("+", m2).convert(VM, "]").outputl("xxxx=");
	assert(convert(m1.mv("+", m2), VM, "]") eq "101]202]300]4");
	assert(convert(m1.mv("-", m2), VM, "]") eq "-99]-198]-300]4");
	assert(convert(m1.mv("*", m2), VM, "]") eq "100]400]0]0");
	assert(convert(m1.mv(":", m2), VM, "]") eq "1100]2200]300]4");

	printl();
	m2(1, 4) = 400;
	m2.convert(VM, "]").outputl("m2=");
	printl(convert(m1.mv("/", m2), VM, "]"));
	printl("should be \"0.01]0.01]0]0.01\"");
	assert(convert(m1.mv("/", m2), VM, "]") eq "0.01]0.01]0]0.01");

	////////
	//var[n] character selector
	////////

	//accessing individual characters by index 1=first -1=last etc.
	var a = "abc";
	assert(a[1] eq "a");   //a = first character
	assert(a[2] eq "b");   //b = second character
	assert(a[4] eq "");	   //"" = if access after last character
	assert(a[-1] eq "c");  //c = last character
	assert(a[-2] eq "b");  //b = last but one character
	assert(a[-9] eq "a");  //a = first character if too negative
	assert(a[0] eq "a");   //a = zero is the same as too negative

	a = "";
	assert(a[1]  eq "");
	assert(a[2]  eq "");
	assert(a[4]  eq "");
	assert(a[-1] eq "");
	assert(a[-2] eq "");
	assert(a[-9] eq "");
	assert(a[0]  eq "");

	{
		var left = "left";

		//var
		assert(var("leftx").starts(left));
		assert(var("xleft").ends(left));
		assert(var("xleftx").contains(left));

		//cstr
		assert(var("left").starts("left"));
		assert(var("left").ends("left"));
		assert(var("left").contains("left"));
		assert(var("xleft").contains("left"));
		assert(var("xleftx").contains("left"));

		assert(var("leftx").starts("left"));
		assert(var("xleft").ends("left"));
		assert(var("xleft").contains("left"));

		assert(!var("left").starts("leftx"));
		assert(!var("left").ends("xleft"));
		assert(!var("left").contains("xleft"));
		assert(!var("xl").contains("xleft"));

		assert(!var("left").starts("xleft"));
		assert(!var("left").ends("leftx"));
		assert(!var("left").contains("leftx"));
		assert(!var("left").contains("lx"));
		assert(!var("left").contains("ex"));

		//char
		//		assert(var("leftx").starts('l'));
		//		assert(var("xleft").ends('t'));
		//		assert(var("xleft").contains('t'));
		//		assert(var("xleft").contains('e'));
		//		assert(var("xleft").contains('x'));
		//
		//		assert(! var("leftx").starts('x'));
		//		assert(! var("xleft").ends('x'));
		//		assert(! var("xleft").contains('y'));
	}

	{
		printl();
		printl("Check var_proxy1 can be used directly as a bool");
		var x = "xxx^0"_var;
		assert(x(1) and not x(2));
	}

	{
		printl();
		printl("Check var_proxy2 can be used directly as a bool");
		var x = "xxx^0]0]1"_var;
		assert(x(2, 3) and not x(2, 2));
	}

	{
		printl();
		printl("Check var_proxy3 can be used directly as a bool");
		var x = "xxx^0]0]0}0}1"_var;
		assert(x(2, 3, 3) and not x(2, 3, 2));
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
