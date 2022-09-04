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

	// First *byte* of a var may not be valid utf8
	assert(var(greek5x4.toChar()).oconv("HEX").outputl() == "CE");

	assert(textseq("") == "");

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
		fieldstorer(x," ",2,0,"yyy");
		printl(x);
		assert(x == "a yyy b c d");

		x="";
		fieldstorer(x,"|",2,0,"yyy");
		printl(x);
		assert(x == "|yyy");

		x="";
		fieldstorer(x,"β",2,2,"y");
		printl(x);
		assert(x == "βyβ");

		x="";
		fieldstorer(x,"β",3,1,"y");
		printl(x);
		assert(x == "ββy");

		x="";
		fieldstorer(x,"β",3,2,"y");
		printl(x);
		assert(x == "ββyβ");

		x="";
		fieldstorer(x,"β",3,2,"yyy");
		printl(x);
		assert(x == "ββyyyβ");

		x="";
		fieldstorer(x,"β",3,3,"yyy");
		printl(x);
		assert(x == "ββyyyββ");

		x="";
		fieldstorer(x,"β",3,4,"yyy");
		printl(x);
		assert(x == "ββyyyβββ");
	}

	////////
	//locate
	////////

	var vn;

	// BY cannot be XX
	try {var x = "xyz"_var.locateby("XX", "xyz", MV);assert(false);} catch(MVError e){};

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
	// free function
	assert(locateusing(",", "s2", var("AA" _FM_ "f2v1" _VM_ "f2v2s1" _SM_ "f2,v2,s2"),vn,2,2,2));
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
	// free function
	assert(locateby("AL","A",var("1" _FM_ "10" _FM_ "2" _FM_ "B"),lbvn,0)||lbvn==4);
	var order2 = "AL";
	assert(locateby(order2,"A",var("1" _FM_ "10" _FM_ "2" _FM_ "B"),lbvn,0)||lbvn==4);

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
	converter(ascints," ",VM);
	converter(revints," ",VM);
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

	converter(ascints,VM,",");
	converter(revints,VM,",");

	assert(ascints.locateusing(",",30));
	assert(!ascints.locateusing(",",31));

	ascints.locateusing(",",30,locii);
	assert(locii==6);

	ascints.locateusing(locsep,30,locii);
	assert(locii==6);


	converter(ascints,",",VM);
	converter(revints,",",VM);

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

	converter(ascints,VM,",");
	converter(revints,VM,",");

	assert(locateusing(",",30,ascints)==1);
	assert(locateusing(",",31,ascints)==0);

	locateusing(",",30,ascints,locii);
	assert(locii==6);

	locateusing(locsep,30,ascints,locii);
	assert(locii==6);

	printl("Test passed");

	return 0;

}

//function to conveniently test sum function
function test_sum(in instr)
{
	return sum(instr.convert("~^]}>|", _RM_ _FM_ _VM_ _SM_ _TM_ _STM_)).convert(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_,"~^]}>|");
}

programexit()
