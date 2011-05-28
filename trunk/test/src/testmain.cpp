#include <exodus/program.h>
#include <cassert>

/*Ubuntu
 apt-cache search locale |grep -i greek
 greek turkish german
 apt-get install language-support-el language-support-tr language-support-de
*/

//non-ASCII unicode characters
var GreekQuestionMark		=L"\u037E";//GREEK QUESTION MARK (Punctuation)
var GreekCapitalGamma		=L"\u0393";//GREEK CAPITAL LETTER GAMMA (Letter) (Uppercase)
var GreekSmallGamma		=L"\u03B3";//GREEK SMALL LETTER GAMMA (Letter) (Lowercase)
var ArabicIndicDigitZero	=L"\u0660";//ARABIC-INDIC DIGIT ZERO (Decimal Digit)

var GreekSmallAlpha         =L"\u03B1";//GREEK SMALL LETTER ALPHA
var GreekSmallFinalSigma    =L"\u03C2";//GREEK SMALL LETTER FINAL SIGMA
var GreekSmallSigma         =L"\u03C3";//GREEK SMALL LETTER SIGMA
var GreekCapitalSigma       =L"\u03A3";//GREEK CAPITAL LETTER SIGMA
var GreekCapitalAlpha       =L"\u0391";//GREEK CAPITAL LETTER ALPHA
var Greek_sas               =GreekSmallSigma^GreekSmallAlpha^GreekSmallFinalSigma;
var Greek_SAS               =GreekCapitalSigma^GreekCapitalAlpha^GreekCapitalSigma;

var TurkishCapitalDottedI   =L"\u0130";
var TurkishSmallDotlessI    =L"\u0131";
var LatinSmallI             ="i";
var LatinCapitalI           ="I";

var GermanEszet             =L"\u00DF";//German

programinit()

function main()
{

	printl("----------------------------------------------");
	printl("Using Exodus library version:"^var().version());

	var errmsg;
	//if (not createdb("steve",errmsg))
	//	errmsg.outputl();
	//if (not deletedb("steve",errmsg))
	//	errmsg.outputl();

	{
		var data=L"\xFF\x30\x00\x32";
		//assert(len(data) eq 4);
	}
	
	{
		//make a string of first 256 (excluding 0 for the time being!)
		var data="";
		for (var ii=1;ii<=255;++ii)
		 data^=chr(ii);
		assert(len(data) eq 255);
		
		//check can write characters 1-255 out as bytes using C locale
		oswrite(data,"x.txt","C");
		assert(osfile("x.txt")(1) eq 255);

		//check can read in bytes as characters using C locale
		var data2;
		osread(data2,"x.txt","C");
		assert(data2 eq data);
	}
	
	//test oswrite and osbread utf8
	//following code works on win32 and linux64 (ubuntu 10.04)
	//contents of tempfile should end up as unicode bytes ce b3 ce a3
	{

		//greek lowercase gamma and uppercase sigma
		var greek2=L"\u03B3\u03A3";
		assert(greek2[1].seq()=947);
		assert(greek2[2].seq()=931);

		//output as utf8 to temp5.txt		
		var tempfilename5="temp5.txt";
		//greek2.outputl();
		assert(oswrite(greek2,tempfilename5,"utf8"));

		//open temp5.txt as utf8 for random access
		var tempfile;
		assert(osopen(tempfilename5,tempfile,"utf8"));

		//test reading from beyond end of file - returns ""
		//offset2 is BYTE OFFSET NOT CHARACTER OFFSET!!!
		var data,offset2;
		assert(data.osbread(tempfile,offset2=4,2) eq "");
		assert(data.osbread(tempfile,offset2=3,2) eq "");

		assert(data.osbread(tempfile,offset2=2,2) eq greek2[2]);
		assert(data.osbread(tempfile,offset2=2,1) eq greek2[2]);

		//read from non-first-byte of a multibyte utf8 character returns ""
		assert(data.osbread(tempfile,offset2=1,2) eq "");

		assert(data.osbread(tempfile,offset2=0,2) eq greek2);
		assert(data.osbread(tempfile,offset2=0,1) eq greek2[1]);
		
		printl("greek utf8 tested ok");
	}

	assert(oswrite("","temp1234.txt"));
	var offs=2;
	assert(osbwrite("78","temp1234.txt",offs));
//	abort("stopping");

	printl("testmain says 'Hello World!'");
	//assert(setxlocale("fr_FR.utf8"));
	//assert(setxlocale(1036));
	var xx3="1234.5678";
	printl(xx3+1);
	printl(oconv(xx3,"MD20P"));
	//input();
	//stop();

	//SQL tracing
	//DBTRACE=true;

	//save original locale
	//setxlocale("C");
	var locale0=getxlocale().outputl("Original Locale=");

	//windows locales Windows XP and Windows Server 2003
	//http://msdn.microsoft.com/en-us/goglobal/bb895996.aspx
	var english_us="";
	var german_standard="";
	var greek_gr="";
	var turkish_tr="";
	var english_us_locale;
	var greek_gr_locale;
	var turkish_tr_locale;
	if (SLASH_IS_BACKSLASH) {
		english_us=1033;
		german_standard=1031;
		greek_gr=1032;
		turkish_tr=1055;
		//english_us_locale="English";
		greek_gr_locale="Greek";
		turkish_tr_locale="Turkish";
		english_us_locale="English";
		//greek_gr_locale=greek_gr;
	} else {
		english_us="en_US.utf8";
		german_standard="de_DE.utf8";
		greek_gr="el_GR.utf8";
		turkish_tr="tr_TR.utf8";
                //try mac versions
                //see locale -a for list
                if (not setxlocale(english_us)) {
                        english_us="en_US.UTF-8";
                        german_standard="de_DE.UTF-8";
                        greek_gr="el_GR.UTF-8";
                        turkish_tr="tr_TR.UTF-8";
                }
		english_us_locale=english_us;
		greek_gr_locale=greek_gr;
		turkish_tr_locale=turkish_tr;
	}

	//in English/US Locale
	//check ASCII upper/lower casing
//	setxlocale(english_us);
	assert(setxlocale(english_us));
	assert(ucase(LOWERCASE_) eq UPPERCASE_);
	assert(lcase(UPPERCASE_) eq LOWERCASE_);

	//in German/Standard Locale
	//check Eszet (like a Beta) uppercases to SS
	assert(setxlocale(german_standard));
	//FAILS in Windows XPSP3UK
	//if (not SLASH_IS_BACKSLASH)
	//	assert(ucase(GermanEszet) eq "SS");
	GermanEszet.oconv("HEX4").oconv("T#4").outputl("German Eszet:");
	ucase(GermanEszet).oconv("HEX4").oconv("T#4").outputl("Uppercased German Eszet:");

	//in Greek Locale
	//convert word ending in "capital sigma" lower cases to "lower final sigma"
	assert(setxlocale(greek_gr));
	//Greek_sas       .outputl("Greek_sas=");
	//ucase(Greek_sas).outputl("ucased   =");
	//lcase(Greek_SAS).oconv("HEX4").oconv("T#4").outputl();
	assert(ucase(Greek_sas) eq Greek_SAS);
	//FAILS in Windows XPSP3UK and linux
	//assert(lcase(Greek_SAS) eq Greek_sas);

	//NB a codepage is a 256 x one byte map of characters selected from all unicode characters depending on locale

	//test windows codepages
	if (SLASH_IS_BACKSLASH) {
		//show where we are working
		printl(oscwd("OSCWD="));
		var greektestfilename="greeksas.txt";
		//check CANNOT write greek unicode characters using French codepage
		assert(not Greek_sas.oswrite(greektestfilename,"French"));//CANNOT write
		//check can write greek unicode characters to Greek codepage
		assert(Greek_sas.oswrite(greektestfilename,"Greek"));
		//check 3 (wide) characters output as 3 bytes
		assert(osfile(greektestfilename)(1) eq 3);
		//check can read greek wide unicode characters from greek codepage
		var rec2;
		assert(rec2.osread(greektestfilename,"Greek"));
		assert(rec2 eq Greek_sas);
		//check raw read as latin
		assert(rec2.osread(greektestfilename,"C"));
		var rec3=L"\xF3\xE1\xF2";
		assert(rec2 eq rec3);//greek code page characters

	}
	//in Turkish Locale
	//check Latin "I" lowercases to "turkish dotless i"
	//check Latin "i" uppercases to "turkish dotted I"
	//fails on Ubuntu 10.04
	assert(setxlocale(turkish_tr));
	printl("Latin Capital I should lower case to dotless Turkish i:",lcase(LatinCapitalI));
	assert(lcase(TurkishCapitalDottedI) eq LatinSmallI);
	assert(ucase(TurkishSmallDotlessI) eq LatinCapitalI);
	if (SLASH_IS_BACKSLASH) {
		assert(lcase(LatinCapitalI) eq TurkishSmallDotlessI);
		assert(ucase(LatinSmallI)   eq TurkishCapitalDottedI);
	}

	//restore initial locale
	setxlocale(locale0);
	setxlocale(english_us);

	var tempfilename5;
	var record5;
	tempfilename5="temp7657.txt";
	assert(oswrite("",tempfilename5));
	assert(osdelete(tempfilename5));

	//check we cannot write to a non-existent file
	//osdelete(tempfilename5); //make sure the file doesnt exist
	assert(not osfile(tempfilename5) or osdelete(tempfilename5));
	var offset=2;
	assert(not osbwrite(L"34",tempfilename5,offset));
	osclose(tempfilename5);

	//check we can osbwrite to an existent file beyond end of file
	//oswrite("",tempfilename5,"utf8");
	oswrite("",tempfilename5);
	assert(osopen(tempfilename5,tempfilename5));
	offset=2;
	assert(osbwrite("78",tempfilename5,offset));
	offset=2;
	var v78=osbread( tempfilename5, offset, 2);
	assert(v78 eq "78");

	assert(osread(record5,tempfilename5));
	assert(record5.oconv("HEX2") eq "00003738");
	assert(osdelete(tempfilename5));

	if (SLASH_IS_BACKSLASH) {
		//check cannot write non-codepage characters
		assert(oswrite("",tempfilename5));
		var offset=2;
		assert(not osbwrite(GreekSmallFinalSigma,tempfilename5,offset));
		//check can write codepage characters
		assert(osopen(tempfilename5,tempfilename5,"Greek"));
		offset=2;
		assert(osbwrite(GreekSmallFinalSigma,tempfilename5,offset));
		assert(osdelete(tempfilename5));
	}

	assert(oconv("ABc.123","MCN") eq "123");
	assert(oconv("ABc.123","MCA") eq "ABc");
	assert(oconv("ABc.123","MCB") eq "ABc123");
	assert(oconv("ABc.123","MC/N") eq "ABc.");
	assert(oconv("ABc.123","MC/A") eq ".123");
	assert(oconv("ABc.123","MC/B") eq ".");

	assert(oconv("ABc.123","MCL") eq "abc.123");
	assert(oconv("ABc.123","MCU") eq "ABC.123");

	//test unicode regular expressions

	//make some latin and greek upper and lower case letters and punctuation plus some digits.
	var unicode="";
	var expect;
	unicode^=GreekQuestionMark;
	unicode^=GreekCapitalGamma;
	unicode^=GreekSmallGamma;
	unicode^=L"ABc-123.456";//some LATIN characters and punctuation

	var status1 = oswrite( unicode, "GreekLocalFile.txt", greek_gr_locale);
var("xx").input();
	var status2 = oswrite( unicode, "GreekUTF-8File.txt", "utf8");
	var status3 = oswrite( unicode, "GreekEnglFile.txt", english_us_locale);

	//test swapping "letters" (i.e. alphabet) with "?"
	//We expect the question mark to remain as it is,
	// and the greek gammas and ABC to be replaced with latin question marks.
	expect=unicode;
	expect.splicer(2,2,"??");
	expect.splicer(4,3,"???");
	//expect.outputl();
	//expect.oconv("HEX4").outputl();
	//swap(unicode,"\\p{L}","?","ri").outputl();
	//swap(unicode,"\\p{L}","?","ri").oconv("HEX4").outputl();
	//p(L} is regular expression for Unicode Letter
	if (SLASH_IS_BACKSLASH)
		assert(swap(unicode,"\\pL","?","ri") eq expect);
	//but what is its inverse?
	//assert(swap(unicode,"\\PL","?","ri") eq expect);

	setxlocale(greek_gr);
	var punctuation=GreekQuestionMark;//(Punctuation)
	var uppercase=GreekCapitalGamma;//(Uppercase)
	var lowercase=GreekSmallGamma;//(Lowercase)
	var letters=lowercase^uppercase;
	var digits=ArabicIndicDigitZero;//(Decimal Digit)

	assert(oconv(punctuation,"MCA") eq "");//extract only alphabetic
	assert(oconv(punctuation,"MCN") eq "");//extract only numeric
	assert(oconv(punctuation,"MCB") eq "");//extract only alphanumeric
	assert(oconv(punctuation,"MC/A") eq punctuation);//extract non-alphabetic
	assert(oconv(punctuation,"MC/N") eq punctuation);//extract non-numeric
	assert(oconv(punctuation,"MC/B") eq punctuation);//extract non-alphanumeric

	assert(oconv(digits,"MCA") eq "");
	assert(oconv(digits,"MCN") eq digits);
	assert(oconv(digits,"MCB") eq digits);
	assert(oconv(digits,"MC/A") eq digits);
	assert(oconv(digits,"MC/N") eq "");
	assert(oconv(digits,"MC/B") eq "");

	assert(oconv("abc .DEF","MCU") eq "ABC .DEF");
	assert(oconv("abc .DEF","MCL") eq "abc .def");

	//uppercase/lowecase conversion only works for ascii at the moment
	//case conversion is perhaps generally done in order to do case insensitive
	//so as long as we provide a case insensitive comparison then the
	//pressure to do case conversion for all languages is less
	//assert(oconv(lowercase,"MCU") eq uppercase);
	//assert(oconv(uppercase,"MCL") eq lowercase);

	//no change
	assert(oconv(uppercase,"MCU") eq uppercase);
	assert(oconv(lowercase,"MCL") eq lowercase);

	oconv(letters,"MCA").outputl("Expected:"^letters^" Actual:");
	oconv(letters,"MCN").outputl("Expected:\"\" Actual:");
	oconv(letters,"MCB").outputl("Expected:"^letters^" Actual:");
	assert(oconv(letters,"MCA") eq letters);
	assert(oconv(letters,"MCN") eq "");
	assert(oconv(letters,"MCB") eq letters);
	assert(oconv(letters,"MC/A") eq "");
	assert(oconv(letters,"MC/N") eq letters);
	assert(oconv(letters,"MC/B") eq "");

	assert(COMMAND eq "service"
	 or COMMAND eq "main"
	 or COMMAND eq "main2"
	 or COMMAND eq "main2.out"
	 or COMMAND eq "testmain"
	 or COMMAND eq "testmain.out"
	 );

#define MULTIPLE_CONNECTION_CODE_EXCLUDED
#ifdef MULTIPLE_CONNECTION_CODE_EXCLUDED
	{
		var conn1;
		//connect normally doesnt touch default connection
		//!!! EXCEPT if default connection is empty then it sets it
		//default connection is empty to start with
		//OR if you disconnect a connection with the same number (ie the same connection)
		assert(conn1.connect( L""));
	}		// one connection is lost here (hangs)

	{
		var conn1;
		conn1.connect( L"");
		conn1.disconnect();
	}
	var dbname2="exodus2b";
	var dbname3="exodus3b";
	{
		var conn1;
		conn1.connect( L"");			// creates new connection with default parameters (connection string)

		//remove any existing test databases
		conn1.deletedb( dbname2);
		conn1.deletedb( dbname3);

		//verify CANNOT connect to non-existent deleted database2
		assert(not conn1.connect(L"dbname="^dbname2));
		assert(not conn1.connect(L"dbname="^dbname3));

		conn1.disconnect();

	}

	{	// Lets test"
		//	global connect/disconnect
		//	locks/unlocks
		//	createfile/deletefile
		connect();			// global connection
		var file = "NANOTABLE";
		assert( createfile( file));
		file.lock( "1");
		file.lock( "2");
		file.unlock( "2");
		file.unlock( "1");
		file.deletefile();
		disconnect();		// global connection
	}

	{
		printl("create dbs exodus2 and exodus3");
		var conn1;
		assert(conn1.connect(""));
		assert(conn1.createdb(dbname2));
		assert(conn1.createdb(dbname3));

		printl("create table2 on exodus2 and table3 on exodus3 - interleaved");
		var conn2,conn3;
		var table2="TABLE2";
		var table3="TABLE3";
		assert(conn2.connect("dbname="^dbname2));
		assert(conn3.connect("dbname="^dbname3));
		assert(not table2.open("TABLE2",conn2));
		assert(not table3.open("TABLE3",conn3));
		assert(conn2.createfile(table2));
		assert(conn3.createfile(table3));
		assert(not table2.open(table2,conn3));
		assert(not table3.open(table3,conn2));
		assert(table2.open(table2,conn2));
		assert(table3.open(table3,conn3));
		assert(write( "2.1111", table2, "2.111"));
		assert(write( "3.1111", table3, "3.111"));
		assert(write( "2.2222", table2, "2.222"));
		assert(write( "3.2222", table3, "3.222"));
		assert(write( "2.3333", table2, "2.333"));
		assert(write( "3.3333", table3, "3.333"));
		assert(conn2.disconnect());
		assert(conn3.disconnect());
	}
	{
		printl("Go through table2 in exodus2 db and through table3 in exodus3 db");
		var conn2, conn3;
		assert(conn2.connect( L"dbname="^dbname2));
		assert(conn3.connect( L"dbname="^dbname3));
		var table2,table3;
		assert( table2.open( "TABLE2",conn2));
		assert( table3.open( "TABLE3",conn3));

		table2.selectrecord();
		table3.selectrecord();
		var record2, id2, record3, id3;

		assert(table2.readnextrecord( record2, id2) and table3.readnextrecord( record3, id3));
		assert(record2 eq "2.1111" and id2 eq "2.111" and record3 eq "3.1111" and id3 eq "3.111");

		assert(table2.readnextrecord( record2, id2) and table3.readnextrecord( record3, id3));
		assert(record2 eq "2.2222" and id2 eq "2.222" and record3 eq "3.2222" and id3 eq "3.222");

		assert(table2.readnextrecord( record2, id2) and table3.readnextrecord( record3, id3));
		assert(record2 eq "2.3333" and id2 eq "2.333" and record3 eq "3.3333" and id3 eq "3.333");

		assert(not table2.readnextrecord( record2, id2) and not table3.readnextrecord( record3, id3));

		printl("check CANNOT delete databases while a connection is open");
		//NB try to delete db2 from conn3 and vice versa
		assert(not conn3.deletedb(dbname2));
		assert(not conn2.deletedb(dbname3));

		conn2.disconnect();
		conn3.disconnect();

		printl("remove any test databases");
		//connect to exodus first cant delete db if connected to it.
		var conn1;
		assert(conn1.connect(L"dbname=exodus"));
		assert(conn1.deletedb(dbname2));
		assert(conn1.deletedb(dbname3));
		conn1.disconnect();
	}
#endif

//#if 0

	deletefile("XUSERS");
	deletefile("dict_XUSERS");

	createfile("XUSERS");
	createfile("DICT_XUSERS");

	//create some dictionary records (field descriptions)
	//PERSON_NO    Type "F", Key Field (0)
	//BIRTHDAY     Type "F", Data Field 1
	//AGE IN DAYS  Type "S", Source Code needs a dictionary subroutine library called dict_XUSERS
	//AGE IN YEARS Type "S", Source Code ditto
	assert(write(convert( "F|0|Person No||||||R|10"   ,"|",FM),"DICT_XUSERS", "PERSON_NO"   ));
	assert(write(convert( "F|1|Birthday||||D||R|12"   ,"|",FM),"DICT_XUSERS", "BIRTHDAY"    ));
	assert(write(convert( "S||Age in Days||||||R|10"  ,"|",FM),"DICT_XUSERS", "AGE_IN_DAYS" ));
	assert(write(convert( "S||Age in Years||||||R|10" ,"|",FM),"DICT_XUSERS", "AGE_IN_YEARS"));

	//create some users and their birthdays 11000=11 FEB 1998 .... 14000=30 APR 2006
	assert(write("11000","XUSERS","1"));
	assert(write("12000","XUSERS","2"));
	assert(write("13000","XUSERS","3"));
	assert(write("14000","XUSERS","4"));

//DBTRACE=true;
	//check can create and delete indexes
	//errmsg = {var_mvstr="ERROR:  function exodus_extract_date(bytea, integer, integer, integer) does not exist
	//use DBTRACE to see the error
	assert(createindex("XUSERS","BIRTHDAY"));
	assert(listindexes("XUSERS") eq ("xusers"^VM^"birthday"));
	assert(listindexes() ne "");
//ALN: do not delete to make subsequent select work::	assert(deleteindex("XUSERS","BIRTHDAY"));
//	assert(listindexes("XUSERS") eq "");

	//check can select and readnext through the records
	assert(select("SELECT XUSERS WITH BIRTHDAY BETWEEN '1 JAN 2000' AND '31 DEC 2003'"));
	assert(readnext(ID));
	assert(ID eq 2);
	assert(readnext(ID));
	assert(ID eq 3);
	assert(not readnext(ID));//check no more

//	if (not selectrecord("SELECT XUSERS WITH AGE_IN_DAYS GE 0 AND WITH AGE_IN_YEARS GE 0"))
//	if (not select("SELECT XUSERS"))
	if (not selectrecord("SELECT XUSERS"))
		assert(false and var("Failed to Select XUSERS!"));

	DICT="dict_XUSERS";
	while (readnextrecord(RECORD,ID))
//	while (readnext(ID))
	{
		printl("ID=",ID, " RECORD=",RECORD);

//		continue;
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
			//		print(" AGE_IN_DAYS=",calculate("AGE_IN_DAYS"));
			//		printl(" AGE_IN_YEARS=",calculate("AGE_IN_YEARS"));
	}

//#endif

	assert(unquote("\"This is quoted?\"") eq "This is quoted?");

	printl("\nVerify that exodus catches c++ defect at runtime");
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

	//replacing a section of a string - method 3 - but may be slower
	a="abcde";
	a=splice(a,-2,-3,"xx");
	assert(a eq "axxe");

	//replacing a section of a string - method 4 - but may be slower
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

	assert(a7.parse("xx"^FM^"bb") eq 2);
	assert(a7(1) eq "xx");
	assert(a7(2) eq "bb");
	assert(a7.unparse() eq ("xx"^FM^"bb"));

	dim arrx(2,2),arry;
	arrx(1,1)="xx";
	assert(arrx(1,1) eq "xx");
	arrx(1,2)=arrx(1,1);
	assert(arrx(1,2) eq "xx");
	arry=arrx;
	assert(arry(1,1) eq "xx");

	var aa1,aa2,aa3,aa4;
	aa1=aa2=aa3="aa";
	assert(aa1 eq "aa");
	assert(aa2 eq "aa");
	assert(aa3 eq "aa");

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
	gosub xyz(da1);

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

	var tempfile=SLASH^"129834192784";
	if (osopen(tempfile,tempfile))
		assert(false and var("non-existent file opened!"));

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
	assert(mod(2.3,var(1.499)).round(3) eq 0.801);
	assert(mod(-2.3,var(-1.499)).round(3) eq -0.801);
	//following is what c++ fmod does (a rather mathematical concept)
	//assert(mod(-2.3,var(1.499)).round(3).outputl() eq -0.801);
	//assert(mod(2.3,var(-1.499)).round(3).outputl() eq 0.801);
	//but arev and qm ensure that the result is somewhere from 0 up to or down to
	//(but not including) the divisor
	//method is ... do the fmod and if the result is not the same sign as the divisor, add the divisor
	assert(mod(-2.3,var(1.499)).round(3) eq 0.698);
	assert(mod(2.3,var(-1.499)).round(3) eq -0.698);

	assert(oconv(1234,"MD20P") eq "1234.00");
	assert(var(10000).oconv("DY0") eq "");

	assert(var("a")<var("B"));
	assert(var(1000).oconv("MD80") eq "1000.00000000");
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
	/*
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
	*/

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
	assert(dcount(log1,"x") eq 3);

	printl(SENTENCE);
	assert(var("xyz").substr(4,1) eq "");

	//TODO ensure isnum converts ints larger that the maximum int to FLOATS

	var subs="xyz";
	assert(subs.substr(-1) eq "z");
	assert(subs[-1] eq "z");

	assert(oconv("a","L#3") eq "a  ");
	assert(oconv("abc","L#3") eq "abc");
	assert(oconv("abcd","L#3") eq "abc");
	assert(oconv("a"^FM^"abc"^FM^"abcd","L#3") eq ("a  "^FM^"abc"^FM^"abc"));

	assert(oconv("a","R#3") eq "  a");
	assert(oconv("abc","R#3") eq "abc");
	assert(oconv("abcd","R#3") eq "bcd");
	assert(oconv("a"^FM^"abc"^FM^"abcd","R#3") eq ("  a"^FM^"abc"^FM^"bcd"));

	assert(oconv("a","T#3") eq "a  ");
	assert(oconv("abc","T#3") eq "abc");
	assert(oconv("abcd","T#3") eq ("abc"^TM^"d  "));
	assert(oconv("a"^FM^"abc"^FM^"abcd","T#3") eq ("a  "^FM^"abc"^FM^"abc"^TM^"d  "));

	assert(oconv("a","L(0)#3") eq "a00");
	assert(oconv("a","R(0)#3") eq "00a");
	assert(oconv("a","T(0)#3") eq "a00");
	assert(oconv("abcd","T(0)#3") eq ("abc"^TM^"d00"));

	initrnd(999);
	var rnd1=rnd(999);
	//777 on win32, 802 on Ubuntu 10.04 64bit
	assert(rnd1 eq 777 or rnd1 eq 802);
	assert(iconv("23 59 59","MT") eq 86399);
	assert(iconv("xx11yy12zz13P","MT") eq 83533);
	assert(iconv("24 00 00","MT") eq "");

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
	//assert(var("FF"^FM^"00").iconv("HEX") eq ("00FF"^FM^"00FF"));
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
	printl("\nTest catching MVexceptions");
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

//	assert(oconv(FM ^ L"\x0035","HEX4") eq "00FE0035");
	assert(oconv(FM ^ L"\x0035","HEX4") eq "02FE0035");
	assert(oconv(FM,"HEX4") eq "02FE");

	printl();
	printl("osdir("^SLASH^")=",osdir(SLASH));

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
	printl("char:     ",(int)sizeof(char));
	printl("wchar_t:  ",(int)sizeof(wchar_t));
	printl("string:   ",(int)sizeof(std::string));
	printl("wstring:  ",(int)sizeof(std::wstring));
	printl("int:      ",(int)sizeof(int));
	printl("long:     ",(int)sizeof(long));
	printl("long long:",(int)sizeof(long long));
	printl("double:   ",(int)sizeof(double));
	printl("var:      ",(int)sizeof(var));

	var tconv=FM.oconv("T#20");
	assert(tconv eq (L"                    " _FM_ L"                    "));
	tconv="xxxxx/xxxxx xxx" ^ FM ^ "xx";
	tconv=tconv.oconv("T#8");
	assert(tconv eq ("xxxxx/xx" ^ TM ^ "xxx xxx " ^ FM ^ "xx      "));

	//test regular expression
	//four digits followed by dash or space) three times ... followed by four digits
	var regex1="(\\d{4}[- ]){3}\\d{4}";
	var regtest="1247-1234-1234-1234";
	assert(regtest.match(regex1,"r"));

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
//	var xx=xx[1];

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

	write("F"^FM^0^FM^"Currency Code"^FM^FM^FM^FM^FM^FM^"L"^"10","DICT_CURRENCIES","CURRENCY_CODE");
	write("F"^FM^1^FM^"Currency Name"^FM^FM^FM^FM^FM^FM^"T"^"20","DICT_CURRENCIES","CURRENCY_NAME");
	write("F"^FM^1^FM^"Market Code"^FM^FM^FM^FM^FM^FM^"L"^"10","DICT_MARKETS","CODE");
	write("F"^FM^1^FM^"Market Name"^FM^FM^FM^FM^FM^FM^"T"^"20","DICT_MARKETS","NAME");

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
		while (ii<3&&ads.readnextrecord(record,key))
		{
			++ii;
			if (!(ii%10000))
				printl(" ",key);
			if (record.lcase().index("QWEQWE"))
				print("?");

		}
	}
	clearselect();

#ifdef FILE_IO_CACHED_HANDLES_EXCLUDED
	{	// test to reproduce cached_handles error
		var file1( "FILE1.txt");
		oswrite( L"", file1);
		var off1 = 0;
		osbwrite( L"This text is written to the file 'FILE1.txt'", file1, off1);

		var file2( "FILE2.txt");
		oswrite( L"", file2);
		var off2 = 0;
		osbwrite( L"This text is written to the file 'FILE2.txt'", file2, off2);

		var file1x = file1;		// wicked copy of file handle
		file1x.osclose();		// we could even do: var( file1).osclose();

		var file3( "FILE3.txt");
		oswrite( L"", file3);
		var off3 = 0;
		osbwrite( L"This text is written to the file 'FILE3.txt'", file3, off3);

		osbwrite( L"THIS TEXT INTENDED FOR FILE 'FILE1.txt' BUT IT GOES TO 'FILE3.txt'", file1, off1);
	}
#endif

    printl("testmain exiting ...");

    return 0;
}

function xyz(in xyzz)
{
	assert(xyzz(2,2,2) eq "b22");
	return 1;
}

function accrest() {
        var infilename="\\tapex";//=field(sentence()," ",2);
        var infile;
        if (not osopen(infilename,infile))
                abort("Cant read "^infilename);

        var fms=FM^VM^SM^TM^STM^SSTM;
        var visibles="^]\???";
        var EOL="\r\n";
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

programexit()
