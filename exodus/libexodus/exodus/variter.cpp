
#if EXO_MODULE
	import std;
#else
#	include <string>
#endif

#include <exodus/var.h>
//#include <exodus/dim.h>
//#include <exodus/rex.h>

//mv_iter class enables c++ range based programming over a dynamic array
//
//uses FM as field separator at the moment
//
//Example:
//
// for (in v : dynstr) {...}
//
//Warning: updating the string probably invalidates the iterator! It is a byte pointer into the string.
//
//TODO add option a) to specify field separator eg VM, SM and/or b) acquire field number as well
//as provided by SRP precompiler
//https://wiki.srpcs.com/display/SRPUtilities/SRP_PreCompiler#SRP_PreCompiler-ForEachLoops
//For Each Value in MyValues using @ST setting Pos
//    NewValues<Pos> = Value
//Next Value

namespace exo {

///////////
// var_iter
///////////

//CONSTRUCTOR from a var (ie begin())
var_iter::var_iter(in var1)
	: pvar_(&var1){
	if (!var1.len())
		startpos_ = std::string::npos;
}

//check iter != iter (i.e. iter != end()
bool var_iter::operator!=([[maybe_unused]] const var_iter& var_iter1) {
	//no need to use var_iter1 since the end is always string::npos;
	return this->startpos_ != std::string::npos;
}

//CONVERSION - conversion to var
var var_iter::operator*() const {

	//find the end of the field if not already known
	if (endpos_ == std::string::npos) {
		endpos_ = pvar_->var_str.find(FM_, startpos_);
	}

	var rvo;
	rvo.var_typ = VARTYP_STR;

	//extract the field
	if (endpos_ == std::string::npos)
		rvo.var_str = pvar_->var_str.substr(startpos_);
	else
		rvo.var_str = pvar_->var_str.substr(startpos_, endpos_ - startpos_);

	return rvo;
}

//INCREMENT
var_iter var_iter::operator++() {

	//find the end of the field if not already found from a call to above CONVERSION
	if (endpos_ == std::string::npos)
		endpos_ = pvar_->var_str.find(FM_, startpos_);

	//move up to the next field
	startpos_ = endpos_;

	//skip over any FM character
	if (startpos_ != std::string::npos) {
		//max str size = 9223372036854775807
		//string npos = 18446744073709551615
		//we will ignore the fact that we could be processing
		//a string of maximum size with a terminating FM
		//and incrementing would take us past maximum string size
		//but not beyond "no position" npos.
		startpos_++;
	}

	//indicate that the end of the next field is not yet known
	endpos_ = std::string::npos;

	return *this;
}

//DECREMENT PREFIX
var_iter var_iter::operator--() {

	// Decrement below zero is not allowed and throws if attempted
	if (startpos_ < 1)
		UNLIKELY
		throw VarError(__PRETTY_FUNCTION__);

	// Point to the separator before the current field
	// and record the new endpos as that pos
	startpos_--;
	endpos_ = startpos_;

	// The first char could be a FM in which case startpos and endpos will be 0
	// resulting in an empty field
	if (startpos_ == 0)
		return *this;

	// Skip before the separator
	startpos_--;

	// Find the separator marking the beginning of the previous field
	startpos_ = pvar_->var_str.rfind(FM_, startpos_);

	if (startpos_ == std::string::npos)
		// If not found then then new field starts from 0 the first char
		startpos_ = 0;
	else
		// new field starts one after the found separator
		startpos_++;

	return *this;
}

//INCREMENT POSTFIX
var_iter var_iter::operator++(int) {
	var_iter before = *this;
	++*this;
	return before;
}

//DECREMENT POSTFIX
var_iter var_iter::operator--(int) {
	var_iter before = *this;
	--*this;
	return before;
}

//BEGIN - var member function to create an iterator -> begin
var_iter var::begin() const {
	return var_iter(*this);
}

//END - var member function to create an interator -> end
var_iter var::end() const {
	// No need to use var1 since the end is always string::npos
	// so var_iter!=var_iter is implemented in terms of startpos_ != string::npos;
	return var_iter();
}

//PUBLIC void var_iter::operator=(SV replacementstr) {
//
//	// Find the end of the field if not already found from a call to above CONVERSION
//	if (endpos_ == std::string::npos)
//		endpos_ = pvar_->var_str.find(FM_, startpos_);
//
//	// Replace the field
//	if (endpos_ == std::string::npos) {
//		pvar_->var_str.resize(startpos_);
//		pvar_->var_str.append(replacementstr);
//		endpos_ = std::string::npos;
//	}
//	else {
//		pvar_->var_str.replace(startpos_, endpos_ - startpos_, replacementstr);
//		endpos_ = startpos_ + replacementstr.size() + 1;
//	}
//}
//

}  //namespace exo
