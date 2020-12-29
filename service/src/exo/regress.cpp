#include <exodus/program.h>
programinit()

function main() {
	printl("regress says 'Hello World!'");
	var osfilename=field(SENTENCE," ",2);
	var osfile;
	if (not(osopen(osfilename,osfile)))
		stop("cant open "^osfilename);
	var data;
	if (not(osread(data,osfilename)))
		stop("cant open "^osfilename);

	data.converter("\r\n",RM^RM);
	var bit=data.substr(1,100);
	var ix=0;
	var delimiter;
	int ln=0;
	while (true) {
		++ln;
		if (not(mod(ln,1000)))
			printl(ln);
		var line=data.substr2(ix,delimiter);
//		line.outputl();
		var output=line.field("\t",1);
		var op=line.field("\t",2);
		var input=line.field("\t",3);
		var arg1=line.field("\t",4);
		//op.outputl();
		var test;
		//if (op=="LOCATEUSING") {
		//	//0 1     LOCATEUSING     BCCCB   A               B               -1      -1      -1
		//	test=locateusing(input.a(,
		//}
		if (op=="OCONV") {
			test=oconv(input,arg1);
		} else if (op=="ISNUM") {
			test=input.isnum();
		} else {
			printl(line);
			stop();
		}
		if (test ne output) {
			printt(ln,op,input.convert(FM^VM^SM,"^]\\"),arg1,output.convert(FM^VM^SM,"^]\\"),test.convert(FM^VM^SM,"^]\\"));
			printl();
		}
		if (not delimiter)
			break;
	}
	return 0;
}

programexit()

