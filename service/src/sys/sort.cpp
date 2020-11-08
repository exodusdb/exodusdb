#include <exodus/library.h>
libraryinit()

#include <nlist.h>

function main() {
	//c sys

	//only to enable perform('LIST ...') in c++
	//appears as a loadable library liblist.so

	//duplicate in LIST and SORT

	//NB ../add/list.cpp in c++ provides ~/bin/list os command line
	//list os command line will not work for dicts using labelled commons

	//../sys/list.cpp -> executable "list" for use from os command line
	//../add/list.cpp -> loadable library "list" for exodus command perform()
	//../add/sort.cpp -> loadable library "list" for exodus command perform()
	//../add/nlist.cpp -> loadable library called by both to do the actual work

	//adecom is hardcoded to output to list.cpp despite VOC LIST pointing to NLIST

	call nlist();
	return 0;
}

libraryexit()
