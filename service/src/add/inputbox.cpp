#include <exodus/library.h>
libraryinit()

function main(in msg, in maxlen, in show, in allowablechars, io data, in escx) {

	false && show && allowablechars && escx;

	var prompt=msg;
	if (!data.assigned())
		data="";
	if (data)
		prompt^=" (default=" ^ data ^ ")";
	var reply;
	reply.input(prompt,maxlen);
	if (reply)
		data=reply;
	return 0;
}

libraryexit()

