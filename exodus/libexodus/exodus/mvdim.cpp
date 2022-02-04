/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <algorithm>

#include <exodus/mv.h>

// based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

// one based two dimensional array but (0,0) is a separate element set or got if either or both
// index is zero

namespace exodus {

// was declared private to prevent it being called but somehow still "dim xyz();" still compiles
// although with a warning now public to allow usage in class variables
dim::dim()
	: nrows_(0), ncols_(0), initialised_(false)
// data_ <--initialized below (after the 'if/throw' statement)
{
	// throw MVArrayDimensionedZero();
	data_ = new var[nrows_ * ncols_ + 1];
	//std::cout<< "created[] " << data_ << std::endl;
}

dim::~dim() {
	//std::cout<< "deleted[] " << data_ << std::endl;
	delete[] data_;
}

// move contructor
dim::dim(dim&& sourcedim) noexcept {
	nrows_ = sourcedim.nrows_;
	ncols_ = sourcedim.ncols_;
	initialised_ = sourcedim.ncols_;
	data_ = sourcedim.data_;
}

dim::dim(int rows, int cols)
	: nrows_(rows), ncols_(cols), initialised_(true)
// data_ <--initialized below (after the 'if/throw' statement)
{
	if (rows == 0 || cols == 0)
		throw MVArrayDimensionedZero();
	data_ = new var[rows * cols + 1];
	//var(rows * cols + 1).errputl("created dim[] ");
}

bool dim::read(CVR filehandle, CVR key) {

	THISIS("bool dim::read(CVR filehandle, CVR key)")
	filehandle.assertString(functionname);
	key.assertString(functionname);

	var temprecord;
	if (!temprecord.read(filehandle, key))
		return false;

	// dont use following because it redimensions the array to the actual number of fields found
	// and this causes redim to clear the array when redim is in common.h and called
	// repetitively in subroutines
	//(*this)=temprecord.split();

	this->split(temprecord);

	// var(nrows_).outputl("nrows now=");

	// this->join("|").outputl("read=");
	return true;
}

bool dim::write(CVR filehandle, CVR key) const {

	THISIS("bool dim::write(CVR filehandle, CVR key) const")
	filehandle.assertString(functionname);
	key.assertString(functionname);

	var temprecord = this->join();
	return temprecord.write(filehandle, key);
}

bool dim::osread(CVR osfilename, CVR codepage /*=0*/) {\

	THISIS("bool dim::osread(CVR osfilename, CVR codepage = \"\")")
	osfilename.assertString(functionname);
	codepage.assertString(functionname);

	var txt;
	if (not txt.osread(osfilename, codepage)) {
		var().lasterror(osfilename.quote() ^ " cannot be osread.");
		return false;
	}

	int n = this->split(txt, '\n');

	// If the first line ended with \r\n then we need to remove trailing \r from every element
	if (!this->data_[1].var_str.empty() && this->data_[1].var_str.back() == '\r') {
		for (int i = 1; i <= n; i++) {
			if (!this->data_[i].var_str.empty())
				//Assume all \n were preceeded by \r. No standalone \n chars.
				this->data_[i].var_str.pop_back();
		}
	}

	return true;
}

bool dim::oswrite(CVR osfilename, CVR codepage /*=0*/) const {

	THISIS("bool dim::oswrite(CVR osfilename, CVR codepage = \"\")")
	osfilename.assertString(functionname);
	codepage.assertString(functionname);

	//TODO option for linesep to be \r\n
	static char linesep = '\n';

	var txt = this->join(linesep);

	// Join suppresses trailing empty elements
	// So append a trailing linesep since text files should end with one
	txt.var_str.push_back(linesep);

	return txt.oswrite(osfilename, codepage);
}

var dim::rows() const {
	if (!this->initialised_)
		throw MVArrayNotDimensioned();
	return nrows_;
}

var dim::cols() const {
	if (!this->initialised_)
		throw MVArrayNotDimensioned();
	return ncols_;
}

bool dim::redim(int rows, int cols) {

	if (rows == 0 || cols == 0)
		throw MVArrayDimensionedZero();

	// do nothing if no change
	if (initialised_ && rows == nrows_ && cols == ncols_)
		return true;

	//std::cout<<"rows:"<<rows<<" "<<nrows_<<std::endl;
	//std::cout<<"cols:"<<cols<<" "<<ncols_<<std::endl;

	//(var(initialised_)^" "^var(nrows_)^" "^var(ncols_)^" -> "^var(rows)^"
	//"^var(cols)).outputl("redim=");

	// how exception safe is this?

	// 1. create new data first
	var* newdata;
	newdata = new var[rows * cols + 1];
	//std::cout<< "created[] " << newdata << std::endl;

	// 2. only then delete the old data
	//std::cout<< "deleted[] " << data_ << std::endl;
	delete[] data_;

	// 3. and point to the new data
	data_ = newdata;

	initialised_ = true;
	nrows_ = rows;
	ncols_ = cols;

	return true;
}

// the same () function is called regardless of being on LHS or RHS
// second version is IDENTICAL except for lack of const (used only on "const dim")
VARREF dim::operator()(int rowno, int colno) {

	// check bounds
	if (rowno > nrows_)
		throw MVArrayIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ nrows_);
	if (colno > ncols_)
		throw MVArrayIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ ncols_);

	if (rowno == 0 || colno == 0)
		return data_[0];

	return data_[ncols_ * (rowno - 1) + colno];
}

VARREF dim::operator()(int rowno, int colno) const {

	// check bounds
	if (rowno > nrows_ || rowno < 0)
		throw MVArrayIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ nrows_);
	if (colno > ncols_ || colno < 0)
		throw MVArrayIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ ncols_);

	if (rowno == 0 || colno == 0) {
		return (data_)[0];
	}

	return data_[ncols_ * (rowno - 1) + colno];
}

dim& dim::init(CVR sourcevar) {
	if (!initialised_)
		throw MVArrayNotDimensioned();
	int arraysize = nrows_ * ncols_ + 1;
	for (int ii = 0; ii < arraysize; ii++)
		data_[ii] = sourcevar;
	return *this;
}

dim& dim::operator=(const dim& sourcedim) {
	// cannot copy an undimensioned array
	if (!sourcedim.initialised_)
		throw MVArrayNotDimensioned();

	this->redim(sourcedim.nrows_, sourcedim.ncols_);

	int ncells = nrows_ * ncols_ + 1;
	for (int celln = 0; celln < ncells; ++celln)
		data_[celln] = sourcedim.data_[celln].clone();
	return *this;
}

dim& dim::operator=(CVR sourcevar) {
	//if (!initialised_)
	//	throw MVArrayNotDimensioned();
	this->init(sourcevar);
	return *this;
}

dim& dim::operator=(const int sourceint) {
	//if (!initialised_)
	//	throw MVArrayNotDimensioned();
	init(sourceint);
	return *this;
}

dim& dim::operator=(const double sourcedbl) {
	//if (!initialised_)
	//	throw MVArrayNotDimensioned();
	this->init(sourcedbl);
	return *this;
}

var dim::join(CVR sepchar) const {
	if (!initialised_)
		throw MVArrayNotDimensioned();
	int arraysize = nrows_ * ncols_;
	if (!arraysize)
		return "";

	// find last element with any data
	int nn;
	for (nn = arraysize; nn > 0; --nn) {
		if (data_[nn].assigned() && data_[nn].length())
			break;
	}

	// get the first element at least to ensure
	// at least first element is assigned - even if it is an empty string
	var output = "";
	// ensuring converted to a string
	output ^= data_[1];

	// if no elements
	if (!nn)
		return output;

	//when sepchar is one byte (usual case), use push_back for speed
	if (sepchar.length() == 1) {
		char sepbyte = *sepchar[1].data();

		// append any additional elements
		for (int ii = 2; ii <= nn; ++ii) {
			output.var_str.push_back(sepbyte);
			output ^= data_[ii];
		}
	} else {
		std::string sepstring = sepchar.toString();

		// append any additional elements
		for (int ii = 2; ii <= nn; ++ii) {
			output.var_str += sepstring;
			output ^= data_[ii];
		}
	}

	return output;
}

// dim=var.split()
dim var::split(CVR separator) const {

	//TODO provide a version that can split on any utf8 character
	// should use dim's move constructor to place the array directly in place avoiding a slow
	// deep copy and perhaps even copy/move elision to not even copy the base dim object (which
	// contains a pointer to an array of vars)
	dim tempdim2;
	tempdim2.split(*this, separator);
	return tempdim2;  //NRVO hopefully since single named return
					  //return dim().split(*this);//doesnt work because split returns a var, the number of fields
}

// number=dim.split(varstr)
// returns number of elements
var dim::split(CVR str1, CVR separator) {

	THISIS("var dim::split(CVR var1, CVR separator=\"\")")
	str1.assertString(functionname);
	separator.assertString(functionname);

	//TODO template another version to be fast for single byte separator esp. default FM_
	//var sep = separator.var_str.size() ? separator : FM_;
	var sep = separator.var_str.size() ? separator : FM;
	int sepsize = sep.var_str.size();

	// maybe dimension to the size of the string
	// do NOT redimension always since pick matread/matparse do not
	// and we may get VNA accessing array elements if too few.
	if (!this->initialised_ || this->ncols_ != 1)
		this->redim(str1.count(sep) + 1);

	// empty string just fills array with empty string
	if (str1.length() == 0) {
		(*this) = "";
		return this->nrows_;
	}

	// start at the beginning and look for FM delimiters
	std::string::size_type start_pos = 0;
	std::string::size_type next_pos = 0;
	int fieldno;
	for (fieldno = 1; fieldno <= this->nrows_;/*no limit*/) {

		// find the next FM delimiter
		//next_pos = str1.var_str.find(FM_, start_pos);
		next_pos = str1.var_str.find(sep.var_str, start_pos);

		// not found - past end of string?
		if (next_pos == std::string::npos) {
			this->data_[fieldno] = str1.var_str.substr(start_pos);
			break;
		}

		// fill an element with a field
		this->data_[fieldno] = str1.var_str.substr(start_pos, next_pos - start_pos);

		start_pos = next_pos + sepsize;
		fieldno++;

	}

	int nfields = fieldno;

	// stuff any excess fields into the last element
	if (next_pos != std::string::npos) {
		//this->data_[this->nrows_] ^= FM ^ str1.var_str.substr(start_pos);
		this->data_[this->nrows_] ^= sep ^ str1.var_str.substr(start_pos);
	} else {
		++fieldno;
		// fill any remaining array elements with empty string
		for (; fieldno <= (this->nrows_); ++fieldno)
			this->data_[fieldno] = "";
	}

	return nfields;	 //NRVO hopefully since single named return and/or requires a conversion from int to var
}

dim& dim::sort(bool reverse) {
	//THISIS("var dim::sort(bool reverse = false)")

	//note that _data[0] may be empty
	//std::cout<<nfields<<std::endl;
	//std::cout<<data_[0]<<std::endl;
	if (!reverse)
		std::sort(data_ + 1, data_ + this->nrows_ * this->ncols_ + 1);
	else
		std::sort(data_ + 1, data_ + this->nrows_ * this->ncols_ + 1, std::greater<var>());

	return *this;
}

//sorting var - using temporary dim
var var::sort(CVR separator) const{

	THISIS("var var::sort(CVR separator=UNASSIGNED)")
	assertString(functionname);
	separator.assertString(functionname);

	//perhaps has slow but sorts alphanumerically by testing var<var
	//eg bb cc aa 2 10 -> aa bb cc 10 2
	//split return a temporary dim array that is sorted then joined back up again
	var sep2 = separator ?: FM;
	return this->split(sep2).sort().join(sep2);

}

}  // namespace exodus
