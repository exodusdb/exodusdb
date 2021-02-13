#include <exodus/library.h>
libraryinit()

function main(in msg, in maxlen, in show, in allowablechars, io data, in escx) {

	//unused parameters
	false && maxlen, show && allowablechars && escx;

	var prompt=msg;
	if (!data.assigned())
		data="";
	/*
	if (data)
		prompt^=" (default=" ^ data ^ ")";
	var reply;
	//reply.input(prompt,maxlen);
	reply.input(prompt);
	if (reply)
		data=reply;
	*/
	data.input(prompt);

	return 0;
}

libraryexit()

