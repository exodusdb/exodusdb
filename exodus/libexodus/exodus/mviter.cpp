#include <mv.h>

//mv_iter class enables c++ range based programming over a dynamic array
//
//uses FM as field separator at the moment
//
//Example:
//
// for (CVR v : dynstr) {...}
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

///////////
// var_iter
///////////

//CONSTRUCTOR from a var (ie begin())
var_iter::var_iter(CVR var1)
	: pvar_(&var1){
	if (!var1.length())
		startpos_ = std::string::npos;
	//std::cerr << __PRETTY_FUNCTION__ << std::endl;
}

//check iter != iter (i.e. iter != end()
bool var_iter::operator!=([[maybe_unused]] var_iter& var_iter1) {
	//std::cerr << __PRETTY_FUNCTION__ << std::endl;
	//no need to use var_iter1 since the end is always string::npos;
	return this->startpos_ != std::string::npos;
}

//CONVERSION - conversion to var
var var_iter::operator*() const {
	//std::cerr << __PRETTY_FUNCTION__ << std::endl;

	//find the end of the field if not already known
	if (endpos_ == std::string::npos) {
		endpos_ = pvar_->var_str.find(FM_, startpos_);
	}

	//extract the field
	if (endpos_ == std::string::npos)
		return pvar_->var_str.substr(startpos_);
	else
		return pvar_->var_str.substr(startpos_, endpos_ - startpos_);
}

//INCREMENT
var_iter var_iter::operator++() {
	//std::cerr << __PRETTY_FUNCTION__ << std::endl;

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

//BEGIN - free function to create an iterator -> begin
PUBLIC var_iter begin(CVR var1) {
	//std::cerr << __PRETTY_FUNCTION__ << std::endl;
	return var_iter(var1);
}

//END - free function to create an interator -> end
PUBLIC var_iter end([[maybe_unused]] CVR var1) {
	//std::cerr << __PRETTY_FUNCTION__ << std::endl;
	// No need to use var1 since the end is always string::npos
	// so var_iter!=var_iter is implemented in terms of startpos_ != string::npos;
	return var_iter();
}

///////////
// dim_iter
///////////

//CONSTRUCTOR from a dim (ie begin())
dim_iter::dim_iter(const dim& d1)
	: pdim_(&d1){}

//check iter != iter (i.e. iter != end()
bool dim_iter::operator!=(const dim_iter& dim_iter1) {
	return this->index_ != dim_iter1.index_;
}

//CONVERSION - conversion to var
//dim_iter::operator var*() {
var& dim_iter::operator*() {
	return pdim_->data_[index_];
}

//INCREMENT
dim_iter dim_iter::operator++() {

	index_++;

	return *this;
}

//DECREMENT
dim_iter dim_iter::operator--() {

	index_--;

	return *this;
}

void dim_iter::end() {
	index_ = pdim_->nrows_ * pdim_->ncols_ + 1;
}

//BEGIN - free function to create an iterator -> begin
PUBLIC dim_iter begin(const dim& d1) {
	return dim_iter(d1);
}

//END - free function to create an interator -> end
PUBLIC dim_iter end(const dim& d1) {
	dim_iter diter1(d1);
	diter1.end();
	return diter1;
}

}  //namespace exodus


