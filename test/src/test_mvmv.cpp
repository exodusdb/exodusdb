#undef NDEBUG //because we are using assert to check actual operations that cannot be skipped in release mode testing
#include <cassert>

#include <exodus/program.h>
programinit()

function main() {
	printl("test_mvmv says 'Hello World!'");

	////////////
	//fieldstore
	////////////

	//multibyte sep fieldstore
    var greek5x4="αβγδεαβγδεαβγδεαβγδε";
	var greek5x2="αβγδεαβγδε";
	var greekstr2=fieldstore(greek5x2,"β",2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	assert(greekstr2.length()==15);
	//on a temporary
	greekstr2=var(greek5x2).fieldstore("β",2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	assert(greekstr2.length()==15);

	//fieldstorer
	//in place - oo method
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	assert(greekstr2.length()==15);
	//in place - procedural
	greekstr2=greek5x2;
	fieldstorer(greekstr2,"β",2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	assert(greekstr2.length()==15);
	//empty separator character
	try {
		printl(fieldstore(greekstr2,"",2,1,"xxx"));
		assert(false && "empty separator character in fieldstore() should generate an error");
	}
	catch (MVError e) {
	}
	try {
		printl(field(greekstr2,"",1));
		assert(false && "empty separator character in field() should generate an error");
	}
	catch (MVError e) {
	}
	//fieldstore after end of string
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",5,1,"xxx");
    assert(greekstr2=="αβγδεαβγδεββxxx");
	//negative field number from back
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",-2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	assert(greekstr2.length()==15);
	//positive number of fields indicates to replace fieldwise with the fields in the insertion. empty fields if the number of fields in the insertion value is insufficient
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",2,2,"xxx");
	assert(greekstr2=="αβxxxβ");
	assert(greekstr2.length()==9);
	//positive number of fields indicates to replace fieldwise with the fields in the insertion.
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",2,2,"xxxβyyy");
	assert(greekstr2=="αβxxxβyyy");
	assert(greekstr2.length()==12);
	//negative number of fields indicates to replace that positive number of fields with whatever is the insertion.
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",2,-2,"xxx");
	assert(greekstr2=="αβxxx");
	assert(greekstr2.length()==7);
	//replacing an empty field with something
	greekstr2="αββγδε";
	greekstr2.fieldstorer("β",2,1,"xxx");
	assert(greekstr2=="αβxxxβγδε");
	//replacing a field with nothing
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",2,1,"");
	assert(greekstr2=="αββγδε");
	//replacing an empty field with nothing
	greekstr2="αββγδε";
	greekstr2.fieldstorer("β",2,1,"");
	//replacing 0 fields
	greekstr2=greek5x2;
	greekstr2.fieldstorer("β",2,0,"xxx");
	printl(greekstr2);
	//assert(greekstr2 == "αβγδεαβγδε");

	//insert into empty string
	greekstr2="";
	greekstr2.fieldstorer("β",1,3,"xxx");
	printl(greekstr2);
	assert(greekstr2=="xxxββ");
	//insert into empty string
	greekstr2="";
	greekstr2.fieldstorer("β",0,0,"");
	printl(greekstr2);
	assert(greekstr2=="");
	//insert into empty string
	greekstr2="";
	greekstr2.fieldstorer("β",3,3,"xxx");
	printl(greekstr2);
	assert(greekstr2=="ββxxxββ");

	//temporary
	greekstr2=fieldstore(greek5x2,"β",2,1,"1β2β3");
	assert(greekstr2=="αβ1βγδε");

	greekstr2=fieldstore(greek5x2,"β",2,-1,"1β2β3");
	assert(greekstr2=="αβ1β2β3βγδε");

	//var greek5x4="αβγδεαβγδεαβγδεαβγδε";
	assert(fieldstore(greek5x4,"β",2,3,"ζ")=="αβζβββγδε");
	assert(fieldstore(greek5x4,"β",2,-3,"ζ")=="αβζβγδε");

	//more fieldstorer
	{
		var x="a b c d";
		x.fieldstorer(" ",2,0,"yyy");
		printl(x);
		assert(x == "a yyy b c d");

		x="";
		x.fieldstorer("|",2,0,"yyy");
		printl(x);
		assert(x == "|yyy");

		x="";
		x.fieldstorer("β",2,2,"y");
		printl(x);
		assert(x == "βyβ");

		x="";
		x.fieldstorer("β",3,1,"y");
		printl(x);
		assert(x == "ββy");

		x="";
		x.fieldstorer("β",3,2,"y");
		printl(x);
		assert(x == "ββyβ");

		x="";
		x.fieldstorer("β",3,2,"yyy");
		printl(x);
		assert(x == "ββyyyβ");

		x="";
		x.fieldstorer("β",3,3,"yyy");
		printl(x);
		assert(x == "ββyyyββ");

		x="";
		x.fieldstorer("β",3,4,"yyy");
		printl(x);
		assert(x == "ββyyyβββ");
	}

	////////
	//locate
	////////

	var vn;

	assert(var("f100" _FM_ "f210" _VM_ "" _VM_ "f230").locateby("AL", "", vn, 2));
	assert(vn == 2);

	assert(var("f100" _FM_ "f210" _VM_ "" _VM_ "f230").locateby("AL", "", vn, 2, 2));
	assert(vn == 1);

	assert(var("").locateby("AL", "", vn));
	assert(vn == 1);

	//no orderby
	assert(var("XXX").locate("XXX"));
	assert(var("AAA" _VM_ "XXX").locate("XXX"));
	assert(var("AA" _VM_ "BB" _FM_ "22").locate("BB",vn,1));
	assert(!var("AA" _FM_ "BB").locate("BB",vn,1));
	assert(var("AA" _FM_ "BB1" _VM_ "BB2").locate("BB2",vn,2));
	assert (vn==2);
	//
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2").locate("f2v2",vn,2));
	assert (vn==2);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2" _SM_ "X").locate("f2v2",vn,2));
	//assert (vn=2);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2" _VM_ "X").locate("f2v2",vn,2));
	assert (vn==2);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2" _FM_).locate("f2v2",vn,2));
	assert (vn==2);
	// search for a subvalue
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2v2s2").locate("f2v2s2",vn,2,2));
	assert (vn==2);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2v2s2" _SM_ "X").locate("f2v2s2",vn,2,2));
	assert (vn==2);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2v2s2" _VM_ "X").locate("f2v2s2",vn,2,2));
	assert (vn==2);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2v2s2" _FM_ "X").locate("f2v2s2",vn,2,2));
	assert (vn==2);
	//search in a subvalue using ","
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2").locateusing(",","s2",vn,2,2,2));
	assert (vn==3);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2").locateusing(",","s2",vn,2,2,-2));
	assert (vn==1);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _SM_ "X").locateusing(",","s2",vn,2,2,2));
	assert (vn==3);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _VM_ "X").locateusing(",","s2",vn,2,2,2));
	assert (vn==3);
	assert(var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _FM_ "X").locateusing(",","s2",vn,2,2,2));
	assert (vn==3);
	//search in a subvalue using "," where subvalue doesnt exist
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2").locateusing(",","s2",vn,2,2,3));
	assert (vn==1);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2").locateusing(",","s2",vn,2,2,-3));
	assert (vn==1);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _SM_ "X").locateusing(",","s2",vn,2,2,3));
	assert (vn==2);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _VM_ "X").locateusing(",","s2",vn,2,2,3));
	assert (vn==1);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _FM_ "X").locateusing(",","s2",vn,2,2,3));
	assert (vn==1);
	//check SV search doesnt cross into next multivalue or field
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _VM_ _SM_ "X").locateusing(",","s2",vn,2,2,3));
	assert (vn==1);
	assert(!var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2" _FM_ _SM_ "X").locateusing(",","s2",vn,2,2,3));
	assert (vn==1);

	//check field number zero means search using FM
	var fn;
	assert(var("aa" _FM_ "bb" _FM_ "cc").locate("bb", fn, 0));
	assert(fn == 2);

	var lbvn;
	//no fieldno/value given means using character VM
	assert(var("1" _VM_ "10" _VM_ "2" _VM_ "B").locateby("AL","A",lbvn)||lbvn==4);
	//fieldno given means search in that field using character VM
	assert(var("1" _VM_ "10" _VM_ "2" _VM_ "B").locateby("AL","A",lbvn,1)||lbvn==4);
	//fieldno given and =0 means search whole string using character FM
	assert(var("1" _FM_ "10" _FM_ "2" _FM_ "B").locateby("AL","A",lbvn,0)||lbvn==4);

	//search values within a field

	assert(!var("").locate("X",MV,2));
	assert(!var(_FM_).locate("X",MV,2));
	assert(!var(_FM_ _FM_).locate("X",MV,2));
	assert(var(_FM_ _FM_).locate("",MV,2));
	assert(!var(_FM_ "f2v1" _VM_ "f2v2"  _FM_).locate("X",MV,2));
	assert(!var(_FM_ "f2v1" _VM_ "f2v2").locate("X",MV,2));

	//search subvalues within a value within a field

	assert(!var("").locate("X",MV,2,2));
	assert(!var(_FM_).locate("X",MV,2,2));
	assert(!var(_FM_ _VM_).locate("X",MV,2,2));
	assert(!var(_FM_ _VM_ _VM_).locate("X",MV,2,2));
	assert(var(_FM_ _VM_ _VM_).locate("",MV,2,2));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2"  _VM_).locate("X",MV,2,2));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2").locate("X",MV,2,2));

	assert(!var("").locate("X",MV,2,3));
	assert(!var(_FM_ _FM_).locate("X",MV,2,3));
	assert(!var(_FM_ _VM_ _FM_).locate("X",MV,2,3));
	assert(!var(_FM_ _VM_ _VM_ _FM_).locate("X",MV,2,3));
	assert(var(_FM_ _VM_ _VM_ _FM_).locate("",MV,2,3));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2"  _VM_ _FM_).locate("X",MV,2,3));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2" _FM_).locate("X",MV,2,3));

	//check searching for VMs does not stray over into the next field
	assert(!var("").locate("X",MV,2,3));
	assert(!var(_FM_ _FM_ _VM_).locate("X",MV,2,3));
	assert(!var(_FM_ _VM_ _FM_ _VM_).locate("X",MV,2,3));
	assert(!var(_FM_ _VM_ _VM_ _FM_ _VM_).locate("X",MV,2,3));
	assert(var(_FM_ _VM_ _VM_ _FM_ _VM_).locate("",MV,2,3));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2"  _VM_ _FM_ _VM_).locate("X",MV,2,3));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2" _FM_ _VM_).locate("X",MV,2,3));

	//search subvalues within a value within a field (using a specfic separator)

	//check searching for SMs does not stray over into the next value
	assert(!var("").locateusing(",","X",MV,2,3,-3));
	assert(!var("").locateusing(",","X",MV,2,3,3));
	assert(!var(_FM_ _FM_ _VM_).locateusing(",","X",MV,2,2,2));
	assert(!var(_FM_ _VM_ _FM_ _VM_).locateusing(",","X",MV,2,2,2));
	assert(!var(_FM_ _VM_ _VM_ _FM_ _VM_).locateusing(",","X",MV,2,2,2));
	assert(var(_FM_ _VM_ _VM_ _FM_ _VM_).locateusing(",","",MV,2,2,2));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2"  _VM_ _FM_ _VM_).locateusing(",","X",MV,2,2,2));
	assert(!var(_FM_ _VM_ "f2v2s1" _SM_ "f2v2s2" _FM_ _VM_).locateusing(",","X",MV,2,2,2));

    //locate using multibyte unicode char separator and target
	//var greek5x4="αβγδεαβγδεαβγδεαβγδε";
    var setting;
    assert(greek5x4.locateusing("β","γδεα",setting));
    assert(setting==2);

	var sort="aa" _FM_ "bb" _FM_ "dd";
	var sortn;
	assert(!sort.locatebyusing(var("AL"),FM,"a",sortn));
	assert(sortn==1);
	assert(!sort.locatebyusing(var("AL"),FM,"b",sortn));
	assert(sortn==2);
	assert(!sort.locatebyusing("AL",FM,"a",sortn));
	assert(sortn==1);
	assert(sort.locatebyusing("AL",FM,"bb",sortn));
	assert(sortn==2);
	assert(!sort.locatebyusing("AL",FM,"cc",sortn));
	assert(sortn==3);
	assert(!sort.locatebyusing("AL",FM,"ee",sortn));
	assert(sortn==4);
	//usingchar is cstr
	assert(sort.locatebyusing("AL", _FM_, "bb", sortn));
	assert(sortn==2);

	assert(sort.locateusing(FM,"aa",sortn));
	assert(sortn==1);
	assert(sort.locateusing(FM,"bb",sortn));
	assert(sortn==2);
	assert(!sort.locateusing(FM,"cc",sortn));
	assert(sortn==4);
	assert(!sort.locateusing(FM,"ee",sortn));
	assert(sortn==4);

	var locii;
	var ascints="1 2 3 10 20 30 100 200 300";
	var revints="300 200 100 30 20 10 3 2 1";
	ascints.converter(" ",VM);
	revints.converter(" ",VM);
	var ar="AR";
	var locsep=",";

	//negative field number or value number finds nothing
	assert(!ascints.locate(10,MV,-1));
	assert(!ascints.convert(VM,SM).locate(10,MV,1,-1));

	assert(ascints.locate(10)==1);
	assert(ascints.locate(10));

	assert(ascints.locate(9)==0);
	assert(!ascints.locate(9));

	assert(ascints.locate(30,locii));
	assert(locii==6);

	assert(!ascints.locate(31,locii));
	assert(locii==10);

	assert(ascints.locate(30,locii,1));
	assert(locii==6);

	assert(!ascints.locate(31,locii,1));
	assert(locii==10);

	assert(!ascints.locate(31,locii,2));
	assert(locii==1);

	assert(!ascints.locateby(ar,21,locii));
	assert(locii==6);

	assert(!ascints.locateby(ar,299,locii));
	assert(locii==9);

	assert(ascints.locateby(ar,300,locii));
	assert(locii==9);

	assert(!revints.locateby("DL",0,locii));
	assert(locii==10);

	assert(revints.locateby("DL",1,locii));
	assert(locii==9);

	assert(!revints.locateby("DL",1.1,locii));
	assert(locii==9);


	assert(!revints.locateby("DR",0,locii));
	assert(locii==10);

	assert(revints.locateby("DR",1,locii));
	assert(locii==9);

	assert(!revints.locateby("DR",301,locii));
	assert(locii==1);

	assert(revints.locateby("DR",300,locii));
	assert(locii==1);

	assert(!revints.locateby("DR",299,locii));
	assert(locii==2);


	var revints2 = FM ^ revints ^ FM ^ FM ^ VM;

	assert(!revints2.locateby("DR",0,locii, 2));
	assert(locii==10);

	assert(revints2.locateby("DR",1,locii, 2));
	assert(locii==9);

	assert(!revints2.locateby("DR",301,locii, 2));
	assert(locii==1);

	assert(revints2.locateby("DR",300,locii, 2));
	assert(locii==1);

	assert(!revints2.locateby("DR",299,locii, 2));
	assert(locii==2);


	var order="AR";
	ascints.convert(VM,SM).locateby(order,20,locii,1,1);
	assert(locii==5);
	ascints.convert(VM,SM).locateby("AR",20,locii,1,1);
	assert(locii==5);


	assert(ascints.locateby("AR",20,locii));
	assert(locii==5);

	assert(ascints.locateby("AL",20,locii));
	assert(locii==5);

	assert(revints.locateby("DR",20,locii));
	assert(locii==5);

	assert(revints.locateby("DL",20,locii));
	assert(locii==5);


	assert(!ascints.locateby("AR",21,locii));
	assert(locii==6);

	assert(!ascints.locateby("AL",21,locii));
	assert(locii==3);

	assert(!revints.locateby("DR",21,locii));
	assert(locii==5);

	//?
	assert(!revints.locateby("DL",21,locii));
	TRACE(locii);
	assert(locii==2);


	assert(!ascints.locateby("AR",321,locii));
	assert(locii==10);

	assert(!ascints.locateby("AL",321,locii));
	assert(locii==10);

	assert(!revints.locateby("DR",321,locii));
	assert(locii==1);

	assert(!revints.locateby("DL",321,locii));
	assert(locii==1);

	ascints.converter(VM,",");
	revints.converter(VM,",");

	assert(ascints.locateusing(",",30));
	assert(!ascints.locateusing(",",31));

	ascints.locateusing(",",30,locii);
	assert(locii==6);

	ascints.locateusing(locsep,30,locii);
	assert(locii==6);


	ascints.converter(",",VM);
	revints.converter(",",VM);

	assert(locate(10,ascints)==1);

	locate(30,ascints,locii);
	assert(locii==6);

	locate(31,ascints,locii);
	assert(locii==10);

	locate(30,ascints,locii,1);
	assert(locii==6);

	locate(31,ascints,locii,1);
	assert(locii==10);

	locate(31,ascints,locii,2);
	assert(locii==1);

	locateby(ar,21,ascints,locii);
	assert(locii==6);

	locateby("AR",21,ascints,locii);
	assert(locii==6);

	locateby("AL",21,ascints,locii);
	assert(locii==3);

	locateby("DR",21,revints,locii);
	assert(locii==5);

	locateby("DL",21,revints,locii);
	assert(locii==2);

	ascints.converter(VM,",");
	revints.converter(VM,",");

	assert(locateusing(",",30,ascints)==1);
	assert(locateusing(",",31,ascints)==0);

	locateusing(",",30,ascints,locii);
	assert(locii==6);

	locateusing(locsep,30,ascints,locii);
	assert(locii==6);

	/////////
	//replace
	/////////

	//.r() is field/value/subvalue in place like x<1,2.3>='abc'
	// but we use pickreplace which returns a temporary for convenience in testing
	// note that replace() function in exodus is regular expression replace a with b

	var rec = "1" _FM_ "21" _VM_ "22" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4";

	assert(rec.pickreplace(1,"1r") == "1r" _FM_ "21" _VM_ "22" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4");
	printl(rec.pickreplace(2,"2r"));//1^2r^311}312]32^4
	assert(rec.pickreplace(2,"2r") == "1" _FM_ "2r" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4");

	assert(rec.pickreplace(-1,"5-1r") == "1" _FM_ "21" _VM_ "22" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4" _FM_ "5-1r");
	assert(rec.pickreplace(2,-1,"2-1r") == "1" _FM_ "21" _VM_ "22" _VM_ "2-1r" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4");
	assert(rec.pickreplace(3,2,-1,"32-1r") == "1" _FM_ "21" _VM_ "22" _FM_ "311" _SM_ "312" _VM_ "32" _SM_ "32-1r" _FM_ "4");

	assert(rec.pickreplace(2,1,"21r") == "1" _FM_ "21r" _VM_ "22" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4");
	assert(rec.pickreplace(2,4,"24r") == "1" _FM_ "21" _VM_ "22" _VM_ _VM_ "24r" _FM_ "311" _SM_ "312" _VM_ "32" _FM_ "4");

	//.r()

	//plain replace subvalue in the middle of subvalues
	var v123 = "f100" _FM_ "f210" _VM_ "f221" _SM_ "f222" _SM_;
	assert(v123.r(2, 2, 2, "n222") == "f100" _FM_ "f210" _VM_ "f221" _SM_ "n222" _SM_);
	v123(2, 2, 2) = "n222";
	assert(v123 == "f100" _FM_ "f210" _VM_ "f221" _SM_ "n222" _SM_);
	assert(var("f100" _FM_ "f210" _VM_ "f221" _SM_ "f222" _SM_).r(2, 2, 2, "n222") == "f100" _FM_ "f210" _VM_ "f221" _SM_ "n222" _SM_);
	//plain replace subvalue in the middle of subvalues with null, smaller and larger
	assert(var("f100" _FM_ "f210" _VM_ "f221" _SM_ "f222" _SM_).r(2, 2, 2, "") == "f100" _FM_ "f210" _VM_ "f221" _SM_ "" _SM_);
	assert(var("f100" _FM_ "f210" _VM_ "f221" _SM_ "f222" _SM_).r(2, 2, 2, "x") == "f100" _FM_ "f210" _VM_ "f221" _SM_ "x" _SM_);
	assert(var("f100" _FM_ "f210" _VM_ "f221" _SM_ "f222" _SM_).r(2, 2, 2, "xyz123") == "f100" _FM_ "f210" _VM_ "f221" _SM_ "xyz123" _SM_);

	assert(var("f100" _FM_ "f210" _VM_ "f221" _SM_ "f222" _SM_).r(2, "xyz123") == "f100" _FM_ "xyz123");

	//replace field zero
	assert(rec.pickreplace(0,"f000") == "f000");
	assert(rec.pickreplace(0,0,"f000") == "f000");
	assert(rec.pickreplace(0,0,0,"f000") == "f000");

	var da2="aa" _FM_ "b1" _VM_ "b2" _SM_ "b22" _FM_ "cc";

	//this shouldnt compile without a warning since it has no effect
	//pickreplace(da2,3,"x");//or this
	//pickreplace(da2,3,3,"x");//or this
	//pickreplace(da2,3,3,3,"x");//or this

	//replacement
	//da2(2)="x";//this will not compile because da2(2) is a temporary;

	da2="f1" _FM_ "f2" _FM_ "f3";

	//replace field 2 with "R2"
	da2="";
	assert(pickreplacer(da2, 2, "R2") eq ( _FM_ "R2"));

	//replace field 2, value 3 with "R22"
	da2="";
	assert(pickreplacer(da2, 2, 3, "R23") eq ( _FM_ _VM_ _VM_ "R23"));

	//replace field 2, value 3, subvalue 4 with "R234"
	da2="";
	assert(pickreplacer(da2, 2, 3, 4, "R234") eq ( _FM_ _VM_ _VM_ _SM_ _SM_ _SM_ "R234"));

	/////////
	//extract
	/////////

	//extraction
	var da1="aa" _FM_ "b1" _VM_ "b2" _SM_ "b22" _FM_ "cc";
	assert(da1.a(2) eq extract(da1,2));//this extracts field 2
	assert(da1.a(2,2) eq extract(da1,2,2));//this extracts field 2, value 2
	assert(da1.a(2,2,2) eq extract(da1,2,2,2));//this extracts field 2, value 2, subvalue 2

	//extract or a() or plain ()
	var xx = "11" _SM_ "111" _VM_ "12" _FM_ "22" _FM_ "31" _VM_ "321" _SM_ "322" _FM_;
	assert(extract(xx,3,2,2) == "322");
	assert(xx.extract(3,2,2) == "322");
	//extract beyond end of string
	assert(var("f100" _FM_ "f200").extract(3) == "");
	assert(var("f100" _FM_ "f210" _VM_).extract(2,3) == "");
	assert(var("f100" _FM_ "f210" _VM_ "f221" _SM_).extract(2,2,2) == "");
	assert(var("f100" _FM_ "f200").extract(3) == "");
	assert(var("f100" _FM_ "f210" _VM_).extract(2,3) == "");
	assert(var("f100" _FM_ "f211" _SM_).extract(2,2,2) == "");

	//plain extract subvalue from middle of subvalues
	assert(var("100" _FM_ "210" _VM_ "221" _SM_ "222" _SM_).extract(2,2,2) == "222");

	//another way
	assert(xx.a(3,2,2) == "322");
	//yet another way
	assert(xx.a(3,2,2) == "322");

	//strange requests
	assert(xx.a(-1) == "");
	assert(xx.a(-2) == "");
	assert(xx.a(1,-1) == "");
	assert(xx.a(1,1,-1) == "");
	assert(xx.a(0,0,0) == xx);
	assert(xx.a(0,2,0) == "12");
	assert(xx.a(0,0,2) == "111");

	//missing fields
	assert(xx.a(4,0,0) == "");
	assert(xx.a(1,4,0) == "");
	assert(xx.a(1,1,4) == "");
	assert(xx.a(4,1,0) == "");
	assert(xx.a(4,1,0) == "");
	assert(xx.a(4,1,2) == "");

	///////
	//field
	///////

	//arev/pick convention is that field numbers and field counts less than 1 are treated as 1
	//this is probably not a logical or even useful convention
	assert(field("aa bb cc"," ",0) == "aa");//arev
	assert(field("aa bb cc"," ",-1) == "aa");
	assert(field("aa bb cc"," ",-2) == "aa");
	assert(field("aa bb cc"," ",-3) == "aa");
	assert(field("aa bb cc"," ",-4) == "aa");//?
	assert(field("aa bb cc"," ",1) == "aa");
	assert(field("aa bb cc"," ",2) == "bb");
	assert(field("aa bb cc"," ",3) == "cc");
	assert(field("aa bb cc"," ",4) == "");
	//
	assert(field("aa bb cc"," ",0,2) == "aa bb");
	assert(field("aa bb cc"," ",-1,2) == "aa bb");
	assert(field("aa bb cc"," ",-2,2) == "aa bb");
	assert(field("aa bb cc"," ",-3,2) == "aa bb");
	assert(field("aa bb cc"," ",-4,2) == "aa bb");
	assert(field("aa bb cc"," ",1,2) == "aa bb");
	assert(field("aa bb cc"," ",2,2) == "bb cc");
	assert(field("aa bb cc"," ",3,2) == "cc");
	assert(field("aa bb cc"," ",4,2) == "");

	//field2() is SB invention being very useful to extract the last field of a string
	//it should perhaps also work like [x,y]/substr character extraction rules
	//instead of treating any references before the first field as field 1
	//and negative field counts should work backwards perhaps
	assert(field2("aa bb cc"," ",0) == "aa");//arev but perhaps should return whole string
	assert(field2("aa bb cc"," ",-1) == "cc");
	assert(field2("aa bb cc"," ",-2) == "bb");
	assert(field2("aa bb cc"," ",-3) == "aa");
	assert(field2("aa bb cc"," ",-4) == "aa");//?
	assert(field2("aa bb cc"," ",1) == "aa");
	assert(field2("aa bb cc"," ",2) == "bb");
	assert(field2("aa bb cc"," ",3) == "cc");
	assert(field2("aa bb cc"," ",4) == "");

	assert(field2("aa bb cc"," ",0,2) == "aa bb");//aren
	assert(field2("aa bb cc"," ",-1,2) == "cc");
	assert(field2("aa bb cc"," ",-2,2) == "bb cc");
	assert(field2("aa bb cc"," ",-3,2) == "aa bb");
	assert(field2("aa bb cc"," ",-4,2) == "aa bb");//?
	assert(field2("aa bb cc"," ",1,2) == "aa bb");
	assert(field2("aa bb cc"," ",2,2) == "bb cc");
	assert(field2("aa bb cc"," ",3,2) == "cc");
	assert(field2("aa bb cc"," ",4,2) == "");

	//////////////////////////////////////////////////////////////
	//var& var::substr(const int startindex1, const int length) &&
	//////////////////////////////////////////////////////////////

	//negative length reverses the characters

	var ss="abcd";
	assert(ss.substr(2,2) == "bc");

	assert(var("").substr(0) == "");
	assert(var("").substr(0,0) == "");
	assert(var("").substr(0,0) == "");
	assert(var("").substr(-2) == "");
	assert(var("").substr(-1) == "");
	assert(var("").substr(0) == "");
	assert(var("").substr(1) == "");
	assert(var("").substr(2) == "");

	//single index extracts up to the end the string. 2nd parameter defaults to length of string
	assert(var("ab").substr(-3) == "ab");//start before beginning
	assert(var("ab").substr(-2) == "ab");//last but one char
	assert(var("ab").substr(-1) == "b");//last char
	assert(var("ab").substr(0) == "ab");//start at 0 = start at 1
	assert(var("ab").substr(1) == "ab");//from 1st
	assert(var("ab").substr(2) == "b");//from 2nd
	assert(var("ab").substr(3) == "");//starting after the end of the string
	//
	//backwards 3 bytes from (including) starting character
	assert(var("ab").substr(-3,-3) == "");//start before beginning
	assert(var("ab").substr(-2,-3) == "a");//last but one char
	assert(var("ab").substr(-1,-3) == "ba");//last char
	//assert(var("ab").substr(0,-3) == "a");//start at 0 = start at 1
	assert(var("ab").substr(0,-3) == "");//start at 0 = start before beginning if negative length
	assert(var("ab").substr(1,-3) == "a");//from 1st
	assert(var("ab").substr(2,-3) == "ba");//from 2nd
	assert(var("ab").substr(3,-3) == "ba");//from one after the end of the string
	//
	//backwards 2 bytes from (including) starting character
	assert(var("ab").substr(-3,-2) == "");//start before beginning
	assert(var("ab").substr(-2,-2) == "a");//last but one char
	assert(var("ab").substr(-1,-2) == "ba");//last char
	//assert(var("ab").substr(0,-2) == "a");//start at 0 = start at
	assert(var("ab").substr(0,-2) == "");//start at 0 = start at
	assert(var("ab").substr(1,-2) == "a");//from 1st
	assert(var("ab").substr(2,-2) == "ba");//from 2nd
	//
	//backwards 2 bytes from (excluding) starting character
	assert(var("ab").substr(-3,-1) == "");//start before beginning
	assert(var("ab").substr(-2,-1) == "a");//last but one char ??
	assert(var("ab").substr(-1,-1) == "b");//last char
	//assert(var("ab").substr(0,-1) == "a");//start at 0 = start at
	assert(var("ab").substr(0,-1) == "");//start at 0 = start at
	assert(var("ab").substr(1,-1) == "a");//from 1st
	assert(var("ab").substr(2,-1) == "b");//from 2nd
	assert(var("ab").substr(3,-1) == "b");//from one after the end of the string ??
	//
	//length 0 bytes
	assert(var("ab").substr(-3,0) == "");//start before beginning
	assert(var("ab").substr(-2,0) == "");//last but one char ??
	assert(var("ab").substr(-1,0) == "");//last char
	assert(var("ab").substr(0,0) == "");//start at 0 = start at
	assert(var("ab").substr(1,0) == "");//from 1st
	assert(var("ab").substr(2,0) == "");//from 2nd
	assert(var("ab").substr(3,0) == "");//from one after the end of the string ??
	//
	//length 1 bytes
	assert(var("ab").substr(-3,1) == "a");//start before beginning ??
	assert(var("ab").substr(-2,1) == "a");//last but one char ??
	assert(var("ab").substr(-1,1) == "b");//last char
	assert(var("ab").substr(0,1) == "a");//start at 0 = start at
	assert(var("ab").substr(1,1) == "a");//from 1st
	assert(var("ab").substr(2,1) == "b");//from 2nd
	assert(var("ab").substr(3,1) == "");//from one after the end of the string ??
	//
	//length 2 bytes
	assert(var("ab").substr(-3,2) == "ab");//start before beginning
	assert(var("ab").substr(-2,2) == "ab");//last but one char
	assert(var("ab").substr(-1,2) == "b");//last char
	assert(var("ab").substr(0,2) == "ab");//start at 0 = start at
	assert(var("ab").substr(1,2) == "ab");//from 1st
	assert(var("ab").substr(2,2) == "b");//from 2nd
	assert(var("ab").substr(3,2) == "");//from one after the end of the string ??
	//
	//length 3 bytes (same as length 2 bytes since the source string is only 2 bytes)
	assert(var("ab").substr(-3,3) == "ab");//start before beginning
	assert(var("ab").substr(-2,3) == "ab");//last but one char
	assert(var("ab").substr(-1,3) == "b");//last char
	assert(var("ab").substr(0,3) == "ab");//start at 0 = start at
	assert(var("ab").substr(1,3) == "ab");//from 1st
	assert(var("ab").substr(2,3) == "b");//from 2nd
	assert(var("ab").substr(3,3) == "");//from one after the end of the string ??

	//
	assert(var("").substr(-2,-2) == "");
	assert(var("").substr(-2,-1) == "");
	assert(var("").substr(-2,0) == "");
	assert(var("").substr(-2,1) == "");
	assert(var("").substr(-2,2) == "");
	//
	assert(var("").substr(-1,-2) == "");
	assert(var("").substr(-1,-1) == "");
	assert(var("").substr(-1,0) == "");
	assert(var("").substr(-1,1) == "");
	assert(var("").substr(-1,2) == "");
	//
	assert(var("").substr(0,-2) == "");
	assert(var("").substr(0,-1) == "");
	assert(var("").substr(0,0) == "");
	assert(var("").substr(0,1) == "");
	assert(var("").substr(0,2) == "");
	//
	assert(var("").substr(1,-2) == "");
	assert(var("").substr(1,-1) == "");
	assert(var("").substr(1,0) == "");
	assert(var("").substr(1,1) == "");
	assert(var("").substr(1,2) == "");
	//
	assert(var("").substr(2,-2) == "");
	assert(var("").substr(2,-1) == "");
	assert(var("").substr(2,0) == "");
	assert(var("").substr(2,1) == "");
	assert(var("").substr(2,2) == "");

	////////////////////////////////////////////////////////////////////////////////////////
	//var var::substr(const int startindex1, const var& delimiterchars, int& endindex) const
	////////////////////////////////////////////////////////////////////////////////////////

	int pos;
	//from beginning of string (char 1) or zero or negative the same
	assert(var("abc/:cde").substr(-1,":/", pos) == "abc");
	assert(var("abc/:cde").substr(0,":/", pos) == "abc");
	assert(var("abc/:cde").substr(1,":/", pos) == "abc");

	//starting after end of string
	assert(var("abc/:cde").substr(10,":/", pos) == "");
	assert(pos ==  9);

	//no delimiter found
	//start from middle
	assert(var("abc/:cde").substr(3,"xyz", pos) == "c/:cde");
	assert(pos ==  9);

	//note multibyte delimiters and indexing dont work properly due to both being bytewise currently
	//var greek5x4b="αβγδεαβγδεαβγδεαβγδε";
	//assert(greek5x4b.substr(3,"β",pos) == "γδεα");

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

	var t1="aa";

	assert(t1.insert(-2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^xyz");//-2 is the same as -1 i.e append
	assert(t1.insert(-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^xyz");
	assert(t1.insert(0,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz^aa");
	assert(t1.insert(2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^xyz");
	assert(t1.insert(3,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^^xyz");
	assert(t1.insert(1,1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz]aa");
	assert(t1.insert(2,1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^xyz");
	assert(t1.insert(2,2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]xyz");

	assert(var("f110" _VM_ "f120" _VM_ "f130" _VM_ "f140" _FM_ "f211" _VM_ "f212").inserter(1,3,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="f110]f120]xyz]f130]f140^f211]f212");
	assert(var(t1).inserter(1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz^aa");

	assert(t1.insert(2,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^xyz");
	assert(var("aa" _FM_ _VM_).insert(1,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa]xyz^]");
	assert(var("aa" _FM_ _SM_).insert(1,1,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa}xyz^}");
	assert(var("aa" _FM_ _SM_).insert(1,2,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa]xyz^}");
	assert(var("aa" _FM_ _SM_).insert(1,3,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa]]xyz^}");
	assert(var("aa" _FM_ _SM_).insert(1,3,4,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa]]}}}xyz^}");
	assert(var("f111" _SM_ "f112" _SM_ "f113" _FM_ "f211" _SM_ "f212").insert(1,1,2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="f111}xyz}f112}f113^f211}f212");

	assert(t1.insert(2,2,-2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]xyz");//-2 same as -1
	assert(t1.insert(2,2,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]xyz");
	assert(t1.insert(2,2,0,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]xyz");
	assert(t1.insert(2,2,1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]xyz");
	assert(t1.insert(2,2,2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]}xyz");

	assert(t1.insert(1,2,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa]xyz");
	assert(t1.insert(1,2,-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa]xyz");

	//on a temporary
	assert(var(t1).insert(-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^xyz");
	assert(var(t1).insert(2,2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]xyz");
	assert(var(t1).insert(2,2,2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="aa^]}xyz");

	t1="";
	assert(t1.insert(-1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz");
	assert(t1.insert(0,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz");
	assert(t1.insert(1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz");
	assert(t1.insert(2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="^xyz");
	assert(t1.insert(3,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="^^xyz");
	assert(t1.insert(1,1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="xyz");
	assert(t1.insert(2,1,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="^xyz");
	assert(t1.insert(2,2,"xyz").convert(_FM_ _VM_ _SM_,"^]}")=="^]xyz");

	//insert "I2" at field 2
	da1="f1" _FM_ "f2";
	assert(insert(da1, 2, "I2") eq ( "f1" _FM_ "I2" _FM_ "f2"));
	assert(insert(da1, 2, 1, "I21") eq ( "f1" _FM_ "I21" _VM_ "f2"));
	assert(insert(da1, 2, 1, 1, "I211") eq ( "f1" _FM_ "I211" _SM_ "f2"));

	////////////////////////
	//remove (pickos delete)
	////////////////////////

	//remove field 1
	da1="f1" _FM_ "f2";
	assert(remove(da1, 1) eq ( "f2"));
	assert(remover(da1, 1) eq ( "f2"));
	assert(da1 == "f2");

	//remove field 1, value 2
	da1="f1" _VM_ "f1v2" _VM_ "f1v3" _FM_ "f2";
	assert(remove(da1, 1, 2) eq ("f1" _VM_ "f1v3" _FM_ "f2"));

	//remove field 1, value 2, subvalue 2
	da1="f1" _VM_ "f1v2s1" _SM_ "f1v2s2" _SM_ "f1v2s3" _VM_ "f1v3" _FM_ "f2";
	assert(remove(da1, 1, 2, 2) eq ("f1" _VM_ "f1v2s1" _SM_ "f1v2s3" _VM_ "f1v3" _FM_ "f2"));

	//remove field 2, value 1, subvalue 1
	assert(remove(da1, 2, 1, 1) eq ("f1" _VM_ "f1v2s1" _SM_ "f1v2s2" _SM_ "f1v2s3" _VM_ "f1v3" _FM_));
	assert(remove(da1, 2, 0, 0) eq ("f1" _VM_ "f1v2s1" _SM_ "f1v2s2" _SM_ "f1v2s3" _VM_ "f1v3"));
	assert(remove(da1, 3, 0, 0) eq da1);

	da1="1^2^31]32]331|332|333]34^4";
	assert(da1.convert("^]|",_FM_ _VM_ _SM_).remove(3,3,0) == var("1^2^31]32]34^4").convert("^]|",_FM_ _VM_ _SM_));

	da1="1^2^311|312]32]331|332|333]34^4";
	assert(da1.convert("^]|",_FM_ _VM_ _SM_).remove(3,1,0) == var("1^2^32]331|332|333]34^4").convert("^]|",_FM_ _VM_ _SM_));

	da1="1^2^311|312]32]331|332|333]34^4";
	assert(da1.convert("^]|",_FM_ _VM_ _SM_).remove(3,1,1) == var("1^2^312]32]331|332|333]34^4").convert("^]|",_FM_ _VM_ _SM_));

	//remove 0, 0, 0
	assert(remove(da1,0 ,0 ,0) eq "");

	/////
	//SUM
	/////

	//sum with defined separator
	assert(var("2245000900.76" _VM_ "102768099.9" _VM_ "-2347769000.66" _VM_ ).sum(VM) == 0);
	//multilevel
	assert(var("2245000900.76" _VM_ "102760000" _SM_ "8099.9" _VM_ "-2347769000.66" _VM_ ).sum(VM) == 0);
	assert(var("2245000900.76").sum(VM) == 2245000900.76);
	assert(var("").sum(VM) == 0);
	//sumall
	assert(var("2245000900.76" _VM_ "102760000" _SM_ "8099.9" _VM_ "-2347769000.66" _VM_ ).sumall() == 0);

	//test sum rounds result to no more than the input decimals input
	printl(sum("2245000900.76" _VM_ "102768099.9" _VM_ "-2347769000.66" _VM_ ));
	assert(sum("2245000900.76" _VM_ "102768099.9" _VM_ "-2347769000.66" _VM_ ) == 0);
	assert(sum("2245000900.76") == 2245000900.76);
	assert(sum("") == 0);

	printl(test_sum("1}2]3}4"));
	assert(test_sum("1]2^3]4")          == "3^7");
	assert(test_sum("1}2]3}4^9")        == "3]7^9");
	assert(test_sum("1}2}3}4}9")        == "19");
	assert(test_sum("1}2]3}4~9")        == "3]7~9");
	assert(test_sum("1~2^3]5}6|7")      == "1~2^3]5}13");
	assert(test_sum("1~2>2>2~3~4}5}6")  == "1~6~3~4}5}6");
	assert(test_sum("1~2|2|2~3~4}5}6")  == "1~6~3~4}5}6");
	assert(test_sum("1~2>2|2~3~4}5}6")  == "1~2>4~3~4}5}6");

	assert(test_sum("1~2~3~4~5~6")      == "21");//rm
	assert(test_sum("1^2^3^4^5^6")      == "21");//fm
	assert(test_sum("1]2]3]4]5]6")      == "21");//vm
	assert(test_sum("1}2}3}4}5}6")      == "21");//sm
	assert(test_sum("1>2>3>4>5>6")      == "21");//tm
	assert(test_sum("1|2|3|4|5|6")      == "21");//stm

	//check max decimal places respected
	assert(test_sum("1.2345|2|3|4|5|6")			== "21.2345");

	//check trailing zeros are eliminated
	assert(test_sum("1.2345|2|3|4|5|6|-.2345")	== "21");

	//test strange combinations
	assert(test_sum("^-1]1")== "^0");
	assert(test_sum("]^-1]1")== "0^0");
	assert(test_sum("]7^-1]1")== "7^0");
	assert(test_sum("]7.10^-1]1")== "7.1^0");
	assert(test_sum("]7.00^-1]1")== "7^0");

	/////////////////////////
	//.mv() +++ --- *** /// :
	/////////////////////////


	assert((1 ^ FM ^ 2).mv("*",10 ^ FM ^ 20) == "10" _FM_ "40");

	var aaa="11"^VM^VM^"13"^VM^FM^"21";
	var bbb="1011"^VM^VM^"1013";

	assert(aaa.mv(":",bbb).convert(_VM_ _FM_, "]^")=="111011]]131013]^21");
	assert(aaa.mv("+",bbb).convert(_VM_ _FM_, "]^")=="1022]0]1026]0^21");
	assert(aaa.mv("-",bbb).convert(_VM_ _FM_, "]^")=="-1000]0]-1000]0^21");
	assert(aaa.mv("*",bbb).convert(_VM_ _FM_, "]^")=="11121]0]13169]0^0");

	assert(bbb.mv(":",aaa).convert(_VM_ _FM_, "]^")=="101111]]101313]^21");
	assert(bbb.mv("+",aaa).convert(_VM_ _FM_, "]^")=="1022]0]1026]0^21");
	assert(bbb.mv("-",aaa).convert(_VM_ _FM_, "]^")=="1000]0]1000]0^-21");
	assert(bbb.mv("*",aaa).convert(_VM_ _FM_, "]^")=="11121]0]13169]0^0");

	aaa="11"  ^VM^VM^"13"  ^VM^FM^"21";
	bbb="1011"^VM^"0"^VM^"1013"^VM^FM^"2011";
	printl(aaa.mv("/",bbb).oconv("MD90P").convert(_VM_ _FM_, "]^"));
	assert(aaa.mv("/",bbb).oconv("MD90P").convert(_VM_ _FM_, "]^")=="0.010880317]0.000000000]0.012833169]0.000000000^0.010442566");

	aaa=""^VM^"" ^VM^"0"^VM^"0"^VM;
	bbb=""^VM^"0"^VM^""^VM^"0"^VM^""^VM^"0"^VM^""^VM^"0";
	assert(aaa.mv("/",bbb).convert(_VM_ _FM_, "]^")=="0]0]0]0]0]0]0]0");

	//testing .mv(+ - * / :)
	var m1="1" _VM_ "2" _VM_ _VM_ "4";
	var m2="100" _VM_ "200" _VM_ "300"; 

	m1.convert(VM,"]").outputl("m1=");
	m2.convert(VM,"]").outputl("m2=");

	m1.mv("+",m2).convert(VM,"]").outputl("xxxx=");
	assert(m1.mv("+",m2).convert(VM,"]")=="101]202]300]4");
	assert(m1.mv("-",m2).convert(VM,"]")=="-99]-198]-300]4");
	assert(m1.mv("*",m2).convert(VM,"]")=="100]400]0]0");
	assert(m1.mv(":",m2).convert(VM,"]")=="1100]2200]300]4");

	printl();
	m2(1, 4) = 400;
	m2.convert(VM,"]").outputl("m2=");
	printl(m1.mv("/",m2).convert(VM,"]"));
	printl("should be \"0.01]0.01]0]0.01\"");
	assert(m1.mv("/",m2).convert(VM,"]")=="0.01]0.01]0]0.01");


	////////
	//var[n] character selector
	////////

	//accessing individual characters by index 1=first -1=last etc.
	var a="abc";
	assert(a[1] eq "a");	//a = first character
	assert(a[2] eq "b");	//b = second character
	assert(a[4] eq "");	//"" = if access after last character
	assert(a[-1] eq "c");	//c = last character
	assert(a[-2] eq "b");	//b = last but one character
	assert(a[-9] eq "a");	//a = first character if too negative
	assert(a[0] eq "a");	//a = zero is the same as too negative

	a="";
	assert(a[1] eq "");
	assert(a[2] eq "");
	assert(a[4] eq "");
	assert(a[-1] eq "");
	assert(a[-2] eq "");
	assert(a[-9] eq "");
	assert(a[0] eq "");

	{
		var left="left";

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

		assert(! var("left").starts("leftx"));
		assert(! var("left").ends("xleft"));
		assert(! var("left").contains("xleft"));
		assert(! var("xl").contains("xleft"));

		assert(! var("left").starts("xleft"));
		assert(! var("left").ends("leftx"));
		assert(! var("left").contains("leftx"));
		assert(! var("left").contains("lx"));
		assert(! var("left").contains("ex"));

		//char
		assert(var("leftx").starts('l'));
		assert(var("xleft").ends('t'));
		assert(var("xleft").contains('t'));
		assert(var("xleft").contains('e'));
		assert(var("xleft").contains('x'));

		assert(! var("leftx").starts('x'));
		assert(! var("xleft").ends('x'));
		assert(! var("xleft").contains('y'));


	}

	printl("Test passed");

	return 0;

}

//function to conveniently test sum function
function test_sum(in instr)
{
	return sum(instr.convert("~^]}>|", _RM_ _FM_ _VM_ _SM_ _TM_ _STM_)).convert(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,"~^]}>|");
}

programexit()
