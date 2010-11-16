#include <exodus/program.h>
#include <cassert>

programinit()

function xyz(in xyzz)
{
	xyzz(2,2,2).outputl();
	return 1;
}

function accrest() {
        var infilename="\\tapex";//=field(sentence()," ",2);
        var infile;
        if (not osopen(infilename,infile))
                abort("Cant read "^infilename);

        var fms=FM^VM^SM^TM^STM^SSTM;
        var visibles="^]\???";
        var EOL="\n\r";
        var offset=0;
        var blocksize=50000;
        while (true) {
                var block=osbread(infile,offset,blocksize);
 //printl(offset," ",len(block));
                if (not len(block))
                        break;
                offset+=blocksize;
                converter(block,fms,visibles);
                swapper(block,IM,EOL);
                print(block);
				var xx;
                inputn(xx,1);
        }
        return 0;
}

//program()

function main()
{
	//DBTRACE=true;

	assert(COMMAND eq "service"
	 or COMMAND eq "main"
	 or COMMAND eq "main2"
	 or COMMAND eq "main2.out");

	printl(var().version());

	var errmsg;
	//if (not createdb("steve",errmsg))
	//	errmsg.outputl();
	//if (not deletedb("steve",errmsg))
	//	errmsg.outputl();

	deletefile("XUSERS");
	deletefile("dict_XUSERS");

	createfile("XUSERS");
	createfile("DICT_XUSERS");

	write("S"^FM^FM^"Age in Days"^FM^FM^FM^FM^FM^FM^"R"^FM^"10","DICT_XUSERS","AGE_IN_DAYS");
	write("S"^FM^FM^"Age in Years"^FM^FM^FM^FM^FM^FM^"R"^FM^"10","DICT_XUSERS","AGE_IN_YEARS");
	write("F"^FM^1^FM^"Number"^FM^FM^FM^FM^FM^FM^"R"^FM^"10","DICT_XUSERS","NUMBER");

	write("1","XUSERS","1");
	write("2","XUSERS","2");
	write("3","XUSERS","3");
	write("4","XUSERS","4");
//DBTRACE=true;
	assert(createindex("XUSERS","NUMBER"));
	assert(listindexes("XUSERS") eq ("xusers"^VM^"number"));
	assert(listindexes() ne "");
	assert(deleteindex("XUSERS","NUMBER"));
	assert(listindexes("XUSERS") eq "");

	assert(select("SELECT XUSERS WITH NUMBER BETWEEN 2 and 3"));
	assert(readnext(ID));
	assert(ID eq 2);
	assert(readnext(ID));
	assert(ID eq 3);
	assert(not readnext(ID));

//	if (not selectrecord("SELECT XUSERS WITH AGE_IN_DAYS GE 0 AND WITH AGE_IN_YEARS GE 0"))
//	if (not select("SELECT XUSERS"))
	if (not selectrecord("SELECT XUSERS"))
		printl("Failed to Select");

	DICT="dict_XUSERS";
	while (readnextrecord(RECORD,ID))
//	while (readnext(ID))
	{
		printl(ID);
		print("ID=",ID, " RECORD=",RECORD);

		continue;
//following requires dict_XUSERS to be a dictionary library something like
//edic dict_XUSERS
/*
#include <exodus/dict.h>

dict(AGE_IN_DAYS) {
	ANS=date()-RECORD(1);
}

dict(AGE_IN_YEARS) {
	ANS=calculate("AGE_IN_DAYS")/365.25;
}
*/
		print(" AGE_IN_DAYS=",calculate("AGE_IN_DAYS"));
		printl(" AGE_IN_YEARS=",calculate("AGE_IN_YEARS"));
	}


	assert(unquote("\"This is quoted?\"") eq "This is quoted?");

	printl("Verify that exodus catches c++ defect at runtime");
	try {
		var abc=abc+1;
		//should never get here because above should cause a runtime undefined error
//		assert(false);
	}
	catch (MVUndefined mve) {
		mve.description.outputl();
	}
	catch(MVUnassigned mve) {
		mve.description.outputl();
	}
	catch (...) {
		//should never get here because above should cause a runtime error
		assert(false);
	}

	//this turns on tracing SQL statements to postgres
	//DBTRACE=true;

	var ee="";
	var e0=0;
	var e1=1;
	var e2=2;
	var e3=3;

	assert((e2+true) eq 3);
	assert((e2+false) eq 2);
	assert((true+e2) eq 3);
	assert((false+e2) eq 2);

	assert((e2-true) eq 1);
	assert((e2-false) eq 2);
	assert((true-e2) eq -1);
	assert((false-e2) eq -2);

	assert((e2*true) eq 2);
	assert((e2*false) eq 0);
	assert((true*e2) eq 2);
	assert((false*e2) eq 0);

	//assert((e2*true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2*false) eq 0);//ditto
	assert((true/e2) eq 0.5);
	assert((false/e2) eq 0);

	//assert((e2%true) eq 2);//wont compile to prevent risk of divide by zero if true is an expression
	//assert((e2%false) eq 0);//ditto
	assert((true % e2) eq 1);
	assert((false % e2) eq 0);

	//the six logical comparisons >= <= > < and the lesser precedence eq  ne
	//between logical results and vars are not supported by exodus
	//because they seem useless, probably non existant in real code
	//and because I am too lazy at this point.
	//real code would have to be something like
	// if (cc eq aa > bb)
	//which due to lower precendence of eq and ne is actually
	// if (cc eq (aa > bb))
	assert((e1 eq true) eq true);
	assert((e1 eq false) eq false);
	assert((true eq e1) eq true);
	assert((false eq e1) eq false);

	assert((e1!=true) eq false);
	assert((e1!=false) eq true);
	assert((true!=e1) eq false);
	assert((false!=e1) eq true);

	assert((e1 eq true) eq 1);
	assert((e1 eq false) eq 0);
	assert((true eq e1) eq 1);
	assert((false eq e1) eq 0);

	assert((e1 ne true) eq 0);
	assert((e1 ne false) eq 1);
	assert((true ne e1) eq 0);
	assert((false ne e1) eq 1);

	assert((e3-(e3 eq var(0))) eq 3);

	//exodus's concatenate operator ^ has the wrong priority versus logical comparison operators
	//to avoid this causing unnoticed different operation than classic multivalue basic
	//exodus refuses to concatenate the result of logical expressions like e2>e3
	//printl(e1^e2>e3); //wont compile (because it would produce the wrong result)

	//just add brackets to clarify and correct the precedence for exodus and it will compile
	assert( ((e1^e2)>e3) eq 1); //=1 ... correct result because (1^2)>3 i.e. "12">3 is true

	var s1="111";
	var s2;
	exchange(s1,s2); //works even on unassigned variables (rather meaningless if both are unassigned though!)
	assert(s1.unassigned());
	assert(s2 eq "111");
	exchange(s1,s2); //check s1 can be unassigned too
	assert(s1 eq "111");
	assert(s2.unassigned());

	s1="1";
	s2="2";
	assert(exchange(s1,s2) eq "1");
	assert(s1 eq "2");
	assert(s2 eq "1");

	s1="1";
	s2="2";
	assert(transfer(s1,s2) eq "1");
	assert(s1 eq "");
	assert(s2 eq "1");

	//accessing individual characters by index 1=first -1=last etc.
	var a="abc";
	assert(a[1] eq "a");	//a = first character
	assert(a[2] eq "b");	//b = second character
	assert(a[4] eq "");	//"" = if access after last character
	assert(a[-1] eq "c");	//c = last character
	assert(a[-2] eq "b");	//b = last but one character
	assert(a[-9] eq "a");	//a = first character if too negative
	assert(a[0] eq "a");	//a = zero is the same as too negative

	//replacing a section of a string:
	//given start character number, number of characters to replace and a replacement string
	//(this is equivalent to the following classic mv basic syntax
	//but the simple [x,y] syntax is not available in curly bracket languages)
	//tempstr[2,3]='abc'

	//replacing a section of a string - method 1
	a="abcde";
	splicer(a,3,2,"xx");
	assert(a eq "abxxe");

	//replacing a section of a string - method 2
	a="abcde";
	a.splicer(-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 3 – but may be slower
	a="abcde";
	a=splice(a,-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 4 – but may be slower
	a="abcde";
	a=a.splice(-2,-3,"xx");
	assert(a eq "axxe");

	//test single character extraction
	var expected="a" ^ FM ^ "a" ^ FM ^ "b" ^ FM ^ "a" ^ FM ^ "a" ^ FM ^ "b" ^ FM ^ "";
	var tempstr2="ab";
	for (var ii=-3; ii<=3; ++ii)
		assert(tempstr2[ii] eq expected.extract(ii+4));

	//test single character extraction on ""
	tempstr2="";
	for (var ii=-3; ii<=3; ++ii)
		assert(tempstr2[ii] eq "");

	var spac1="  xxx  xxx  ";
	assert(trimmer(spac1) eq "xxx xxx");

	assert(timedate().outputl("timedate()=") ne "");

	dim a9;
	var a10;
	assert(matparse("xx"^FM^"bb",a9) eq 2);
	assert(matunparse(a9) eq ("xx" ^FM^ "bb"));

	var r[2];
	assert(unassigned(r[0]));

	dim a7(2,3);

	for (int ii=1;ii<=2;++ii)
		for (int jj=1;jj<=3;++jj)
			a7(ii,jj)=ii^var(".")^jj;

	dim a8(4,5);
	for (int ii=1;ii<=4;++ii)
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj)=ii^var(".")^jj;

	a8=2.2;

	for (int ii=1;ii<=4;++ii) {
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj).outputt("=");
//		printl();
	}

	a8=a7;

	for (int ii=1;ii<=4;++ii) {
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj).outputt("=");
//		printl();
	}

	var nfields=a7.parse("xx"^FM^"bb");
	a7(1).outputl();
	a7(2).outputl();
	a7.unparse().outputl();

	dim arrx(2,2),arry;
	arrx(1,1)="xx";
	arrx(1,1).outputl("arrx(1,1)=");
	arrx(1,2)=arrx(1,1);
	arrx(1,2).outputl("arrx(1,2)=");
	arry=arrx;
	arry(1,1).outputl();

	var aa1,aa2,aa3,aa4;
	aa1=aa2=aa3="aa";
	aa1.outputl();
	aa2.outputl();
	aa3.outputl();

	//accrest();

	initrnd(999);
	for (int ii=1; ii<1000; ++ii) {
		var time=rnd(500000).mod(86400);
//		time.outputl("Internal=")
//			.oconv("MTHS").outputl("oconv=")
//			.iconv("MTHS").outputl("iconv=");
//		assert(time.oconv("MTHS").iconv("MTHS") eq time);
//		assert(time.oconv("MTS").iconv("MTS") eq time);
	}
	var tempinp;
//	input("Press Enter ...",tempinp);
	//ensure lower case sorts before uppercase (despite "A" \x41 is less than "a" \x61)
	a="a";
	var A="A";
	assert(a<A);

	var da1="aa"^FM^"b1"^VM^"b2"^SM^"b22"^FM^"cc";
	xyz(da1);

	//extraction
	assert(da1(2) eq extract(da1,2));//this extracts field 2
	assert(da1(2,2) eq extract(da1,2,2));//this extracts field 2, value 2
	assert(da1(2,2,2) eq extract(da1,2,2,2));//this extracts field 2, value 2, subvalue 2

	//this wont work
	replace(da1,3,"x");//or this
	replace(da1,3,3,"x");//or this
	replace(da1,3,3,3,"x");//or this
	insert(da1,3,"x");//or this
	insert(da1,3,3,"x");//or this
	insert(da1,3,3,3,"x");//or this

	//replacement
	da1(2)="x";//sadly this compile and runs without error but does nothing!

	da1="f1" ^FM^ "f2" ^FM^ "f3";

	//replace field 2 with "R2"
	da1="";
	assert(replacer(da1, 2, "R2") eq ( FM ^ "R2"));

	//replace field 2, value 3 with "R22"
	da1="";
	assert(replacer(da1, 2, 3, "R23") eq ( FM ^VM^VM^ "R23"));

	//replace field 2, value 3, subvalue 4 with "R234"
	da1="";
	assert(replacer(da1, 2, 3, 4, "R234") eq ( FM^ VM^VM^ SM^SM^SM^ "R234"));

	//insert "I2" at field 2
	da1="f1" ^FM^ "f2";
	assert(inserter(da1, 2, "I2") eq ( "f1" ^FM^ "I2" ^FM^ "f2"));

	//insert "I21" at field 2, value 1
	da1="f1" ^FM^ "f2";
	assert(inserter(da1, 2, 1, "I21") eq ( "f1" ^FM^ "I21" ^VM^ "f2"));

	//insert "I211" at field 2, value 1, subvalue 1
	da1="f1" ^FM^ "f2";
	assert(inserter(da1, 2, 1, 1, "I211") eq ( "f1" ^FM^ "I211" ^SM^ "f2"));

	//erase (delete) field 1
	da1="f1" ^FM^ "f2";
	assert(eraser(da1, 1) eq ( "f2"));

	//erase (delete) field 1, value 2
	da1="f1" ^VM^ "f1v2" ^VM^ "f1v3" ^FM^ "f2";
	assert(eraser(da1, 1, 2) eq ("f1" ^VM^ "f1v3" ^FM^ "f2"));

	//erase (delete) field 1, value 2, subvalue 2
	da1="f1" ^VM^ "f1v2s1" ^SM^ "f1v2s2" ^SM^ "f1v2s3" ^VM^ "f1v3" ^FM^ "f2";
	assert(eraser(da1, 1, 2, 2) eq ("f1" ^VM^ "f1v2s1" ^SM^ "f1v2s3" ^VM^ "f1v3" ^FM^ "f2"));

	var tempfile;
	if (osopen("\\testsort.cpp",tempfile))
		printl("opened");
	else
		printl("not opened");

	//math.h seems to have been included in one of the boost or other special headers
	//in this main.cpp file and that causes confusion between math.h and exodus.h sin() and other functions.
	//we resolved the issue here by being specific about the namespace
	assert( exodus::sin(30).round(8) eq 0.5);
	assert( exodus::cos(60).round(8) eq 0.5);
	assert( exodus::tan(45).round(8) eq 1);
	assert( exodus::atan(1).round(6) eq 45);

	assert( exodus::abs(30.0) eq 30);
	assert( exodus::abs(-30.0) eq 30);
	assert( exodus::pwr(10,3) eq 1000);
	assert( exodus::exp(1).round(9) eq 2.718281828);
	assert( exodus::loge(1) eq 0);
	assert( exodus::loge(2.718281828).round(9) eq 1);
	assert( exodus::loge(10).round(9) eq 2.302585093);
	assert( exodus::sqrt(100) eq 10);

	var xx=osshellread("dir");
	//xx.osread(xx);
	/*
	var inpz=input();

	var inpx;
	input(inpx);

	var inpy;
	input("prompt",inpy);

	var inpq;
	inputn(inpq,5);
	*/

	//ossetenv("EXODUS_PORT",5433);
	//if (not connect("port=5433"))
	//	stop("Cannot connect!");

	//check floating point modulo
	assert(mod(2.3,var(1.499)).round(3).outputl() eq 0.801);
	assert(mod(-2.3,var(-1.499)).round(3).outputl() eq -0.801);
	//following is what c++ fmod does (a rather mathematical concept)
	//assert(mod(-2.3,var(1.499)).round(3).outputl() eq -0.801);
	//assert(mod(2.3,var(-1.499)).round(3).outputl() eq 0.801);
	//but arev and qm ensure that the result is somewhere from 0 up to or down to
	//(but not including) the divisor
	//method is ... do the fmod and if the result is not the same sign as the divisor, add the divisor
	assert(mod(-2.3,var(1.499)).round(3).outputl() eq 0.698);
	assert(mod(2.3,var(-1.499)).round(3).outputl() eq -0.698);

	assert(oconv(1234,"MD20P") eq "1234.00");
	assert(var(10000).oconv("DY0") eq "");

	var temp;
	if (not open("dict_test_symbolics",temp))
		createfile("dict_test_symbolics");
	write("F"^FM^0^FM^"ID","dict_test_symbolics","ID");
	write("S"^FM^FM^"Col1","dict_test_symbolics","SYMCOL1");

	writev("F","dict_test_symbolics","ID",1);

	if (not open("test_symbolics",temp))
		createfile("test_symbolics");
	write(1000^FM^2000,"test_symbolics","3000");

	if (select("select test_symbolics with SYMCOL1 = 3000 or with ID = 3000")) {
	//if (select("select test_symbolics with SYMCOL1 = 124")) {
		var key1;
		if (not readnext(key1))
			printl("no 124");
		var xrec;
		if (not xrec.read("test_symbolics",key1))
			printl(key1, " missing"); 
	}
	clearselect();

	assert(var("a")<var("B"));
	assert(var(1000).oconv("MD80").outputl("1000 MD80->") eq "1000.00000000");
	assert(var("31 JAN 2008").iconv("D") eq "14641");

	assert(var("1/31/2008").iconv("D") eq 14641);
	assert(var("2008/1/31").iconv("DS") eq "14641");

	assert(var("JAN/31/2008").iconv("D") eq "14641");
	assert(var("2008/JAN/31").iconv("DS") eq "14641");

	assert(var("31/1/2008").iconv("DE") eq "14641");
	assert(var("31 1 2008").iconv("DE") eq "14641");
	assert(var("31-1-2008").iconv("DE") eq "14641");
	assert(var("31/JAN/2008").iconv("DE") eq "14641");
	assert(var("JAN/31/2008").iconv("DE") eq "14641");
	assert(var("29 FEB 2008").iconv("D") eq "14670");

	assert(var("32/1/2008").iconv("DE") eq "");
	assert(var("30/2/2008").iconv("DE") eq "");
	assert(var("1/31/2008").iconv("DE") eq "");

	//select("select test_alphanum with f1 between 20 and 21");
	printl(oconv(10.1,"MD20"));

	/*
	var alphanum1="Flat 10a";
	var alphanum2="Flat 2b";
	//Flat 2b is before Flat 10a
	assert(naturalorder(alphanum1.tostring()) > naturalorder(alphanum2.tostring()));

	alphanum1="Part A-10";
	alphanum2="Part A-2";
	//Part A-2 is before Part A-10
	assert(naturalorder(alphanum1.tostring()) > naturalorder(alphanum2.tostring()));

	alphanum1="Part -10";
	alphanum2="Part -2";
	//Part -10 is before Part -2
	assert(naturalorder(alphanum1.tostring()) < naturalorder(alphanum2.tostring()));

	alphanum1="-10";
	alphanum2="-2";
	//-10 is before -2
	assert(naturalorder(alphanum1.tostring()) < naturalorder(alphanum2.tostring()));

	alphanum1="-1.11";
	alphanum2="-1.2";
	//-1.2 is before -1.11
	assert(naturalorder(alphanum1.tostring()) > naturalorder(alphanum2.tostring()));

	alphanum1="01.10";
	alphanum2="1.1";
	//01.10 is equal to 1.1
	assert(naturalorder(alphanum1.tostring()) = naturalorder(alphanum2.tostring()));

	alphanum1="A B C..C+";
	alphanum2="A B C.C";
	//A B C..C+ is before A B C.C
	assert(naturalorder(alphanum1.tostring()) < naturalorder(alphanum2.tostring()));
*/

	/*
	for (double xx=-5;xx<=5;++xx)
	{
		ostringstream ostringstream1;

		appenddouble2sortstring(xx,ostringstream1);

		cout.width(3);
		cout<<xx<<" "<<ostringstream1.str()<<endl;
	}
	stop();
	*/

	//investigate the bytes of a double in hex for natural sort
	double d2=1;
	double d3=2;
	if (d2<d3)
		d2=d3;
	union {double d1; char chars[8];};
	for (d1=-5;d1<=5;++d1) {
		print(d1," ");
		for (unsigned int partn=0;partn<sizeof(chars);++partn)
			//var(chars[partn]).oconv("HEX").output();
			std::cout << std::hex << int(chars[partn]) << " " ;
		printl();
	}

	//var steve;steve.input(1);

	//catching errors - doesnt work now that backtrace aborts (to prevent system crashes ... maybe better solution is to trap in main()
	//var unass;
	//try{unass+1;}
	//catch(...){};

	//using mv dimensioned arrays
	//mv dimensioned arrays have a zero element that is
	//used in case either or both of the indexes are zero
	dim arr1(3), arr2(3,3);
	arr1(0)=0;
	arr1(0,0)=0;
	for (int ii=1; ii<=3; ++ii) {
		arr1(ii)=ii;
		for (int jj=1; jj<=3; ++jj)
			arr2(ii,jj)=ii*3+jj;
	}
	assert(arr1(0) eq "0");
	assert(arr1(0,0) eq "0");
	for (int ii=1; ii<=3; ++ii) {
		assert(arr1(ii) eq ii);
		for (int jj=1; jj<=3; ++jj)
			assert(arr2(ii,jj) eq ii*3+jj);
	}

/* 
	//using c arrays UNSAFE! USE exodus dim INSTEAD;
	var arrxxx[10];

	//can use int but not var for indexing c arrays
	int intx=0;
	arrxxx[intx]="x";
	var varx=0;
	//following will not compile on MSVC (g++ is ok) due to "ambiguous" due to using var for index element
	//arrxxx[varx]="y";
	arrxxx[int(varx)]="y";
*/
	//bool cannot be used numerically ON MSVC (unlike in pick)
	//could change all logical ops to return var or find a way to allow void* pointer to promote to bool
	//or perhaps add bool to the list of automatic constructors?
	var log1="xx";
	var log2="x";
	//following will not compile now that all exodus logical operators return bool instead of var
	//if (log1 eq log2^log1) {}
/*
in.cpp(181) : error C2666: 'exodus::operator ^' : 7 overloads have similar conversions
could be 'exodus::var exodus::operator +(const int,const exodus::var &)' [found using argument-dependent lookup]
or 'exodus::var exodus::operator +(const exodus::var &,const double)' [found using argument-dependent lookup]
or 'exodus::var exodus::operator +(const exodus::var &,const int)' [found using argument-dependent lookup]
or 'exodus::var exodus::operator +(const exodus::var &,const exodus::var &)' [found using argument-dependent lookup]
or 'built-in C++ operator+(int, bool)'
or 'built-in C++ operator+(unsigned int, bool)'
or 'built-in C++ operator+(bool, bool)'
while trying to match the argument list '(exodus::var, bool)'
*/
	
	//neither will the following
	//var log3=count(log1,"x")+(log1 ne "");
	//just convert it to the better
	var log4=dcount(log1,"x");

	printl(SENTENCE);
	var("xyz").substr(4,1).outputl();

	//TODO ensure isnum converts ints larger that the maximum int to FLOATS

	var subs="xyz";
	printl(subs.substr(-1));

	assert(oconv("a","L#3").outputl() eq "a  ");
	assert(oconv("abc","L#3").outputl() eq "abc");
	assert(oconv("abcd","L#3").outputl() eq "abc");
	assert(oconv("a"^FM^"abc"^FM^"abcd","L#3").outputl() eq ("a  "^FM^"abc"^FM^"abc"));

	assert(oconv("a","R#3").outputl() eq "  a");
	assert(oconv("abc","R#3").outputl() eq "abc");
	assert(oconv("abcd","R#3").outputl() eq "bcd");
	assert(oconv("a"^FM^"abc"^FM^"abcd","R#3").outputl() eq ("  a"^FM^"abc"^FM^"bcd"));

	assert(oconv("a","T#3").outputl() eq "a  ");
	assert(oconv("abc","T#3").outputl() eq "abc");
	assert(oconv("abcd","T#3").outputl() eq ("abc"^TM^"d  "));
	assert(oconv("a"^FM^"abc"^FM^"abcd","T#3").outputl() eq ("a  "^FM^"abc"^FM^"abc"^TM^"d  "));

	assert(oconv("a","L(0)#3").outputl() eq "a00");
	assert(oconv("a","R(0)#3").outputl() eq "00a");
	assert(oconv("a","T(0)#3").outputl() eq "a00");
	assert(oconv("abcd","T(0)#3").outputl() eq ("abc"^TM^"d00"));

	initrnd(999);
	var rnd1=rnd(999);
	//777 on win32, 802 on Ubuntu 10.04 64bit
	assert(rnd1 eq 777 or rnd1 eq 802);
	assert(iconv("23 59 59","MT").outputl() eq 86399);
	assert(iconv("xx11yy12zz13P","MT").outputl() eq 83533);
	assert(iconv("24 00 00","MT").outputl() eq "");

	//http://www.regular-expressions.info/examples.html
	assert(swap("Steve Bush Bash bish","B.","Ru","ri") eq "Steve Rush Rush Rush");

	if (sizeof(wchar_t) eq 2)
	{
		//ucs-16 "fake utf16" on windows
		assert(oconv("Aa019KK","HEX") eq "00410061003000310039004B004B");
		assert(var("00410061003000310039004B004B").iconv("HEX") eq "Aa019KK");

	}
	else if (sizeof(wchar_t) eq 4)
	{
		assert(oconv("Aa019KK","HEX") eq "00000041000000610000003000000031000000390000004B0000004B");
		assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX") eq "Aa019KK");

	}

	assert(oconv("Aa019KK","HEX2") eq "41613031394B4B");
	assert(oconv("Aa019KK","HEX4") eq "00410061003000310039004B004B");
	assert(oconv("Aa019KK","HEX8") eq "00000041000000610000003000000031000000390000004B0000004B");
	assert(var("41613031394B4B").iconv("HEX2") eq "Aa019KK");
	assert(var("00410061003000310039004B004B").iconv("HEX4") eq "Aa019KK");
	assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX8") eq "Aa019KK");

	//doesnt accept FMs etc yet
	//assert(var("FF"^FM^"00").iconv("HEX").outputl() eq ("00FF"^FM^"00FF"));
	assert(var("FF"^FM^"00").iconv("HEX2").oconv("HEX2") eq "");
	//anything invalid returns empty string
	assert(var("XF").iconv("HEX").oconv("HEX") eq "");

	var time1=var("10:10:10").iconv("MT");
	assert(var("abcabdef").trim("abef") eq "cbd");
	assert(var("abcabdef").trimf("abef").trimb("abef") eq "cabd");

	var temp3="c";
	var temp2("c");
	var temp1="abc";
	//var temp2=temp2=L"xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//show cause MVUndefined and not simply crash do to assigning an uninitialised string
	//var undefinedassign=undefinedassign=L"xxx";

	//undefined behaviour but decided to allow it in order not to slow down all copy construction
	//var undefinedassign2=undefinedassign2=11;

	//undefined
	//var undefinedboolconv=undefinedboolconv?true:false;

	//undefined
	//var conn1=conn1.connect();

	//assert(crop(VM ^ FM) eq "");
	//assert(crop("xxx" ^ VM ^ FM) eq "xxx");
	assert(crop("aaa" ^ VM ^ FM ^ "bbb") eq ("aaa" ^ FM ^ "bbb"));
	assert(crop("aaa" ^ VM ^ FM ^ "bbb") eq ("aaa" ^ FM ^ "bbb"));
	assert(crop("aaa" ^ FM ^ "bbb" ^ FM ^ VM ^ SM ^ SM ^ FM ^ "ddd") eq ("aaa" ^ FM ^ "bbb" ^ FM ^ FM ^ "ddd"));
	assert(crop("aaa" ^ FM ^ "bbb" ^ FM ^ VM ^ SM ^ SM ^ FM ^ RM ^ "ddd") eq ("aaa" ^ FM ^ "bbb" ^ RM ^ "ddd"));

	assert(space(-11) eq "");
	assert(var("x").str(-7) eq "");

	var xyz;
	//xyz=xyz;
	printl("test catching MVexceptions");
	try {
		//runtime errors
		var x1=x1^=1;
		var undefx=undefx++;
		var z=z+1;
		var xx=xx.operator++();
	}
	catch (MVException except) {
		print(except.description);
	}

	var x9;
//	if (var xx eq x) {};
	//TODO implement some kind of a switch to turn off exit in mvexceptions
	var filehandle;
/*
	try {
		printl(filehandle);
	}
	catch (MVException except) {
		except.description.outputl();
	}
*/
	assert(oconv(0,"D4") eq "31 DEC 1967");
	assert(oconv("xxx","") eq "xxx");

	assert(oconv("","MX") eq "");
	assert(oconv(" ","MX") eq " ");
	assert(oconv(0,"MX") eq "0");
	assert(oconv(-0,"MX") eq "0");
	assert(oconv("X","MX") eq "X");
	//assert(oconv("-1.5","MX") eq "FFFFFFFFFFFFFFFE");
	assert(oconv("-1.5","MX") eq "FFFFFFFE");
	assert(oconv("-1","MX") eq "FFFFFFFF");
	assert(oconv("1.5","MX") eq "2");
	assert(oconv("20" ^ FM ^ 255,"MX") eq ("14" ^FM^ "FF"));

	assert(oconv("","D") eq "");
	assert(oconv("X","D") eq "X");
	assert(oconv("-1.5","D") eq "29 DEC 1967");
	assert(oconv("1.5","D") eq "01 JAN 1968");
	assert(oconv("1.5" ^ FM ^ -1.5,"D") eq ("01 JAN 1968"^FM^"29 DEC 1967"));

	assert(oconv(14276,"D") eq "31 JAN 2007");
	assert(oconv(14276,"D2") eq "31 JAN 07");
	assert(oconv(14276,"D4") eq "31 JAN 2007");
	assert(oconv(14276,"D/") eq "01/31/2007");
	assert(oconv(14276,"D ") eq "01 31 2007");
	assert(oconv(14276,"D2/") eq "01/31/07");
	assert(oconv(14276,"D2-") eq "01-31-07");
	assert(oconv(14276,"D/") eq "01/31/2007");
	assert(oconv(14276,"D/E") eq "31/01/2007");
	assert(oconv(14276,"D2 E") eq "31 01 07");
	assert(oconv(14276,"D S") eq "2007 01 31");
	assert(oconv(14276,"DM") eq "1");
	assert(oconv(14276,"DMA") eq "JANUARY");
	assert(oconv(14276,"DW") eq "3");
	assert(oconv(14276,"DWA") eq "WEDNESDAY");
	assert(oconv(14276,"DY") eq "2007");
	assert(oconv(14276,"DY2") eq "07");
	assert(oconv(14276,"D2Y") eq "07 JAN 31");
	assert(oconv(14276,"D5Y") eq "02007 JAN 31");
	assert(oconv(14276,"DD") eq "31");
	assert(oconv(14276,"DL") eq "31");
	assert(oconv(14276,"DQ") eq "1");
	assert(oconv(14276,"DJ") eq "31");

	var feb29_2004=13209;//iconv("29 FEB 2004","D");
	assert(oconv(feb29_2004,"DL") eq "29");

	assert(oconv(14591,"D") eq "12 DEC 2007");
	assert(oconv(14591,"D2/") eq "12/12/07");
	assert(oconv(14591,"D2-") eq "12-12-07");
	assert(oconv(14591,"D-") eq "12-12-2007");
	assert(oconv(14591,"D2-") eq "12-12-07");
	assert(oconv(14591,"DJ") eq "346");
	assert(oconv(14591,"DM") eq "12");
	assert(oconv(14591,"DMA") eq "DECEMBER");
	assert(oconv(14591,"DW") eq "3");
	assert(oconv(14591,"DWA") eq "WEDNESDAY");
	assert(oconv(14591,"DY") eq "2007");
	assert(oconv(14591,"DQ") eq "4");
	assert(oconv(14591,"DD") eq "12");
	assert(oconv(14591,"DL") eq "31");

	//check times around noon and midnight round trip ok
	for (var ii=0; ii<=61 ; ++ii)
		//assert(var(ii).output("START=").oconv("MTHS").output(" OCONV=").iconv("MTHS").outputl(" ICONV=") eq ii);
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);
	for (var ii=43200-61; ii<=43200+61 ; ++ii)
		assert(var(ii).oconv("MTHS").iconv("MTHS") eq ii);

	//test that some random times iconv/oconv roundtrip ok
	initrnd(1000);
	var timex;
	for (int ii=1; ii<1000; ++ii) {
		timex=rnd(18600);
//		timex.oconv("MTHS").output(" ").iconv("MTHS").outputl(" ");
		assert(timex.oconv("MTHS").iconv("MTHS") eq timex);
	}

	//oconv(46622,"MTH").outputl("oconv 46622 MTH is" );
	assert(oconv(46622,"MTH") eq "12:57PM");

	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MT") eq "17:35");

	assert(oconv(0,"MTH") eq "12:00AM");
	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MTH") eq "05:35PM");
	assert(oconv(31653,"MTS") eq "08:47:33");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTHS") eq "05:35:06PM");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTS.") eq "17.35.06");
	assert(oconv(63306,"MTh") eq "17h35");

	assert(oconv(61201,"MT") eq "17:00");
	assert(oconv(61201,"MTS") eq "17:00:01");
	assert(oconv(61201,"MTH") eq "05:00PM");
	assert(oconv(61201,"MTHS") eq "05:00:01PM");

	var time2=43261;
	assert(time2.oconv("MT") eq "12:01");
	assert(time2.oconv("MTH") eq "12:01PM");
	assert(time2.oconv("MTS") eq "12:01:01");
	assert(time2.oconv("MTSH") eq "12H01H01");
	assert(time2.oconv("MTx") eq "12x01");
	assert(time2.oconv("MTHx") eq "12x01PM");
	assert(time2.oconv("MTSx") eq "12x01x01");
	assert(time2.oconv("MTSHx") eq "12H01H01");

	assert(oconv(FM ^ L"\x0035","HEX4") eq "00FE0035");
	assert(oconv(FM,"HEX4") eq "00FE");

	printl(osdir(SLASH));

	//root directories

	//check one step multilevel subfolder creation (requires boost version > ?)
	var topdir1=SLASH^"exodus544";
	var topdir1b=topdir1^"b";
	var subdir2=topdir1^SLASH^"abcd";
	var subdir2b=topdir1b^SLASH^"abcd";

	//try to remove any old versions (subdir first to avoid problems)
	osrmdir(topdir1b,true);
	osrmdir(topdir1);
	osrmdir(subdir2b,true);
	osrmdir(subdir2);

	assert(osmkdir(subdir2));

	printl("\nCheck CANNOT rename multilevel root folders");
	assert(not osrename(topdir1,topdir1b));

	printl("\nCheck CANNOT force delete root folders");
	assert(not osrmdir(topdir1,true));
	printl();

	//check can remove root folders one by one without force
	assert(osrmdir(subdir2));
	assert(osrmdir(topdir1));

	//printl(osdir("c:\\config.sys"));

	//relative directories ie not-root
	var tempdir="exotemp746";
	if (osdir(tempdir))
		assert(osrmdir(tempdir,true));

	//check mkdir
	assert(osmkdir(tempdir));
	assert(osdir(tempdir));
	assert(not osmkdir(tempdir));

	//check rmdir
	assert(osrmdir(tempdir));
	assert(not osdir(tempdir));

	//check writing a 1Mb file
	//restrict to ascii characters so size on disk=number of characters in string
	//also restrict to size 1 2 4 8 16 etc
	//var str1=L"1234ABC\x0160";//Note: you have to prefix strings with L if you want to put multibyte hex chars
	var str1="1234ABCD";
	var filesize=1024*1024;
	assert(osmkdir(tempdir));
	var tempfilename=tempdir^SLASH^"temp1";
	assert(oswrite(str(str1,filesize/len(str1)),tempfilename));
	var filedate=date();
	assert(osfile(tempfilename));
	var info=osfile(tempfilename);
	assert(info.extract(1) eq filesize);
	assert(info.extract(2) eq filedate);

	//check copying to a new file
	var tempfilename2=tempfilename^2;
	if (osfile(tempfilename2))
		assert(osdelete(tempfilename2));
	assert(oscopy(tempfilename,tempfilename2));
	assert(osfile(tempfilename2) eq info);

	//check renaming
	var tempfilename3=tempfilename^3;
	assert(osrename(tempfilename2,tempfilename3));
	assert(osfile(tempfilename3) eq info);

	//check force delete of subdirectory
	assert(osrmdir(tempdir,true));

	var x;
	var y;
	x="0";
	y=date();
	date().oconv("D").outputl("Date is:");
	time().oconv("MTS").outputl("Time is:");
	timedate().outputl("Time and Date is:");
	//assert( not (x > y) );

	assert(var(1000).oconv("MD20P,") eq "1,000.00");

	assert(var("0")<var(".5"));

	var ss;
	printl("sizeof");
	printl("char:   ",(int)sizeof(char));
	printl("wchar_t:",(int)sizeof(wchar_t));
	printl("string: ",(int)sizeof(std::string));
	printl("wstring:",(int)sizeof(std::wstring));
	printl("int:    ",(int)sizeof(int));
	printl("double: ",(int)sizeof(double));
	printl("var:    ",(int)sizeof(var));

	var tconv=FM.oconv("T#20");
	tconv="xxxxx/xxxxx xxx" ^ FM ^ "xx";
	tconv=tconv.oconv("T#8");

	//test regular expression
	var exp="(\\d{4}[- ]){3}\\d{4}";
	var res=var("1247-1234-1234-1234").match(exp,"r");

	//test redimensioning
	dim aaa(10);
	aaa.redim(20,30);

	var sentence=sentence();

    //wcout<<"main()"<<endl;
    //MVSystem mvsystem;
    //mvsystem.run();
    //wcout<<"exit"<<endl;

//    var xx="XXX";
//  xx.inverter();

	var dividend=100;
	assert(mod(dividend,30) eq 10);

	var env=osgetenv("");
	osgetenv("PATH");
	osgetenv("HOME");
	env="Steve";
	env.ossetenv("XYZ");
	osgetenv("XYZ");

//	var().debug();
//	var xx=xx.substr(1,1);

//experiment with some syntactic sugar
#define on ,
#define from ,

	var temprecord;
	var tempfilename0="tempfile";
	assert(oswrite("123" on tempfilename0));
	assert(osfile(tempfilename0));
	assert(osread(temprecord from tempfilename0));
	assert(temprecord eq "123");
	assert(osdelete(tempfilename0));
	assert(not osfile(tempfilename0));

//	var().stop();

// cannot connectlocal in main and thread until pipes are numbered
//	tss_environmentns.reset(new int(0));
//	tss_environmentns.reset(0);

//	if (!init_thread(0))
//		abort("Couldnt init thread 0");

	var filenames=listfiles();
	var indexnames=listindexes("test_people");

	/*
	var().begin();
	var testfile;
	if (!testfile.open("TESTFILE"))
		testfile.createfile("TESTFILE");
	var record1=var("x").str(300);
	var started=var().ostime();
	int nn=10000;
	if (0)
	{
		for (int ii=1;ii<nn;++ii)
		{
			if (!(ii%10000)) cout<<"Written "<<ii<<endl;
			record1.insertrecord(testfile,ii);
		}
	}
	var stopped=var().ostime();
	wcout<<(stopped-started)/nn;

	started=var().ostime();
	for (int ii=1;ii<nn;++ii)
	{
		if (!(ii%10000)) cout<<"Read "<<ii<<endl;
		record1.read(testfile,ii);
	}

	var().end();

	stopped=var().ostime();
	printl((stopped-started)/nn*1000000);

	wcin>>nn;

	//	testfile.deletefile();

	*/

//	var().connectlocal("");

	var filenames2="JOBS";
	filenames2^=FM^"PRODUCTION_ORDERS";
	filenames2^=FM^"PRODUCTION_INVOICES";
	filenames2^=FM^"COMPANIES";
	filenames2^=FM^"BRANDS";
	filenames2^=FM^"CLIENTS";
	filenames2^=FM^"VEHICLES";
	filenames2^=FM^"SUPPLIERS";
	filenames2^=FM^"CURRENCIES";
	filenames2^=FM^"MARKETS";
	filenames2^=FM^"ADS";

	write("F"^FM^0^FM^"Currency Code"^FM^FM^FM^FM^FM^FM^"L"^"10","DICT_CURRENCIES","CURRENCY_CODE");
	write("F"^FM^1^FM^"Currency Name"^FM^FM^FM^FM^FM^FM^"T"^"20","DICT_CURRENCIES","CURRENCY_NAME");
	write("F"^FM^1^FM^"Market Code"^FM^FM^FM^FM^FM^FM^"L"^"10","DICT_MARKETS","CODE");
	write("F"^FM^1^FM^"Market Name"^FM^FM^FM^FM^FM^FM^"T"^"20","DICT_MARKETS","NAME");

	printl();
	var nfiles=dcount(filenames2,FM);
	for (int ii=1;ii<=nfiles;++ii) {
		var filename=filenames2.extract(ii);

		if (not open(filename, tempfile)) {
			printl("creating "^filename);
			assert(createfile(filename));
		}

		if (not open("dict_"^filename, tempfile)) {
			assert(createfile("dict_"^filename));
			printl("creating dict_"^filename);
		}

	}
//	var("x:y:z:").dcount(":").outputl();
//	var().stop();

	var ads;
	if (!ads.open("ADS"))
	{
		var().createfile("ADS");
		if (!ads.open("ADS"))
			printl("Cannot create ADS");
			//abort("Cannot create ADS");
	}

	var dictrec="";
	dictrec.replacer(1,"F");
	dictrec.replacer(2,"3");
	dictrec.replacer(3,"Brand Code");
	if (not dictrec.write("DICT_ADS","BRAND_CODE"))
		printl("cannot write dict_ads, BRAND_CODE");
	if (not ads.createindex("BRAND_CODE")) {
//		printl("Creating ADS BRAND_CODE Index");
		printl("Index creation failed");
	}
	if (not ads.deleteindex("BRAND_CODE")) {
		printl("Deleting ADS BRAND_CODE Index");
		printl("Index deletion failed");
	}

//DBTRACE=1;
	var changelog, locks, users, accessible_columns;
	if (not open("CHANGELOG",changelog))
								assert(createfile("CHANGELOG"));
	if (not open("LOCKS",locks))
								assert(createfile("LOCKS"));
	if (not open("USERS",users))
								assert(createfile("USERS"));
    if (not open("ACCESSIBLECOLUMNS",accessible_columns))
								assert(createfile("ACCESSIBLECOLUMNS"));
	var market;
	market.read("MARKETS","PAN");

//	var("").select("MARKETS","WITH CURRENCY_NAME = '' AND WITH AUTHORISED");
//	var("").selectrecord("MARKETS","WITH AUTHORISED");
//	var("").select("ADS","WITH AUTHORISED");
//	ads.select("ADS","BY MARKET_CODE WITH MARKET_CODE 'BAH'");
//	ads.selectrecord("ADS","BY MARKET_CODE");
//	var().selectrecord("ADS");
//	var("").select("SCHEDULES","WITH AUTHORISED");
//	var("").select("SCHEDULES","");
	//MvLibs mvlibs;
	var key;
	int ii=0;
//	cin>>ii;
	var record;
	if (ads.selectrecord("SELECT ADS")) {
		while (ii<3&&ads.readnextrecord(key,record))
		{
			++ii;
			if (!(ii%10000))
				printl(" ",key);
			if (record.lcase().index("QWEQWE"))
				print("?");

		}
	}
	clearselect();

    printl("Shutting down ...");

    return 0;
}

programexit()
