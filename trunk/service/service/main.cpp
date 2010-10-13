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

using namespace std;

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

#include <exodus/mv.h>
#include "server.h"

DLL_PUBLIC
boost::thread_specific_ptr<int> tss_environmentns;

using namespace exodus;

#include <exodus/mvenvironment.h>
#include "mvwindow.h"

#include "Definition.h"
#include "Market.h"

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
cout<<"111";
		//create a new window common for the thread
		//the window structure needs access to the environment so it is part of the contruction
        //tss_wins.reset(new MvWindow(env));
		global_wins[environmentn]=new MvWindow(env);
cout<<"222";

        mvlibs.set("DEFINITIONS",new Definition);
cout<<"333";
        mvlibs.set("MARKETS",new Market);		
cout<<"444";

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
		wcout<<"MVThread::ctor "<<environmentn<<endl;

		setenvironmentn(environmentn);
	}

	//DESTRUCTOR
	virtual ~MVThread()
	{
		boost::mutex::scoped_lock lock(io_mutex);
		wcout<<"MVThread::dtor "<<environmentn<<endl;
	}

    //MvLib
	//declare and define the MvLib that is the thread
	void operator()()
	{

		{
			boost::mutex::scoped_lock lock(io_mutex);
			wcout<<"MVThread::operator() start "<<environmentn<<endl;
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
			//if (i==5&&environmentn==1) i=i/(i-i);

			//ensure single threaded cout
			//boost::mutex::scoped_lock lock(io_mutex);
			//cout << "thread=" << environmentn << ": i=" << i << " " << endl;

		}
        */
		{
			boost::mutex::scoped_lock lock(io_mutex);
			wcout<<"MVThread::operator() stop "<<environmentn<<endl;
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

#include <exodus/exodus.h>

program()
{
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

	var errmsg;
	if (not var("CREATE DATABASE exodus WITH ENCODING='UTF8' OWNER=exodus;").sqlexec(errmsg))
		printl();

	oconv(1234,"MD20P").outputl();
	assert(var(10000).oconv("DY0")=="");

	var temp;
	if (not open("dict_test_symbolics",temp))
		createfile("dict_test_symbolics");
	write("F"^FM^0^FM^"ID","dict_test_symbolics","ID");
	write("S"^FM^FM^"Col1","dict_test_symbolics","SYMCOL1");

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
	assert(var(1000).oconv("MD80").outputl("1000 MD80->")=="1000.00000000");
	assert(var("31 JAN 2008").iconv("D")=="14641");

	assert(var("1/31/2008").iconv("D")==14641);
	assert(var("2008/1/31").iconv("DS")=="14641");

	assert(var("JAN/31/2008").iconv("D")=="14641");
	assert(var("2008/JAN/31").iconv("DS")=="14641");

	assert(var("31/1/2008").iconv("DE")=="14641");
	assert(var("31 1 2008").iconv("DE")=="14641");
	assert(var("31-1-2008").iconv("DE")=="14641");
	assert(var("31/JAN/2008").iconv("DE")=="14641");
	assert(var("JAN/31/2008").iconv("DE")=="14641");
	assert(var("29 FEB 2008").iconv("D")=="14670");

	assert(var("32/1/2008").iconv("DE")=="");
	assert(var("30/2/2008").iconv("DE")=="");
	assert(var("1/31/2008").iconv("DE")=="");

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

	std::wcout<< var("xyz");
	//var steve;steve.input(1);

	//catching errors - doesnt work now that backtrace aborts (to prevent system crashes ... maybe better solution is to trap in main()
	//var unass;
	//try{unass+1;}
	//catch(...){};

	//using mv dimensioned arrays
	//mv dimensioned arrays have a zero element that is
	//used in case either or both of the indexes are zero
	varray arr1(3), arr2(3,3);
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

	//using c arrays UNSAFE! USE STL VECTOR INSTEAD;
	var arrxxx[10];

	//can use int but not var for indexing c arrays
	int intx=0;
	arrxxx[intx]="x";
	var varx=0;
	//following will not compile on MSVC (g++ is ok) due to "ambiguous" due to using var for index element
	//arrxxx[varx]="y";
	arrxxx[int(varx)]="y";

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

	printl(_SENTENCE);
	var("xyz").substr(4,1).outputl();

	//TODO ensure isnum converts ints larger that the maximum int to FLOATS

	var subs="xyz";
	printl(subs.substr(-1));

	assert(oconv("a","L#3").outputl()=="a  ");
	assert(oconv("abc","L#3").outputl()=="abc");
	assert(oconv("abcd","L#3").outputl()=="abc");
	assert(oconv("a"^FM^"abc"^FM^"abcd","L#3").outputl()==("a  "^FM^"abc"^FM^"abc"));

	assert(oconv("a","R#3").outputl()=="  a");
	assert(oconv("abc","R#3").outputl()=="abc");
	assert(oconv("abcd","R#3").outputl()=="bcd");
	assert(oconv("a"^FM^"abc"^FM^"abcd","R#3").outputl()==("  a"^FM^"abc"^FM^"bcd"));

	assert(oconv("a","T#3").outputl()=="a  ");
	assert(oconv("abc","T#3").outputl()=="abc");
	assert(oconv("abcd","T#3").outputl()==("abc"^TM^"d  "));
	assert(oconv("a"^FM^"abc"^FM^"abcd","T#3").outputl()==("a  "^FM^"abc"^FM^"abc"^TM^"d  "));

	assert(oconv("a","L(0)#3").outputl()=="a00");
	assert(oconv("a","R(0)#3").outputl()=="00a");
	assert(oconv("a","T(0)#3").outputl()=="a00");
	assert(oconv("abcd","T(0)#3").outputl()==("abc"^TM^"d00"));

	initrnd(999);
	assert(rnd(999).outputl()==777);
	assert(iconv("23 59 59","MT").outputl()==86399);
	assert(iconv("xx11yy12zz13P","MT").outputl()==83533);
	assert(iconv("24 00 00","MT").outputl()=="");

	//http://www.regular-expressions.info/examples.html
	assert(swap("Steve Bush Bash bish","B.","Ru","ri").outputl()=="Steve Rush Rush Rush");
	//correct for utf16 windows
	if (sizeof(wchar_t)==2)
	{
		assert(oconv("Aa019KK","HEX").outputl()=="00410061003000310039004B004B");
		assert(var("00410061003000310039004B004B").iconv("HEX").outputl()=="Aa019KK");

		//doesnt accept FMs etc yet
		//assert(var("FF"^FM^"00").iconv("HEX").outputl()==("00FF"^FM^"00FF"));
		assert(var("FF"^FM^"00").iconv("HEX").oconv("HEX").outputl()=="");
		//anything invalid returns empty string
		assert(var("XF").iconv("HEX").oconv("HEX").outputl()=="");
	}

	var time1=var("10:10:10").iconv("MT");
	assert(var("abcabdef").trim("abef").outputl()=="cbd");
	assert(var("abcabdef").trimf("abef").trimb("abef").outputl()=="cabd");

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

	//assert(crop(VM ^ FM)=="");
	//assert(crop("xxx" ^ VM ^ FM)=="xxx");
	assert(crop("aaa" ^ VM ^ FM ^ "bbb")==("aaa" ^ FM ^ "bbb"));
	assert(crop("aaa" ^ VM ^ FM ^ "bbb")==("aaa" ^ FM ^ "bbb"));
	assert(crop("aaa" ^ FM ^ "bbb" ^ FM ^ VM ^ SM ^ SM ^ FM ^ "ddd")==("aaa" ^ FM ^ "bbb" ^ FM ^ FM ^ "ddd"));
	assert(crop("aaa" ^ FM ^ "bbb" ^ FM ^ VM ^ SM ^ SM ^ FM ^ RM ^ "ddd")==("aaa" ^ FM ^ "bbb" ^ RM ^ "ddd"));

	space(-11);
	var("x").str(-7);

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
//	if (var xx==x) {};
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
	assert(oconv("xxx","")=="xxx");

	oconv("",				"MX").outputl(" = ");
	oconv(" ",				"MX").outputl("  = ");
	oconv(0,				"MX").outputl("0 = ");
	oconv(-0,				"MX").outputl("0 = ");
	oconv("X",				"MX").outputl("X = ");
	oconv("-1.5",			"MX").outputl("FFFFFFFFFFFFFFFE = ");
	oconv("1.5",			"MX").outputl("2 = ");
	oconv("20" ^ FM ^ 255,	"MX").outputl("14" ^FM^ "FF = ");

	assert(oconv("","MX")=="");
	assert(oconv(" ","MX")==" ");
	assert(oconv(0,"MX")=="0");
	assert(oconv(-0,"MX")=="0");
	assert(oconv("X","MX")=="X");
	//assert(oconv("-1.5","MX")=="FFFFFFFFFFFFFFFE");
	assert(oconv("-1.5","MX")=="FFFFFFFE");
	assert(oconv("-1","MX")=="FFFFFFFF");
	assert(oconv("1.5","MX")=="2");
	assert(oconv("20" ^ FM ^ 255,"MX")==("14" ^FM^ "FF"));

	oconv("","D").outputl(" = ");
	oconv("X","D").outputl("X = ");
	oconv("-1.5","D").outputl("29 DEC 1967 = ");
	oconv("1.5","D").outputl("01 JAN 1968 = ");
	oconv("1.5" ^ FM ^ -1.5,"D").outputl("01 JAN 1968 FM 29 DEC 1967 = ");

	assert(oconv("","D")=="");
	assert(oconv("X","D")=="X");
	assert(oconv("-1.5","D")=="29 DEC 1967");
	assert(oconv("1.5","D")=="01 JAN 1968");
	assert(oconv("1.5" ^ FM ^ -1.5,"D")==("01 JAN 1968"^FM^"29 DEC 1967"));

	assert(oconv(9649,"D")=="01 JUN 1994");
	assert(oconv(9649,"D2")=="01 JUN 94");
	assert(oconv(9649,"D4")=="01 JUN 1994");
	assert(oconv(9649,"D/")=="06/01/1994");
	assert(oconv(9649,"D ")=="06 01 1994");
	assert(oconv(9649,"D2/")=="06/01/94");
	assert(oconv(9649,"D2-")=="06-01-94");
	assert(oconv(9649,"D/")=="06/01/1994");
	assert(oconv(9649,"D/E")=="01/06/1994");
	assert(oconv(9649,"D2 E")=="01 06 94");
	assert(oconv(9649,"D S")=="1994 06 01");
	assert(oconv(9649,"DM")=="6");
	assert(oconv(9649,"DMA")=="JUNE");
	assert(oconv(9649,"DW")=="3");
	assert(oconv(9649,"DWA")=="WEDNESDAY");
	assert(oconv(9649,"DY")=="1994");
	assert(oconv(9649,"DY2")=="94");
	assert(oconv(9649,"D2Y")=="94 JUN 01");
	assert(oconv(9649,"D5Y")=="01994 JUN 01");
	assert(oconv(9649,"DD")=="1");
	assert(oconv(9649,"DQ")=="2");
	assert(oconv(9649,"DJ")=="152");
	var feb29_2004=13209;//iconv("29 FEB 2004","D");
	oconv(feb29_2004,"DL").outputl();
	assert(oconv(feb29_2004,"DL")=="29");

	assert(oconv(10939,"D")=="12 DEC 1997");
	assert(oconv(10939,"D2/")=="12/12/97");
	assert(oconv(10939,"D2-")=="12-12-97");
	assert(oconv(10939,"D-")=="12-12-1997");
	assert(oconv(10939,"D2-")=="12-12-97");
	assert(oconv(10939,"DJ")=="346");
	assert(oconv(10939,"DM")=="12");
	assert(oconv(10939,"DMA")=="DECEMBER");
	assert(oconv(10939,"DW")=="5");
	assert(oconv(10939,"DWA")=="FRIDAY");
	assert(oconv(10939,"DY")=="1997");
	assert(oconv(10939,"DQ")=="4");

	oconv(31653,"MT").outputl();

	assert(oconv(31653,"MT")=="08:47");
	assert(oconv(63306,"MT")=="17:35");
	printl(oconv(0,"MTH"));
	assert(oconv(0,"MTH")=="12:00AM");
	assert(oconv(31653,"MT")=="08:47");
	assert(oconv(63306,"MTH")=="05:35PM");
	assert(oconv(31653,"MTS")=="08:47:33");
	assert(oconv(63306,"MTS")=="17:35:06");
	printl(oconv(63306,"MTHS"));
	assert(oconv(63306,"MTHS")=="05:35:06PM");
	assert(oconv(63306,"MTS")=="17:35:06");
	assert(oconv(63306,"MTS.")=="17.35.06");
	assert(oconv(63306,"MTh")=="17h35");

	assert(oconv(61201,"MT")=="17:00");
	assert(oconv(61201,"MTS")=="17:00:01");
	assert(oconv(61201,"MTH")=="05:00PM");
	assert(oconv(61201,"MTHS")=="05:00:01PM");

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
	printl("string: ",(int)sizeof(string));
	printl("wstring:",(int)sizeof(wstring));
	printl("int:    ",(int)sizeof(int));
	printl("double: ",(int)sizeof(double));
	printl("var:    ",(int)sizeof(var));

	var tconv=FM.oconv("T#20");
	tconv="xxxxx/xxxxx xxx" ^ FM ^ "xx";
	tconv=tconv.oconv("T#8");

	var exp="(\\d{4}[- ]){3}\\d{4}";
	var res=var("1247-1234-1234-1234").match(exp,"r");
	varray aaa(10);
	aaa.resize(20,30);

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
	wcout<<(stopped-started)/nn*1000000;

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
			abort("Cannot create ADS");
	}

	var dictrec="";
	dictrec.replacer(1,0,0,"F");
	dictrec.replacer(2,0,0,"3");
	dictrec.replacer(3,0,0,"Brand Code");
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
	MvLibs mvlibs;
	var key;
	int ii=0;
//	cin>>ii;
	var record;
	wcout<<"readnexting:"<<endl;
	if (ads.selectrecord("SELECT ADS")) {
		while (ii<3&&ads.readnextrecord(key,record))
		{
			++ii;
			if (!(ii%10000))
				wcout<<ii<<" ";
				wcout<<key<<endl;
			if (record.lcase().index("QWEQWE"))
				wcout<<"?";

		}
	}

	wcout<<L"Press Enter to start threads ... ";
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
    
}
