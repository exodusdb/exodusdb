#include <exodus/library.h>
libraryinit()

function main() {

	//syntax is:
	//makefile volumename filename
	//or
	//makefile filename

	//volumename is ignored

	var command=COMMAND.convert(FM," ").lcase();

	var dataonly=command.index(" data ");
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
	var oscmd="createfile " ^ filename ^ " {S}";// ^ " " ^ OPTIONS;

    //create a dict file as well if not creating a dict file and no "data" keyword
	if (not dataonly and filename.substr(1,5) ne "dict_")
		osshell("createfile dict_"^filename) ^ " {S}";

	oscmd.outputl("oscmd=");
	return osshell(oscmd);
}

libraryexit()

