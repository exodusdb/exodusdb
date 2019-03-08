#include <exodus/library.h>
libraryinit()

function main() {

	//syntax is:
	//makefile volumename filename
	//or
	//makefile filename

	//volumename is ignored

	var command=COMMAND.convert(FM," ").lcase();

	command.swapper(" data "," ");
	command.swapper(" dict "," dict_");
	command.swapper(" dict."," dict_");

	//printl(command);

	//something like DATAVOL\NEOS0001 or REVBOOT will be ignored
	var volname=field(command," ",2);

	var filename=field(command," ",3);
	if (not filename) {
		filename=volname;
		volname="";
	}
	var file;
	//TODO support volume in createfile
	var oscmd="createfile " ^ filename ^ "{S}";// ^ " " ^ OPTIONS;
	//oscmd.outputl("oscmd=");
	return osshell(oscmd);
}

libraryexit()

