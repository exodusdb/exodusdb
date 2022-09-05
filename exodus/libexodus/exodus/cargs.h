#ifndef CARGS_H
#define CARGS_H 1

#include <list>
#include <string>
#include <vector>

#include <exodus/var.h>

namespace exodus {

//Cargs is a class to create temporary argv (char*[]) from a var string
//that will autodestruct when out of scope and not leak memory
class Cargs {

	//a place to store primitive c style strings
	//until object destruction
	std::list<std::string> strings;

	//a vector of char to represent char* argv[]
	std::vector<char*> argv_;

   public:
	//constructor from var
	Cargs(CVR command) {
		//printl(command);
		for (var word : command) {
			//printl(word);
			argv_.push_back(strings.emplace_back(std::string(word ^ "\x00")).data());
		}
		//last arg must be nullptr
		argv_.push_back(nullptr);
	}

	//return argc as int
	int argc() {
		return argv_.size() - 1;
	}

	//return argv_ as char*[]
	const char** argv() {
		return (const char**)argv_.data();
	}
};

}  // namespace exodus

#endif
