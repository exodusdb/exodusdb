#include <exodus/library.h>
libraryinit()

function main(in question0, in options0, io reply) {
	var question=question0;
	var options=options0;
	var nn=options;
	question.converter("|"^VM,"\n\n");
	options.converter("|"^VM,FM^FM);
	printl(question);
	for (var ii=0;ii<nn;++ii) {
		printl(ii, ". ", options.a(ii));
	}
	var ans;
	reply.input("Which option?",reply);
	return options.a(reply);
}

libraryexit()

