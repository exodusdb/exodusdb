#include <exodus/library.h>
libraryinit()

function main() {

	//syntax is:
	//createfile filename dictfilename
	//or
	//createfile filename

	//volumename is ignored

	var command=COMMAND.convert(FM," ").lcase();

	command.swapper(" dict."," dict ");
	command.swapper(" dict_"," dict ");
	var dataonly=command.index(" data ");
	var dictonly=command.index(" dict ");
	command.swapper(" data "," ");
	command.swapper(" dict "," ");

	//printl(command);

	//something like DATAVOL\NEOS0001 or REVBOOT will be ignored
	var volname=field(command," ",2);

	var filename=field(command," ",3);
	if (not filename) {
		filename=volname;
		volname="";
	}

	//TODO support volume in createfile

    //dict file
	if (not dataonly)
		create("dict_"^filename);

    //data file
	if (not dictonly)
		create(filename);

	return 0;
}

subroutine create(in filename) {
	var msg=filename;
	if (createfile(filename))
		msg^=" created";
	else
		msg^=" not created";
	if (not index(OPTIONS,"S"))
		printl(msg);

}
libraryexit()
