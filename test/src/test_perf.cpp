#undef NDEBUG  //because we are using assert to check actual operations that cannot be skipped in release mode testing
//#include <ranges>
#include <boost/range/irange.hpp>
#include <cassert>

#include <exodus/program.h>

#include <stdio.h>
#include <string.h>

/* not using invalid UTF8 characters any more
#define FM_ '\xFE'
#define VM_ '\xFD'
#define SM_ '\xFC'
*/

#define FM_ '\x1E'
#define VM_ '\x1D'
#define SM_ '\x1C'

/*
TODO algorithm could be improved for value and subvalue extraction
by not searching for the end of the field before starting to look for the multivalue
and for  extracting subvalues, and not searching for the end of the multivalue
before starting to look for the subvalue.
*/
void extract(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength)
{
        int start_pos;
        int fieldn2;
        int field_end_pos;
        int valuen2;
        int value_end_pos;
        int subvaluen2;
        int subvalue_end_pos;

        /*assert(assigned());*/

        /*
        any negatives at all returns "" but since this is unexpected
        do it inline instead of special case to avoid wasting time
        if (fieldno<0||valueno<0||subvalueno<0) return;
        */

        /*FIND FIELD*/

        /*default to returning nothing*/
        *outstart=0;
        *outlength=0;

        /*zero means all, negative return ""*/
        if (fieldno<=0)
        {
                if (fieldno<0) return;
                if (valueno||subvalueno) fieldno=1; else
                {
                        *outlength=inlength;
                        return;
                }
        }

        /*find the starting position of the field or return ""*/
        start_pos=0;
        fieldn2=1;

        //ALN:NOTE - bad algorithm - assumes that string is valid UTF
        //and FM_UTF8_1 or VM_UTF8_1 SM_UTF8_1 can not be last char in the string

        while (fieldn2<fieldno)
        {
                for (;
                                start_pos<inlength &&
                                instring[start_pos] != FM_;
                                //(instring[start_pos] != FM_UTF8_1 ||
                                //instring[start_pos+1] != FM_UTF8_2) ;
                        start_pos++)
                {};
                /*past of of string?*/
                if (start_pos>=inlength)
                        return;
//              start_pos += 2;
                start_pos++;
                fieldn2++;
        }

        /*find the end of the field (or one after the end of the string)*/
        for (field_end_pos=start_pos;
                        field_end_pos<inlength &&
                        instring[field_end_pos] != FM_;
                        //(instring[field_end_pos]!=FM_UTF8_1 ||
                        //instring[field_end_pos+1] != FM_UTF8_2);
                field_end_pos++)
        {};

        /*FIND VALUE*/

        /*zero means all, negative return ""*/
        if (valueno<=0)
        {
                if (valueno<0)
                        return;
                if (subvalueno)
                        valueno=1;
                else
                {
                        *outstart=start_pos;
                        *outlength=field_end_pos-start_pos;
                        return;
                }
        }

        /*
        find the starting position of the value or return ""
        using start_pos and end_pos of
        */
        valuen2=1;
        while (valuen2<valueno)
        {
                for (/*start_pos=start_pos*/;
                                start_pos<inlength &&
                                instring[start_pos] != VM_;
                                //(instring[start_pos]!=VM_UTF8_1 ||
                                //instring[start_pos+1]!=VM_UTF8_2);
                        start_pos++)
                {};
                /*past end of string?*/
                if (start_pos>=inlength)
                        return;
//              start_pos += 2;
                start_pos ++;
                /*past end of field?*/
                if (start_pos>field_end_pos)
                        return;
                valuen2++;
        }

        /*find the end of the value (or string)*/
        for (value_end_pos=start_pos;
                        value_end_pos<field_end_pos &&
                        instring[value_end_pos] != VM_;
                        //(instring[value_end_pos]!=VM_UTF8_1 ||
                        //instring[value_end_pos+1]!=VM_UTF8_2);
                value_end_pos++)
        {}


        /*FIND SUBVALUE*/

        /*zero means all, negative means ""*/
        if (subvalueno<=0)
        {
                if (subvalueno<0)
                        return;
                *outstart=start_pos;
                *outlength=value_end_pos-start_pos;
        }

        /*
        find the starting position of the subvalue or return ""
        using start_pos and end_pos of
        */
        subvaluen2=1;
        while (subvaluen2<subvalueno)
        {
                for (/*start_pos=start_pos*/;
                                start_pos<field_end_pos &&
                                instring[start_pos] != SM_;
                                //(instring[start_pos]!=SM_UTF8_1 ||
                                //instring[start_pos+1]!=SM_UTF8_2);
                        start_pos++)
                {};
                /*past end of string?*/
                if (start_pos>=inlength)
                        return;
                //start_pos += 2;
                start_pos++;
                /*past end of value?*/
                if (start_pos>value_end_pos)
                        return;
                subvaluen2++;
        }

        /*find the end of the subvalue (or string)*/
        for (subvalue_end_pos=start_pos;
                        subvalue_end_pos<value_end_pos &&
                        instring[subvalue_end_pos] != SM_;
                        //(instring[subvalue_end_pos]!=SM_UTF8_1 ||
                        //instring[subvalue_end_pos+1]!=SM_UTF8_2);
                subvalue_end_pos++)
        {};
        if (subvalue_end_pos>=value_end_pos)
        {
                *outstart=start_pos;
                *outlength=value_end_pos-start_pos;
                return;
        }
        *outstart=start_pos;
        *outlength=subvalue_end_pos-start_pos;
        return;

}

programinit()

	function main() {

	// Quit if running under make since there are no tests
	if (osgetenv("MAKELEVEL")) {
		printl();
		printl("Test passed - skipped because MAKELEVEL is set");
		return 0;
	}

	printl();

	int nn = 1'000'000;
	var setup_time;

	bool   the_truth_hurts = false;
	double some_dbl		   = 1.1;
	char   some_char	   = ' ';

	printl("Exp: ? ns extract 10,10,10");
	{
		var started = ostime();
		var v1 = str("1" _FM, 9) ^ str("2" _VM, 9) ^ str("3" _SM, 9) ^ "Q";
		auto s1 = v1.toString();
		char* start = s1.data();
		int len = int(s1.size());
		int outstart = 0;
		int outlen = 0;
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var().assigned();
			//void extract(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength)
			extract(start, len, 10, 10, 10, &outstart, &outlen);
		}
		printl(std::string(start+outstart, outlen));
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}

	printl();

	printl("Exp: 7 ns Time creation+destruction of unassigned var + test within int loop");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var().assigned();
		}
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}

	printl();

	printl("Exp: 15  ns - Creation+destruction of string var '12345.67' + test within int loop");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var("12345.67").assigned();
		}
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}
	printl();

	printl("Exp: 65 ns - Conversion of string var '12345.67' to double (Using fast_float::from_chars on Ubuntu 20.04/g++v9.3 and Ubuntu 22.04/g++v11.2)");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			some_dbl = var("12345.67").toDouble();
			//var("12345.67");
		}
		printl("Act:", round((ostime() - started - setup_time) / nn * 1E9), "ns");
	}

	printl();

	printl("Exp: 6 ns - Creation+destruction of double var 12345.67 + test within int loop");
	{
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			the_truth_hurts = var(12367).assigned();
		}
		setup_time = ostime() - started;
		printl("Act:", round((setup_time) / nn * 1E9), "ns");
	}
	printl();

	printl("Exp: 650  ns - Conversion of double var 12345.67 to string and return length (Using RYU D2S on Ubuntu 20.04/g++v9.3 and std::to_chars on Ubuntu 22.04/g++v11.2)");
	{
		var nn		= 100'000;
		var started = ostime();
		for (int ii = 0; ii lt nn; ii++) {
			some_char = var(12345.67).toChar();
		}
		printl("Act:", round((ostime() - started - setup_time) / nn * 1E9), "ns");
	}

	// Test integer loops
	{
		printl();
		printl("Testing simple integer for loops");
		printl("--------------------------------");

		int firstcasen = COMMAND.f(2);
		if (not firstcasen)
			firstcasen = 0;

		int ncases = 20;

		for (int casen = firstcasen; casen le ncases; casen++) {

			printl();
			for (int repeatn = 0; repeatn lt 3; repeatn++) {

				int nn = 1'000'000;

				int i1 = 1;
				//			double d1 = 0;
				//			char c1 = '\0';
				//			char* cp1 = nullptr;
				bool		b1	= true;
				var			v1	= 1;
				std::string ss1 = "x";

				var started = ostime();

				switch (casen) {

					break;
					case 0:
						if (repeatn eq 0)
							printl("Exp: 35     ns - old var method - for (var;;)");
						nn = 1'000'000;
						for (var v2 = 0; v2 le nn; v2++) {
							i1 = v2;
						};

						break;
					case 1:
						if (repeatn eq 0)
							printl("Exp: 15     ns - new var method - for (var:range)");
						for (const var v2 : range(0 to nn)) {
							i1 = v2;
						};

						break;
					case 2:
						if (repeatn eq 0)
							printl("Exp: 0.840 ns - old int method - for (int;;)");
						for (int i2 = 0; i2 le nn; i2++) {
							i1 = i2;
						};

						break;
					case 3:
						if (repeatn eq 0)
							printl("Exp: 0.840 ns - new int method - for (int:range)");
						for (int i2 : range(0 to nn)) {
							i1 = i2;
						};

						break;
					case 4:
						if (repeatn eq 0)
							printl("Exp: 0.840 ns - new int method using boost - for (int:range)");
						for (int i2 : boost::irange(0, nn)) {
							i1 = i2;
						};

						break;
//					case 21:
//						if (repeatn eq 0)
//							printl("Exp: 0.840 ns - using std::iota - for (int:range)");
//						for (int i2 : std::views::iota(0, nn)) {
//							i1 = i2;
//						};
//
//						break;
					case 5:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct empty var + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var x;
							b1 = x.assigned();
						};

						break;
					case 6:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct from int + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var i = 123456;
							b1	  = i.assigned();
						};

						break;
					case 7:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct from bool + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var b = true;
							b1	  = b.assigned();
						};

						break;
					case 8:
						if (repeatn eq 0)
							printl("Exp: 6.5   ns - construct from double + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var d = 123.456;
							b1	  = d.assigned();
						};

						break;
					case 9:
						if (repeatn eq 0)
							printl("Exp: 12    ns - construct from cstr + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var s = "1";
							b1	  = s.assigned();
						};

						break;
					case 10:
						if (repeatn eq 0)
							printl("Exp: 12    ns - construct from char + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var s = '1';
							b1	  = s.assigned();
						};

						break;
					case 11:
						if (repeatn eq 0)
							printl("Exp: 12    ns - construct from std::string + test");
						for (int i2 = 0; i2 le nn; i2++) {
							var s = ss1;
							b1	  = s.assigned();
						};

						break;
					case 12:
						if (repeatn eq 0)
							printl("Exp: 4.5   ns - pure test");
						for (int i2 = 0; i2 le nn; i2++) {
							b1 = v1.assigned();
						};

						break;
					case 13:
						if (repeatn eq 0)
							printl("Exp: 0.9   ns - assign from int");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = 123456;
						};

						break;
					case 14:
						if (repeatn eq 0)
							printl("Exp: 0.9   ns - assign from bool");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = true;
						};

						break;
					case 15:
						if (repeatn eq 0)
							printl("Exp: 1.3   ns - assign from double");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = 123.456;
						};

						break;
					case 16:
						if (repeatn eq 0)
							printl("Exp: 13    ns - assign from cstr");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = "1";
						};

						break;
					case 17:
						if (repeatn eq 0)
							printl("Exp: 11    ns - assign from char");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = '1';
						};

						break;
					case 18:
						if (repeatn eq 0)
							printl("Exp: 11    ns - assign from std::string");
						for (int i2 = 0; i2 le nn; i2++) {
							v1 = ss1;
						};

						break;
					case 19:
						if (repeatn eq 0)
							printl("Exp: 22    ns - var + int + test");
						for (int i2 = 0; i2 le nn; i2++) {
							b1 = (v1 + i2).assigned();
						};

						break;
					case 20:
						if (repeatn eq 0)
							printl("Exp: 22    ns - var + 0.1 + test");
						for (int i2 = 0; i2 le nn; i2++) {
							b1 = (v1 + 0.1).assigned();
						};
						break;
				}
				var ended = ostime();
				if (i1 or b1 or v1)
					printl("Act:", ((ended - started) / int(nn) * 1e9).round(3), "ns");
			}
		}
	}

	// Ensure optimiser doesnt see as unused
	printl(the_truth_hurts, some_dbl, some_char);

	printl();
	printl(elapsedtimetext());
	printl("Test passed");

	return 0;
}

programexit()
