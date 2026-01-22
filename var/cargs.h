#ifndef CARGS_H
#define CARGS_H 1

#if EXO_MODULE > 1
	import std;
#else
#	include <list>
#	include <string>
#	include <vector>
#endif

#include <var/var.h>

namespace exo {

//Cargs is a class to create temporary argv (char*[]) from a var string
//that will autodestruct when out of scope and not leak memory
class Cargs {

	//a place to store primitive c style strings
	//until object destruction
	std::list<std::string> strings;

	//a vector of char to represent char* argv[]
	std::vector<char*> argv_;

   public:
	//constructor from var delimited by FM e.g. COMMAND and optional OPTIONS
	Cargs(in command, in options = "") {
		//printl(command);
		for (var word : static_cast<const var&>(command)) {
			//printl(word);
			argv_.push_back(strings.emplace_back(std::string(word ^ "\x00")).data());
		}
		if (not options.empty())
			argv_.push_back(strings.emplace_back(std::string("{" ^ options ^ "}\x00")).data());

		//last arg must be nullptr
		argv_.push_back(nullptr);
	}

	//return argc as int
	int argc() {
		//warning: conversion from ‘std::vector<char*>::size_type’ {aka ‘long unsigned int’} to ‘int’ may change value [-Wconversion]
		return static_cast<int>(argv_.size()) - 1;
	}

	//return argv_ as char*[]
	const char** argv() {
		//return (const char**)argv_.data();
		return const_cast<const char**>(argv_.data());
	}
};

}  // namespace exo

#endif
