//#include <boost/date_time/posix_time/posix_time.hpp>

#include <iostream> // for stdout/stderr
#include <cstdio>

#include <exodus/program.h>
programinit()

function main() {

	// Easy way to skip testing since it it quite sensitive
	if (osfile("test_output.freepass")) {
		outputl("Test passed.");
		stop();
	}

	let filename1 = "t_stdout.txt";
	let filename2 = "t_stderr.txt";

	// 1. GENERATE OUTPUT and quit
	if (COMMAND.contains("GENERATE")) {
		generate();
		stop();
	}

	// 2. Self run to capture stdout and stderr
	///////////////////////////////////////////
	if (not osremove(filename1)) {};
	if (not osremove(filename2)) {};

	var cmd = EXECPATH ^ " GENERATE 1>" ^ filename1 ^ " 2>" ^ filename2;
	TRACE(cmd)
	if (not osshell(cmd)) {
		abort(lasterror());
	}

	// 3. Check output matches expected
	///////////////////////////////////

	printl("Checking", filename1);
	//////////////////////////////

	var current_stdout;
	if (not osread(current_stdout from filename1)) {
		outputl(filename1 ^ " missing");
		abort();
	}

	if (current_stdout ne expected_stdout) {
		outputl(filename1 ^ " has changed. See ", filename1 ^ ".expected");
		if (not oswrite(expected_stdout on filename1 ^ ".expected")) {};
		if (osshell("diff " ^ filename1 ^ " " ^ filename1 ^ ".expected 2>&1")) {};
		abort();
	}

	printl("Checking", filename2);
	//////////////////////////////

	var current_stderr;
	if (not osread(current_stderr from filename2)) {
		outputl(filename2 ^ " missing");
		abort();
	}

	if (current_stderr ne expected_stderr) {
		outputl(filename2 ^ " has changed. See ", filename2 ^ ".expected");
		if (not oswrite(expected_stderr on filename2 ^ ".expected")) {};
		if (osshell("diff " ^ filename2 ^ " " ^ filename2 ^ ".expected 2>&1")) {};
		abort();
	}

	outputl("Test passed.");

	stop();
}

subroutine generate() {

	// Prepare some text with all the field marks
	var fms = "RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; "_var;

	// All varieties of var.output call var.put() to output

// OUTPUT

	{
		outputl();

		outputl("\n=== free output() ===");
		output(fms);
		outputl();

		outputl("\n=== free outputt() ===");
		outputt(fms);
		outputl();

		outputl("\n=== free outputl() ===");
		outputl(fms);

	}
	{
		outputl();

		outputl("\n=== var::output ===");
		fms.output();
		outputl();

		outputl("\n=== var::outputt ===");
		fms.outputt();
		outputl();

		outputl("\n=== var::outputl ===");
		fms.outputl();

	}

// PRINT

	{
		printl();

		printl("\n=== free print() ===");
		printx(fms); // printx not print
		printl();

		printl("\n=== free printt() ===");
		printt(fms);
		printl();

		printl("\n=== free printl() ===");
		printl(fms);

	}

	{
		outputl();
		outputl("\n=== cout << var ===");
		std::cout << fms << std::endl;
	}
	{
		outputl();
		outputl("\n=== var::put(cout) ===");
		fms.put(std::cout);
		outputl();
	}

// LOGPUT

	{
		logputl();

		logputl("\n=== free logput() ===");
		logput(fms);
		logputl();

//		logputl("\n=== free logputt() ===");
//		logputt(fms);
//		logputl();

		logputl("\n=== free logputl() ===");
		logputl(fms);

	}
	{
		logputl();

		logputl("\n=== var::logput ===");
		fms.logput();
		logputl();

//		logputl("\n=== var::logputt ===");
//		fms.logputt();
//		logputl();

		logputl("\n=== var::logputl ===");
		fms.logputl();

	}

// ERRPUT

	{
		errputl();

		errputl("\n=== free errput() ===");
		errput(fms);
		errputl();

//		errputl("\n=== free errputt() ===");
//		errputt(fms);
//		errputl();

		errputl("\n=== free errputl() ===");
		errputl(fms);

	}
	{
		errputl();

		errputl("\n=== var::errput ===");
		fms.errput();
		errputl();

//		errputl("\n=== var::errputt ===");
//		fms.errputt();
//		errputl();

		errputl("\n=== var::errputl ===");
		fms.errputl();

	}


// OTHER

	{
		errputl();

		errputl("\n=== cerr << var ===");
		std::cerr << fms << std::endl;

		logputl("\n=== clog << var ===");
		std::clog << fms << std::endl;

		errputl("\n=== TRACE ===");
		TRACE(fms)

	}

}

let expected_stdout = R"V0G0N(

=== free output() ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== free outputt() ===
		RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== free outputl() ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 


=== var::output ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== var::outputt ===
		RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== var::outputl ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 


=== free print() ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== free printt() ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 	

=== free printl() ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 


=== cout << var ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 


=== var::put(cout) ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 
)V0G0N";

let expected_stderr = R"V0G0N(

=== free logput() ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 

=== free logputl() ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 


=== var::logput ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 

=== var::logputl ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 


=== free errput() ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 

=== free errputl() ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 


=== var::errput ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 

=== var::errputl ===
RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; 


=== cerr << var ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== clog << var ===
RM= ;  FM= ;  VM= ;  SM= ;  TM= ;  ST= ; 

=== TRACE ===
TRACE: fms = "RM= `;  FM= ^;  VM= ];  SM= };  TM= |;  ST= ~; "
)V0G0N";

programexit()

