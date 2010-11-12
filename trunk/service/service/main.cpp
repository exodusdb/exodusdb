/* Copyright (c) 2007 Stephen John Bush - see LICENCE.TXT*/

//main is a fairly extensive testbed of exodus
//main sets up several thread of a server that listens for request (via files), read/write the database or calls libs and responds.

//this program is dependent on boost to do threads - but threads will be added to exodus directly
//exodus is designed to be threadsafe (a few global variables are not threadsafe and need coordinated access)

//#include <unicode/ustring.h>
//gregory
//"visual leak detector" debugger
//#include <vld.h>

#include <iostream>
#include <vector>

//using namespace std;

//BOOST thread library minimal examples http://www.ddj.com/dept/cpp/184401518
//and boost\boost_1_34_0\libs\thread\example
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

//used to prevent threads overlapping cout operations
boost::mutex io_mutex;
///use like this (is rather slow)
//{
// boost::mutex::scoped_lock lock(io_mutex);
// wcout<<"xyz";
//}

//#include <exodus/mv.h>
//#include <exodus/exodus.h>
#include <exodus/program.h>
#include "server.h"

DLL_PUBLIC
boost::thread_specific_ptr<int> tss_environmentns;

using namespace exodus;

//#include <exodus/mvenvironment.h>
#include "mvwindow.h"

//#include "Definition.h"
//#include "Market.h"

/*
http://www.postgresql.org/docs/8.2/interactive/libpq-pgpass.html
The file .pgpass in a user's home directory or the file referenced by PGPASSFILE can contain passwords to be used if the connection requires a password (and no password has been specified otherwise). On Microsoft Windows the file is named %APPDATA%\postgresql\pgpass.conf (where %APPDATA% refers to the Application Data subdirectory in the user's profile).

This file should contain lines of the following format:

hostname:port:database:username:password
*/

//used to provide thread specific data like global data
//boost::thread_specific_ptr<var> value;
//use like this (ie gets a pointer) is very fast
//value.reset(new var(0)); // initialize the thread's storage
//var& xx = *value.get();
//xx=xx+1;

bool init_thread(const int environmentn)
{
		//set this threads environment number
		//some worker threads might be created with the same environment number
		setenvironmentn(environmentn);

		//create a new environment for the thread
		global_environments[environmentn]=new MvEnvironment;

		//and get a reference to it
		MvEnvironment& env=*global_environments[environmentn];

		//and init it
		if (!env.init(environmentn))
			return false;

		//create a new window common for the thread
		//the window structure needs access to the environment so it is part of the contruction
        //tss_wins.reset(new MvWindow(env));
		global_wins[environmentn]=new MvWindow(env);

        //mvlibs.set("DEFINITIONS",new Definition);
        //mvlibs.set("MARKETS",new Market);		

		return true;

}

class MVThread
{
public:

	//CONSTRUCTOR
    //use a one parameter contructor to pass the thread data in
	MVThread(int environmentn) : environmentn(environmentn)
	{
		boost::mutex::scoped_lock lock(io_mutex);
		exodus::printl("MVThread::ctor ",environmentn);

		setenvironmentn(environmentn);
	}

	//DESTRUCTOR
	virtual ~MVThread()
	{
		boost::mutex::scoped_lock lock(io_mutex);
		printl("MVThread::dtor ",environmentn);
	}

    //MvLib
	//declare and define the MvLib that is the thread
	void operator()()
	{

		{
			boost::mutex::scoped_lock lock(io_mutex);
			printl("MVThread::operator() start ",environmentn);
		}

		init_thread(environmentn);

        Server mvs1;
        mvs1.run();

        /*
    //var aa=environmentn;
		//var bb=environmentn;
		value.reset(new var(0));
		for (int i = 0; i < 1000000; ++i)
		{
			//test thread specific storage speed
			var& xx = *value.get();
			xx++;

			//debug one thread
			//if (i eq 5&&environmentn eq 1) i=i/(i-i);

			//ensure single threaded cout
			//boost::mutex::scoped_lock lock(io_mutex);
			printl("thread=",environmentn,": i=", i," ");

		}
        */
		{
			boost::mutex::scoped_lock lock(io_mutex);
			printl("MVThread::operator() stop ", environmentn);
		}
	};

private:
	int environmentn;
};

/* MVConnection holds and manages a connection to a database
*
*/
class MVConnection
{
private:
};

//programinit()
class ExodusProgram : public ExodusProgramBase {

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

	assert(unquote("\"This is quoted?\"") eq "This is quoted?");

	//verify that exodus catches c++ defect at runtime
/*	try {
		var abc=abc+1;
		//should never get here because above should cause a runtime undefined error
		assert(false);
	}
	catch (MVUndefined mve) {
		mve.description.outputl();
	}
	catch (...) {
		//should never get here because above should cause a runtime error
		assert(false);
	}
*/
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

	var errmsg;
	//if (not createdb("steve",errmsg))
	//	errmsg.outputl();
	//if (not deletedb("steve",errmsg))
	//	errmsg.outputl();

	deletefile("USERS");
	deletefile("dict_XUSERS");

	createfile("XUSERS");
	createfile("DICT_XUSERS");

	write("S"^FM^FM^"Age in Days"^FM^FM^FM^FM^FM^FM^"R"^FM^"10","DICT_XUSERS","AGE_IN_DAYS");
	write("S"^FM^FM^"Age in Years"^FM^FM^FM^FM^FM^FM^"R"^FM^"10","DICT_XUSERS","AGE_IN_YEARS");

	write("1","XUSERS","1");
	write("2","XUSERS","2");

	if (not selectrecord("SELECT XUSERS WITH AGE_IN_DAYS GE 0 AND WITH AGE_IN_YEARS GE 0"))
		printl("Failed to Select");

	DICT="dict_XUSERS";
	while (readnextrecord(RECORD,ID))
	{
		print("ID=",ID, " RECORD=",RECORD);
		print(" AGE_IN_DAYS=",calculate("AGE_IN_DAYS"));
		printl(" AGE_IN_YEARS=",calculate("AGE_IN_YEARS"));
	}

	var spac1="  xxx  xxx  ";
	trimmer(spac1).quote().outputl();

	timedate().outputl();

	dim a9;
	var a10;
	matparse("xx"^FM^"bb",a9).outputl();
	var a11=matunparse(a9).outputl();

	var r[2];

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
		printl();
	}

	a8=a7;

	for (int ii=1;ii<=4;++ii) {
		for (int jj=1;jj<=5;++jj)
			a8(ii,jj).outputt("=");
		printl();
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
	replace(da1,3,"x").outputl();//or this
	replace(da1,3,3,"x").outputl();//or this
	replace(da1,3,3,3,"x").outputl();//or this
	insert(da1,3,"x").outputl();//or this
	insert(da1,3,3,"x").outputl();//or this
	insert(da1,3,3,3,"x").outputl();//or this

	//replacement
	da1(2)="x";//sadly this compile and runs without error but does nothing!

	da1.replacer(3,"x");//this is the right way to do it. replace field2
	da1.replacer(3,3,"x");//this is the right way to do it. replace field2
	da1.replacer(3,3,3,"x");//this is the right way to do it. replace field2

	da1.inserter(3,"x");//this is the right way to do it. replace field2
	da1.inserter(3,3,"x");//this is the right way to do it. replace field2
	da1.inserter(3,3,3,"x");//this is the right way to do it. replace field2

	replacer(da1,3,"x").outputl();//or this
	replacer(da1,3,3,"x").outputl();//or this
	replacer(da1,3,3,3,"x").outputl();//or this

	inserter(da1,3,"x").outputl();//or this
	inserter(da1,3,3,"x").outputl();//or this
	inserter(da1,3,3,3,"x").outputl();//or this

	var tempfile;
	// This file should be in the same directory, where project file is located, like
	//		C:\Exodus\exodusdb\service\service\testsort.cpp
	if (osopen("testsort.cpp",tempfile))	// opening existing file
	{
		printl("opened");
		var testsortbytes = osbread( tempfile, 0, 256);
		printl("first 256 bytes of the file :\n" ^ testsortbytes);
	}
	else
		printl("not opened");

	//math.h seems to have been included in one of the boost or other special headers
	//in this main.cpp file and that causes confusion between math.h and exodus.h sin() and other functions.
	//we resolved the issue here by being specific about the namespace
	exodus::sin(30.0000000001).outputl("sin(30.0)=");
	exodus::cos(30.0).outputl("cos(30.0)=");
	exodus::tan(30.0).outputl("tan(30.0)=");
	exodus::atan(30.0).outputl("atan(30.0)=");

	exodus::abs(30.0).outputl("abs(30.0)=");
	exodus::abs(-30.0).outputl("abs(-30.0)=");
	exodus::pwr(10,3).outputl("pwr(10,3)=");
	exodus::exp(1).outputl("exp(1)=");
	exodus::loge(1).outputl("loge(1)=");
	exodus::loge(10).outputl("loge(10)=");
	exodus::sqrt(100).outputl("sqrt(100)=");

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

	oconv(1234,"MD20P").outputl();
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

	double d2=1;
	double d3=2;
	if (d2<d3)
		d2=d3;
	union {double d1; char chars[8];};

	for (d1=-5;d1<=5;++d1) {
		print(d1," ");
		for (int partn=0;partn<sizeof(chars);++partn)
			//var(chars[partn]).oconv("HEX").output();
			std::cout << std::hex << int(chars[partn]) << " " ;
		printl();
	}

	var("xyz").outputl();
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
	assert(rnd(999).outputl() eq 777);
	assert(iconv("23 59 59","MT").outputl() eq 86399);
	assert(iconv("xx11yy12zz13P","MT").outputl() eq 83533);
	assert(iconv("24 00 00","MT").outputl() eq "");

	//http://www.regular-expressions.info/examples.html
	assert(swap("Steve Bush Bash bish","B.","Ru","ri").outputl() eq "Steve Rush Rush Rush");

	if (sizeof(wchar_t) eq 2)
	{
		//ucs-16 "fake utf16" on windows
		assert(oconv("Aa019KK","HEX").outputl() eq "00410061003000310039004B004B");
		assert(var("00410061003000310039004B004B").iconv("HEX").outputl() eq "Aa019KK");

	}
	else if (sizeof(wchar_t) eq 4)
	{
		assert(oconv("Aa019KK","HEX").outputl() eq "00000041000000610000003000000031000000390000004B0000004B");
		assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX2").outputl() eq "Aa019KK");

	} 

	assert(oconv("Aa019KK","HEX2").outputl() eq "41613031394B4B");
	assert(oconv("Aa019KK","HEX4").outputl() eq "00410061003000310039004B004B");
	assert(oconv("Aa019KK","HEX8").outputl() eq "00000041000000610000003000000031000000390000004B0000004B");
	assert(var("41613031394B4B").iconv("HEX2").outputl() eq "Aa019KK");
	assert(var("00410061003000310039004B004B").iconv("HEX4").outputl() eq "Aa019KK");
	assert(var("00000041000000610000003000000031000000390000004B0000004B").iconv("HEX8").outputl() eq "Aa019KK");

	//doesnt accept FMs etc yet
	//assert(var("FF"^FM^"00").iconv("HEX").outputl() eq ("00FF"^FM^"00FF"));
	assert(var("FF"^FM^"00").iconv("HEX2").oconv("HEX2").outputl() eq "");
	//anything invalid returns empty string
	assert(var("XF").iconv("HEX").oconv("HEX").outputl() eq "");

	var time1=var("10:10:10").iconv("MT");
	assert(var("abcabdef").trim("abef").outputl() eq "cbd");
	assert(var("abcabdef").trimf("abef").trimb("abef").outputl() eq "cabd");

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

	space(-11);
	var("x").str(-7);

	var xyz;
	//xyz=xyz;
	//test catching MVexceptions
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
	osopen("/ADECOMT.TXT",filehandle);
	oconv(0,"D5").outputl();
	//check empty conversion
	oconv("xxx","").outputl("xxx = ");
	assert(oconv("xxx","") eq "xxx");

	oconv("",				"MX").outputl(" = ");
	oconv(" ",				"MX").outputl("  = ");
	oconv(0,				"MX").outputl("0 = ");
	oconv(-0,				"MX").outputl("0 = ");
	oconv("X",				"MX").outputl("X = ");
	oconv("-1.5",			"MX").outputl("FFFFFFFFFFFFFFFE = ");
	oconv("1.5",			"MX").outputl("2 = ");
	oconv("20" ^ FM ^ 255,	"MX").outputl("14" ^FM^ "FF = ");

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

	assert(oconv(9649,"D") eq "01 JUN 1994");
	assert(oconv(9649,"D2") eq "01 JUN 94");
	assert(oconv(9649,"D4") eq "01 JUN 1994");
	assert(oconv(9649,"D/") eq "06/01/1994");
	assert(oconv(9649,"D ") eq "06 01 1994");
	assert(oconv(9649,"D2/") eq "06/01/94");
	assert(oconv(9649,"D2-") eq "06-01-94");
	assert(oconv(9649,"D/") eq "06/01/1994");
	assert(oconv(9649,"D/E") eq "01/06/1994");
	assert(oconv(9649,"D2 E") eq "01 06 94");
	assert(oconv(9649,"D S") eq "1994 06 01");
	assert(oconv(9649,"DM") eq "6");
	assert(oconv(9649,"DMA") eq "JUNE");
	assert(oconv(9649,"DW") eq "3");
	assert(oconv(9649,"DWA") eq "WEDNESDAY");
	assert(oconv(9649,"DY") eq "1994");
	assert(oconv(9649,"DY2") eq "94");
	assert(oconv(9649,"D2Y") eq "94 JUN 01");
	assert(oconv(9649,"D5Y") eq "01994 JUN 01");
	assert(oconv(9649,"DD") eq "1");
	assert(oconv(9649,"DQ") eq "2");
	assert(oconv(9649,"DJ") eq "152");
	var feb29_2004=13209;//iconv("29 FEB 2004","D");
	oconv(feb29_2004,"DL").outputl();
	assert(oconv(feb29_2004,"DL") eq "29");

	assert(oconv(10939,"D") eq "12 DEC 1997");
	assert(oconv(10939,"D2/") eq "12/12/97");
	assert(oconv(10939,"D2-") eq "12-12-97");
	assert(oconv(10939,"D-") eq "12-12-1997");
	assert(oconv(10939,"D2-") eq "12-12-97");
	assert(oconv(10939,"DJ") eq "346");
	assert(oconv(10939,"DM") eq "12");
	assert(oconv(10939,"DMA") eq "DECEMBER");
	assert(oconv(10939,"DW") eq "5");
	assert(oconv(10939,"DWA") eq "FRIDAY");
	assert(oconv(10939,"DY") eq "1997");
	assert(oconv(10939,"DQ") eq "4");

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

	oconv(31653,"MT").outputl();

	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MT") eq "17:35");
	printl(oconv(0,"MTH"));
	assert(oconv(0,"MTH") eq "12:00AM");
	assert(oconv(31653,"MT") eq "08:47");
	assert(oconv(63306,"MTH") eq "05:35PM");
	assert(oconv(31653,"MTS") eq "08:47:33");
	assert(oconv(63306,"MTS") eq "17:35:06");
	printl(oconv(63306,"MTHS"));
	assert(oconv(63306,"MTHS") eq "05:35:06PM");
	assert(oconv(63306,"MTS") eq "17:35:06");
	assert(oconv(63306,"MTS.") eq "17.35.06");
	assert(oconv(63306,"MTh") eq "17h35");

	assert(oconv(61201,"MT") eq "17:00");
	assert(oconv(61201,"MTS") eq "17:00:01");
	assert(oconv(61201,"MTH") eq "05:00PM");
	assert(oconv(61201,"MTHS") eq "05:00:01PM");

/*
MT 31653 08:47
MT 63306 17:35
MTH 0 12:00am
MTH 31653 08:47am
MTH 63306 05:35pm
MTS 31653 08:47:33
MTS 63306 17:35:06
MTHS 63306 05:35:06pm
MTS. 63306 17:35:06
MT'h' 63306 17h35
*/
	printl(time().oconv("MT"));
	printl(time().oconv("MTH"));
	printl(time().oconv("MTS"));
	printl(time().oconv("MTSH"));
	printl(time().oconv("MTx"));
	printl(time().oconv("MTHx"));
	printl(time().oconv("MTSx"));
	printl(time().oconv("MTSHx"));

	var hexx=oconv(FM ^ L"\x0035","HEX");
	hexx=oconv(FM,"HEX");

	printl(osdir("/"));
	osrmdir("xxxyzz");
	osmkdir("xxxyzz");
	osrmdir("xxxy");
	printl(var("/xyz/aaa").osmkdir());
	printl(var("/xyz").osrmdir());
	outputl(osrename("/xyz","/xyzz"));
	printl(var("/xyzz").osrmdir(true));

	printl(osdir("c:\\config.sys"));

	outputl(oscopy("/adagency7.exe","/adagency777.exe"));
	outputl(osrename("/adagency777.exe","/adagency888.exe"));

	var x;
	var y;
	x="0";
	y=date();
	date().oconv("D").outputl();

	time().oconv("MTS").outputl();
	timedate().outputl();
	//assert( not (x > y) );

	var(1000).oconv("MD20P,").outputl();

	printl(var("0")<var(".5"));
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

	var exp="(\\d{4}[- ]){3}\\d{4}";
	var res=var("1247-1234-1234-1234").match(exp,"r");
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
	printl(mod(dividend,30));

	var env=osgetenv("");
	osgetenv("PATH");
	osgetenv("HOME");
	env="Steve";
	env.ossetenv("XYZ");
	osgetenv("XYZ");

//	var().debug();
//	var xx=xx.substr(1,1);

#define on ,
#define from ,

	var tempfilename="tempfile";
	oswrite("123" on tempfilename);
	var tempdata;
	if (!osread(tempdata from tempfilename))
		abort("Failed to osread after oswrite");
	if (tempdata!="123")
		abort("Failed to osread after oswrite");
	osdelete(tempfilename);
	if (osfile(tempfilename))
		abort("Failed to osdelete tempfile");

//	var().stop();

// cannot connectlocal in main and thread until pipes are numbered
	tss_environmentns.reset(new int(0));
//	tss_environmentns.reset(0);

	if (!init_thread(0))
		abort("Couldnt init thread 0");

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
	filenames2^=FM^"PRODUCTION.ORDERS";
	filenames2^=FM^"PRODUCTION.INVOICES";
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
		print(filename);
		if (!var().open(filename))
			var().createfile(filename);
		else
			print(", data ok");
		if (!var().open("DICT."^filename))
			var().createfile("DICT."^filename);
		else
			print(", dict ok");
		printl();
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
		printl("cannot write brand_code dict");
	printl("Creating ADS BRAND_CODE Index");
	if (not ads.createindex("BRAND_CODE"))
		printl("Index creation failed");
	printl("Deleting ADS BRAND_CODE Index");
	if (not ads.deleteindex("BRAND_CODE"))
		printl("Index deletion failed");

	if (!var().open("CHANGELOG"))
								var().createfile("CHANGELOG");
	if (!var().open("LOCKS"))
								var().createfile("LOCKS");
	if (!var().open("USERS"))
								var().createfile("USERS");
    if (!var().open("ACCESSIBLE_COLUMNS"))
								var().createfile("ACCESSIBLE_COLUMNS");

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
	printl("readnexting:");
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

	print("Press Enter to start threads ... ");
	//cin.get();

	/*
	//create a temporary using a constructor and its operator() will be called
	//boost::thread thread1(MVThread(1));
	//boost::thread thread2(MVThread(2));
	//boost::thread thread3(MVThread(2));
	//thread1.join();
	//thread2.join();
	//thread3.join();

	var xx="CCAA"^FM^SVM^"B";
	xx=xx.extract(0,0,-1);

	var started=var().ostime();
	var aa;
	xx=100.1;
	int iii=100;
	for (int ii=0 ; ii<10000000 ; ++ii)
	{
		//iii=iii*iii;
		xx=xx*xx;
	}
	wcout<<endl<<"iii="<<iii<<endl;
	wcout<<endl<<"xx="<<xx<<endl;
	var started=var().ostime();
	var stopped=var().ostime();
	wcout<<(stopped)<<" "<<(started)<<endl;
	wcout<<" "<<(stopped-started);
	int ii;
	cin>>ii;
	*/
	/*
	int ii;
	assert(var().selftest());
	cin>>ii;
	exit(0);
	*/
	
	printl("Starting ",NTHREADS," threads");
	boost::thread_group threads;
	for (var ii = 0; ii < NTHREADS; ++ii)
	{
		outputl("Creating thread " ^ ii);
		//start from environment number 1 so that main thread has its own environment
		threads.create_thread(MVThread(ii+1));
	}
	threads.join_all();

    stop("Shutting down ...");
    
	return 0;
}

programexit()