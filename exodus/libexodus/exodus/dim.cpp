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

#include <exodus/varimpl.h>
#include <exodus/dim.h>

// based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

// one based two dimensional array but (0,0) is a separate element set or got if either or both
// index is zero

namespace exodus {

// SPECIAL MEMBER FUNCTIONS

/////////////////////////
// 1. DEFAULT CONSTRUCTOR
/////////////////////////

// was declared private to prevent it being called but somehow still "dim xyz();" still compiles
// although with a warning now public to allow usage in class variables
dim::dim()
	: nrows_(0), ncols_(0), initialised_(false)
{
//TRACE("DF CON")
	try {
		//data_ = new var[nrows_ * ncols_ + 1];
		data_ = new var[1];
	}
	catch (const std::bad_alloc& e) {
		//TRACE("dim()")
		throw VarOutOfMemory("dim::dim() "_var ^ e.what());
	}

	//std::cout<< "created[] " << data_ << std::endl;
}

////////////////
// 2. DESTRUCTOR
////////////////

dim::~dim() {
//TRACE("DF DES")
	//std::cout<< "deleted[] " << data_ << std::endl;
	delete[] data_;
}

//////////////////////
// 3. COPY CONSTRUCTOR
//////////////////////

dim::dim(const dim& sourcedim) {
//TRACE("CP CON")

	// same as copy assign
	*this = sourcedim;
}

//////////////////////
// 4. MOVE CONSTRUCTOR
//////////////////////

dim::dim(dim&& sourcedim) {
//TRACE("MV CON")

	// same as copy assign
	*this = sourcedim;

//	// cannot copy an undimensioned array
//	if (!sourcedim.initialised_)
//		throw DimNotDimensioned("");
//
//	nrows_ = sourcedim.nrows_;
//	ncols_ = sourcedim.ncols_;
//	initialised_ = sourcedim.ncols_;
//	std::swap(data_, sourcedim.data_);
}

/////////////////////
// 5. COPY ASSIGNMENT
/////////////////////

VOID_OR_DIMREF dim::operator=(const dim& sourcedim) &{
//TRACE("CP ASS")
	// cannot copy an undimensioned array
	if (!sourcedim.initialised_)
		throw DimNotDimensioned("");

	this->redim(sourcedim.nrows_, sourcedim.ncols_);

	size_t ncells = nrows_ * ncols_ + 1;
	//for (int celln = 0; celln < ncells; ++celln)
	for (size_t celln = 1; celln < ncells; ++celln)
		//data_[celln] = sourcedim.data_[celln].clone();
		data_[celln] = sourcedim.data_[celln];
	return VOID_OR_THIS;
}

/////////////////////
// 6. MOVE ASSIGNMENT
/////////////////////

VOID_OR_DIMREF dim::operator=(dim&& sourcedim) & {
//TRACE("MV ASS")
	// cannot copy an undimensioned array
	if (!sourcedim.initialised_)
		throw DimNotDimensioned("");

	nrows_ = sourcedim.nrows_;
	ncols_ = sourcedim.ncols_;
	initialised_ = sourcedim.ncols_;
	std::swap(data_, sourcedim.data_);

	return VOID_OR_THIS;
}

// constructor - rows only
//dim::dim(const unsigned int rows)
//	: dim(rows, 1) {}

/////////////////////////////////////////////
// CONSTRUCTOR GIVEN NROWS AND OPTIONAL NCOLS
/////////////////////////////////////////////

dim::dim(const unsigned int rows, const unsigned int cols)
	: nrows_(rows), ncols_(cols), initialised_(true)
// data_ <--initialized below (after the 'if/throw' statement)
{

	// Prevent 0 dimensions
//	if (rows == 0 || cols == 0)
//		throw DimDimensionedZero();

	// Prevent zero elements
	// Especially because int*int can overflow to -1
	// and +1 makes it zero
	std::size_t nvars = rows * cols + 1;
	if (!nvars) {
		("dim("_var ^ var(rows) ^ ", " ^ var(cols) ^ ")").errputl();
		throw DimDimensionedZero("");
	}

	//std::clog << "new dim data var x " << nvars << std::endl;
	var("new dim data var x " ^ var(nvars)).logputl();

	// Allocate a new array of vars on the heap
	try {
		//data_ = new var[rows * cols + 1];
		data_ = new var[nvars];
		//std::clog << "new dim data_ at " << data_ << std::endl;
		//this->logputl("new dim data_ at " ^ var(data_);
	}
//	catch (const std::bad_alloc& e) {
//		var(e.what()).errputl();
//	}
	catch (...) {
		var("dim error").errputl();
		delete[] data_;
		data_ = nullptr;
	}

	if (!data_) {
		//TRACE("dim::dim("_var ^ var(nrows_) ^ ", " ^ var(ncols_) ^ ")")
		throw VarOutOfMemory("dim::dim("_var ^ var(nrows_) ^ ", " ^ var(ncols_) ^ ")");
	}

	//var(rows * cols + 1).errputl("created dim[] ");
}

VOID_OR_DIMREF dim::operator=(CVR sourcevar) {
	//if (!initialised_)
	//	throw DimNotDimensioned("");
	this->init(sourcevar);
	return VOID_OR_THIS	;
}

VOID_OR_DIMREF dim::operator=(const int sourceint) {
	this->init(sourceint);
	return VOID_OR_THIS;
}

VOID_OR_DIMREF dim::operator=(const double sourcedbl) {
	this->init(sourcedbl);
	return VOID_OR_THIS;
}

var dim::rows() const {
	if (!this->initialised_)
		throw DimNotDimensioned("");
	return nrows_;
}

var dim::cols() const {
	if (!this->initialised_)
		throw DimNotDimensioned("");
	return ncols_;
}

bool dim::redim(unsigned int rows, unsigned int cols) {

	// Allow redim(0, 0) to clear all date
//	if (rows == 0 || cols == 0)
//		throw DimDimensionedZero();

//	// do nothing if no change
//	if (this->initialised_ && rows == nrows_ && cols == ncols_)
//		return true;

	//std::cout<<"rows:"<<rows<<" "<<nrows_<<std::endl;
	//std::cout<<"cols:"<<cols<<" "<<ncols_<<std::endl;

	//(var(initialised_)^" "^var(nrows_)^" "^var(ncols_)^" -> "^var(rows)^"
	//"^var(cols)).outputl("redim=");

	// how exception safe is this?

	// 0. delete the old data for space
	//std::cout<< "deleted[] " << data_ << std::endl;
	if (data_) {
		delete[] data_;
		data_ = nullptr;
	}

	// 1. create new data
	try {
		data_ = new var[rows * cols + 1];
		//std::cout<< "created[] " << newdata << std::endl;
	}
	catch (const std::bad_alloc& e) {
		throw VarOutOfMemory("redim("_var ^ var(nrows_) ^ ", " ^ var(ncols_) ^ ") " ^ e.what());
	}

//	// 2. only then delete the old data
//	//std::cout<< "deleted[] " << data_ << std::endl;
//	delete[] data_;
//
//	// 3. and point to the new data
//	data_ = newdata;

	initialised_ = true;
	nrows_ = rows;
	ncols_ = cols;

	return true;
}

// the same () function is called regardless of being on LHS or RHS
// second version is IDENTICAL except for lack of const (used only on "const dim")
VARREF dim::operator()(unsigned int rowno, unsigned int colno) {

	// check bounds
	if (rowno > nrows_)
		throw DimIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ var(nrows_));
	if (colno > ncols_)
		throw DimIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ var(ncols_));

	if (rowno == 0 || colno == 0)
		return data_[0];

	return data_[ncols_ * (rowno - 1) + colno];
}

CVR dim::operator()(unsigned int rowno, unsigned int colno) const {

	// check bounds
	//if (rowno > nrows_ || rowno < 0)
	if (rowno > nrows_)
		throw DimIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ var(nrows_));

	//if (colno > ncols_ || colno < 0)
	if (colno > ncols_)
		throw DimIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ var(ncols_));

	if (rowno == 0 || colno == 0) {
		return (data_)[0];
	}

	return data_[ncols_ * (rowno - 1) + colno];
}

dim& dim::init(CVR sourcevar) {
	if (!initialised_)
		throw DimNotDimensioned("");
	size_t arraysize = nrows_ * ncols_ + 1;
	//for (int ii = 0; ii < arraysize; ii++)
	for (size_t ii = 1; ii < arraysize; ii++)
		data_[ii] = sourcevar;
	return *this;
}

var dim::join(SV sepchar) const {

	if (!initialised_)
		throw DimNotDimensioned("");

	size_t arraysize = nrows_ * ncols_;
	if (!arraysize)
		return "";

	// find last element with any data
	size_t nn;
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
	if (sepchar.size() == 1) {

		[[likely]];

		char sepbyte = sepchar[0];

		// append any additional elements
		for (size_t ii = 2; ii <= nn; ++ii) {
			output.var_str.push_back(sepbyte);
			output ^= data_[ii];
		}

	} else {

		// append any additional elements
		for (size_t ii = 2; ii <= nn; ++ii) {
			output.var_str += sepchar;
			output ^= data_[ii];
		}
	}

	return output;
}

// dim=var.split()
dim var::split(SV sepchar) const {

	//TODO provide a version that can split on any utf8 character
	// should use dim's move constructor to place the array directly in place avoiding a slow
	// deep copy and perhaps even copy/move elision to not even copy the base dim object (which
	// contains a pointer to an array of vars)
	dim tempdim2;
	tempdim2.split(*this, sepchar);
	return tempdim2;  //NRVO hopefully since single named return
					  //return dim().split(*this);//doesnt work because split returns a var, the number of fields
}

// number=dim.split(varstr)
// returns number of elements
var dim::split(CVR str1, SV sepchar) {

	THISIS("var dim::split(CVR var1, SV separator = FM)")
	ISSTRING(str1)

	size_t sepsize = sepchar.size();

	// maybe dimension to the size of the string
	// do NOT redimension always since pick matread/matparse do not
	// and we may get VNA accessing array elements if too few.
	if (!this->initialised_ || this->ncols_ != 1)
		this->redim(str1.count(sepchar) + 1);

	// empty string just fills array with empty string
	if (str1.length() == 0) {
		(*this) = "";
		return this->nrows_;
	}

	// start at the beginning and look for FM delimiters
	std::string::size_type start_pos = 0;
	std::string::size_type next_pos = 0;
	size_t fieldno;
	for (fieldno = 1; fieldno <= this->nrows_;/*no limit*/) {

		// find the next FM delimiter
		//next_pos = str1.var_str.find(FM_, start_pos);
		next_pos = str1.var_str.find(sepchar, start_pos);

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

	size_t nfields = fieldno;

	// stuff any excess fields into the last element
	if (next_pos != std::string::npos) {
		//this->data_[this->nrows_] ^= sepchar;
		this->data_[this->nrows_].var_str.append(sepchar);
		this->data_[this->nrows_].var_str.append(str1.var_str.substr(start_pos));
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

bool dim::read(CVR filehandle, CVR key) {

	THISIS("bool dim::read(CVR filehandle, CVR key)")
	ISSTRING(filehandle)
	ISSTRING(key)

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
	ISSTRING(filehandle)
	ISSTRING(key)

	var temprecord = this->join();
	return temprecord.write(filehandle, key);
}

bool dim::osread(CVR osfilename, const char* codepage) {\

	THISIS("bool dim::osread(CVR osfilename, const char* codepage DEFAULT_EMPTY)")
	ISSTRING(osfilename)
//	ISSTRING(codepage)

	var txt;
	if (not txt.osread(osfilename, codepage)) {
		var().lasterror(osfilename.quote() ^ " cannot be osread.");
		return false;
	}

	// Detect \r\n text
	var linesep = "\n";
	int first_nl = txt.index("\n");
	if (first_nl > 1 && txt[first_nl - 1] == "\r")
		linesep = "\r\n";

	//int n =
	this->split(txt, linesep);

	// Save the linesep in the unused element[0]
	// Will be used for oswrite
	data_[0] = linesep;

//	// If the first line ended with \r\n then we need to remove trailing \r from every element
//	if (!this->data_[1].var_str.empty() && this->data_[1].var_str.back() == '\r') {
//		for (size_t i = 1; i <= n; i++) {
//			if (!this->data_[i].var_str.empty())
//				//Assume all \n were preceeded by \r. No standalone \n chars.
//				this->data_[i].var_str.pop_back();
//		}
//	}

	return true;
}

bool dim::oswrite(CVR osfilename, const char* codepage) const {

	THISIS("bool dim::oswrite(CVR osfilename, const char* codepage DEFAULT_EMPTY)")
	ISSTRING(osfilename)
//	ISSTRING(codepage)

	//static char linesep = '\n';
	std::string linesep;
	if (data_[0].assigned() && data_[0].var_typ & VARTYP_STR)
		linesep = data_[0].var_str;
	else
		linesep = "\n";

	var txt = this->join(linesep);

	// Join suppresses trailing empty elements
	// So append a trailing linesep since text files should end with one
	txt.var_str.append(linesep);

	return txt.oswrite(osfilename, codepage);
}

//////////////
// var sorting
//////////////

// no speed or memory advantage since not sorting in place
// but provided for syntactical convenience avoiding need to assign output of sort()
VARREF var::sorter(SV sepchar) {
	(*this) = this->sort(sepchar);
	return *this;
}

//sorting var - using temporary dim
var var::sort(SV sepchar) const{

	THISIS("var var::sort(SV sepchar = FM)")
	assertString(function_sig);

	//perhaps is slower but sorts testing var < var linguistically but not natural numbers like 10a 2b

	//split into a temporary dim array for sorting
	//then join it back up into a single string

	return this->split(sepchar).sort().join(sepchar);

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

}  // namespace exodus
