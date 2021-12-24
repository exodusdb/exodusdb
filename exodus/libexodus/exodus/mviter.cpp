#include <mv.h>

//mv_iter class enables c++ range based programming over a dynamic array
//
//uses FM as field separator at the moment
//
//Example:
//
// for (const var& v : dynstr) {...}
//
//Warning: updating the string probably invalidates the iterator! It is a byte pointer into the string.
//
//TODO add option a) to specify field separator eg VM, SM and/or b) acquire field number as well
//as provided by SRP precompiler
//https://wiki.srpcs.com/display/SRPUtilities/SRP_PreCompiler#SRP_PreCompiler-ForEachLoops
//For Each Value in MyValues using @STM setting Pos
//    NewValues<Pos> = Value
//Next Value

namespace exodus {

//CONSTRUCTOR from a var (ie begin())
var_iter::var_iter(const var& v)
	: data(&v){}

//check iter != iter (i.e. iter != end()
bool var_iter::operator!=([[maybe_unused]] var_iter& vi) {
	//no need to use vi since the end is always string::npos;
	return this->index != std::string::npos;
}

//CONVERSION - conversion to var
var_iter::operator var*() {

	//find the end of the field if not already known
	if (index2 == std::string::npos) {
		index2 = data->var_str.find(FM_, index);
	}

	//extract the field
	if (index2 == std::string::npos)
		field = data->var_str.substr(index);
	else
		field = data->var_str.substr(index, index2 - index);

	return &field;
}

//INCREMENT
var_iter var_iter::operator++() {

	//find the end of the field if not already found from a call to above CONVERSION
	if (index2 == std::string::npos)
		index2 = data->var_str.find(FM_, index);

	//move up to the next field
	index = index2;

	//skip over any FM character
	if (index != std::string::npos) {
		//max str size = 9223372036854775807
		//string npos = 18446744073709551615
		//we will ignore the fact that we could be processing
		//a string of maximum size with a terminating FM
		//and incrementing would take us past maximum string size
		//but not beyond "no position" npos.
		index++;
	}

	//indicate that the end of the next field is not yet known
	index2 = std::string::npos;

	return *this;
}

//BEGIN - free function to create an iterator -> begin
DLL_PUBLIC var_iter begin(const var& v) {
	return var_iter(v);
}

//END - free function to create an interator -> end
DLL_PUBLIC var_iter end([[maybe_unused]] const var& v) {
	return var_iter();
}

}  //namespace exodus
