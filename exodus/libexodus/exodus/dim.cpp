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

#if EXO_MODULE
	import std;
#	include <new> // for std::bad_alloc
#else
#	include <cstdlib>
#	include <algorithm>
#	include <random>
#	include <memory>
#endif

#include <exodus/varimpl.h>
#include <exodus/dim.h>

// based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

// one based two dimensional array but (0,0) is a separate element set or got if either or both
// index is zero

namespace exo {

	using RNG_typ = std::mt19937;
	extern thread_local std::unique_ptr<RNG_typ> thread_RNG;

	// Not currently used because we only provide begin/end iterator for dim
	// and these cannot change the number of vector elements, only reorder them
	//
	// Examples how to calculate nrows from overall array size
	// cols_ 3, data_ 7 (including element 0), rows = 7 - 1 -> 6, 6 / 3 -> 2 complete rows
	// cols_ 3, data_ 8 (including element 0), rows = 8 - 1 -> 7, 7 / 3 -> 2 complete rows plus 1 extra element
	// cols_ 3, data_ 1 (including element 0), rows = 1 - 1 -> 0, 0 / 3 -> 0 complete rows
	// cols_ 3, data_ 4 (including element 0), rows = 3 - 1 -> 2, 2 / 3 -> 0 complete rows plus 2 extra elements
	//#define EXO_DIM_RECALC_NROWS(DIM)\//
	//	auto data_size = static_cast</*unsigned*/ int>((DIM).data_.size());\//
	//	(DIM).nrows_ = data_size ? (--data_size / (DIM).ncols_) : 0;
#define EXO_DIM_RECALC_NROWS(DIM)

// SPECIAL MEMBER FUNCTIONS

/////////////////////
// 5. COPY ASSIGNMENT
/////////////////////

void dim::operator=(const dim& sourcedim) &{
	//TRACE("CP ASS")
	// cannot copy an undimensioned array
	if (!sourcedim.initialised_)
		UNLIKELY
		throw DimNotDimensioned("");

	//this->redim(sourcedim.nrows_, sourcedim.ncols_);
	EXO_DIM_RECALC_NROWS(sourcedim)
	// warning: implicit conversion loses integer precision: 'size_type' (aka 'unsigned long') to 'int' [-Wshorten-64-to-32]
	int nrows = static_cast<int>(sourcedim.data_.size() - 1) / sourcedim.ncols_;
	this->redim(nrows, sourcedim.ncols_);

	// Copy element 0 as well to allow a degree
	// zero-based indexing although split/join/sort/read/write
	// ignore the zeroth element
//	if (sourcedim.data_[0].assigned())
//		data_[0] = sourcedim.data_[0];
		data_[0] = sourcedim.data_[0].clone();

	// Use vector size in case some algorithm has adjusted it
	//std::size_t ncells = nrows_ * ncols_ + 1;
	std::size_t ncells = data_.size();

	//for (/*unsigned*/ int cell_index = 0; cell_index < ncells; ++cell_index)
	for (std::size_t cell_index = 1; cell_index < ncells; ++cell_index)
		data_[cell_index] = sourcedim.data_[cell_index].clone();
		//data_[cell_index] = sourcedim.data_[cell_index];
	return;
}

/////////////////////
// 6. MOVE ASSIGNMENT
/////////////////////

void dim::operator=(dim&& sourcedim) & {
	//TRACE("MV ASS")
	// cannot copy an undimensioned array
	if (!sourcedim.initialised_)
		UNLIKELY
		throw DimNotDimensioned("");

	EXO_DIM_RECALC_NROWS(sourcedim)
	ncols_ = sourcedim.ncols_;
	initialised_ = ncols_;
	std::swap(data_, sourcedim.data_);

	return;
}

// constructor - rows only
//dim::dim(const /*unsigned*/ int rows)
//	: dim(rows, 1) {}

/////////////////////////////////////////////
// CONSTRUCTOR GIVEN NROWS AND OPTIONAL NCOLS
/////////////////////////////////////////////

dim::dim(const /*unsigned*/ int rows, const /*unsigned*/ int cols)
	: ncols_(cols), initialised_(true)
// data_ <--initialized below (after the 'if/throw' statement)
{
	// Partially allow a kind of zero based indexing scheme to work for
	// Unidimensional arrays except that split/join/sort omit [0]th element

	// Prevent 0 elements
	//if (rows == 0 || cols == 0)
	// throw DimDimensionedZero();

	this->redim(rows, cols);
}

void dim::operator=(in sourcevar) {
	this->init(sourcevar);
	return;
}

void dim::operator=(const int sourceint) {
	this->init(sourceint);
	return;
}

void dim::operator=(const double sourcedbl) {
	this->init(sourcedbl);
	return;
}

var dim::rows() const {
	if (!this->initialised_)
		UNLIKELY
		throw DimNotDimensioned("");
	EXO_DIM_RECALC_NROWS(*this)
	var nrows = (data_.size() - 1 ) / ncols_;
	return nrows;
}

var dim::cols() const {
	if (!this->initialised_)
		UNLIKELY
		throw DimNotDimensioned("");
	return ncols_;
}

bool dim::redim(/*unsigned*/ int rows, /*unsigned*/ int cols) {

	// Allow redim(0, 0) to clear all date
//	if (rows == 0 || cols == 0)
//		UNLIKELY throw DimDimensionedZero();

//	// do nothing if no change
//	if (this->initialised_ && rows == nrows_ && cols == ncols_)
//		return true;

	//std::cout<<"rows:"<<rows<<" "<<nrows_<<std::endl;
	//std::cout<<"cols:"<<cols<<" "<<ncols_<<std::endl;

	//(var(initialised_)^" "^var(nrows_)^" "^var(ncols_)^" -> "^var(rows)^"
	//"^var(cols)).outputl("redim=");

//	if (initialised_ and data_[0].unassigned())
//		// Ensure element zero will not throw unassigned during resizing/copying etc.
//		data_[0] = "";

	// how exception safe is this?
	try {
		data_.resize(rows * cols + 1);
		//std::cout<< "created[] " << newdata << std::endl;
	}
	catch (const std::bad_alloc& e) {
		UNLIKELY
		throw VarOutOfMemory("redim(" ^ var(rows) ^ ", " ^ var(cols) ^ ") " ^ e.what());
	}

	initialised_ = true;
//	nrows_ = rows;
	ncols_ = cols;

	return true;
}

// the same function is called regardless of being on LHS or RHS
// Keep IDENTICAL body with the function below
VARREF dim::getelementref(/*unsigned*/ int rowno, /*unsigned*/ int colno) {

	// TODO deduplicate code by calling the const function a la Scott Myers Effective C++
	//return const_cast<int&>(const_cast<const Foo*>(this)->get());
	return const_cast<var&>(const_cast<const dim*>(this)->getelementref(rowno, colno));
/*
	EXO_DIM_RECALC_NROWS(*this)

	// check bounds
	if (rowno > nrows_)
		UNLIKELY
		throw DimIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ var(nrows_));
	if (colno > ncols_)
		UNLIKELY
		throw DimIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ var(ncols_));

	if (rowno == 0 || colno == 0)
		return data_[0];

	return data_[ncols_ * (rowno - 1) + colno];
*/
}

// Keep IDENTICAL body with the function above
CVR dim::getelementref(/*unsigned*/ int rowno, /*unsigned*/ int colno) const {

	EXO_DIM_RECALC_NROWS(*this)

	// check bounds
	//if (rowno > nrows_ || rowno < 0)

	int cell_index = ncols_ * (rowno - 1) + colno;

	// Handle out of range
	int ncells = static_cast<int>(data_.size());
	if (cell_index >= ncells || rowno < 1 || colno < 1 || colno > ncols_) {
		UNLIKELY

		// Allow access to special zero'th element
		if (rowno == 0 || colno == 0) {
			return (data_)[0];
		}

		// Verify within range
		// Prevent negative for now
		int nrows = (ncells - 2) / ncols_;
		if (rowno < 0 || rowno > nrows)
			throw DimIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ var(nrows));
		if (colno < 0 || colno > ncols_)
			throw DimIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ var(ncols_));
		throw DimIndexOutOfBounds("col:" ^ var(colno) ^ " row:" ^ var(rowno) ^ " but only " ^ var(data_.size() - 1) ^ "elements");
	}
	return data_[cell_index];
}

dim& dim::init(in sourcevar) {
	if (!initialised_)
		UNLIKELY
		throw DimNotDimensioned("");

	// Use vector size in case some algorithm has adjusted it
	//std::size_t data_size = nrows_ * ncols_ + 1;
	auto data_size = data_.size();

	// init starts from element[0]
	for (std::size_t elementn = 0; elementn < data_size; elementn++)
	//for (std::size_t elementn = 1; elementn < data_size; elementn++)
		data_[elementn] = sourcevar;

	return *this;
}

var dim::join(SV sepchar) const {

	if (!initialised_)
		UNLIKELY
		throw DimNotDimensioned("");

	// Use vector size in case some algorithm has adjusted it
	//std::size_t data_size = nrows_ * ncols_;
	auto data_size = data_.size();
	auto sepchar_size = sepchar.size();

	if (data_size < 2)
		return "";

	// find the last assigned element
	// and total output string size to we can reserve string size
	// and convert all elements to string
//	for (nelements = data_size; nelements > 0; --nelements) {
//		//if (data_[nelements].assigned() && data_[nelements].len())
//		if (data_[nelements].assigned())
//			break;
//	}
	std::size_t nchars_joined = 0;
	std::size_t nelements;
	for (nelements = 1; nelements < data_size; ++nelements) {
		//if (data_[nelements].assigned() && data_[nelements].len())
		const var& data_element = data_[nelements];
		if (!data_element.assigned()) {
			break;
		}
		nchars_joined += data_element.var_str.size() + sepchar_size;
	}
	--nelements;
	// The additional trailing sepchar_size can be left in the total
	// to allow for trailing char(0)
	//TRACE(nelements)
	//TRACE(nchars_joined)

	// join always starts from element[1] and ignores element[0]

	// get the first element at least to ensure
	// at least first element is assigned - even if it is an empty string
	var output = "";

	// if no elements
	if (!nelements)
		return output;

	// Reserve the total number of characters required. Avoid resizing/mallocs
	output.var_str.reserve(nchars_joined);

	// Ensure converted to a string by using concatenate operator
	output ^= data_[1];

	// When sepchar is one byte (usual case), use push_back for speed
	if (sepchar_size == 1) {

		LIKELY;

		char sepbyte = sepchar[0];

		// Append any additional elements. Single byte sepchar
		for (std::size_t elementn = 2; elementn <= nelements; ++elementn) {
			output.var_str.push_back(sepbyte);
			output ^= data_[elementn];
		}

	} else {

		// Append any additional elements. Multibyte sepchar
		for (std::size_t elementn = 2; elementn <= nelements; ++elementn) {
			output.var_str += sepchar;
			output ^= data_[elementn];
		}
	}

	return output;
}

// dim=var.split()
dim  var::split(SV sepchar) const {

	//TODO provide a version that can split on any utf8 character - perhaps if sepchar is ""

//	THISIS("dim  var::split(SV sepchar = FM)")
	assertString("dim  var::split(SV sepchar) const");

	// an undimensioned dim will be dimensioned automatically
	return dim().splitter(*this, sepchar);
}

// d.splitter(v, sep)
dim& dim::splitter(in str1, SV sepchar) {

	//TODO provide a version that can split on any utf8 character
	// Perhaps if sepchar is ""

	THISIS("dim& dim::splitter(in str1, SV sepchar = FM)")
	ISSTRING(str1)

	EXO_DIM_RECALC_NROWS(*this)

	// maybe dimension to the size of the string
	// do NOT redimension always since pick matread/matparse do not
	// and we may get VNA accessing array elements if too few.
	if (!this->initialised_ || this->ncols_ != 1)
		this->redim(str1.count(sepchar) + 1);

	// empty string just fills array with empty string
	if (str1.var_str.empty()) {
		(*this) = "";
		return *this;
	}

	// split always fills starting from element[1] and ignores element[0]

	// start at the beginning and look for sepchar delimiters
	std::size_t start_pos = 0;
	std::size_t next_pos = 0;
	std::size_t sepsize = sepchar.size();
	//std::size_t fieldno;
	int fieldno;
	int nrows = static_cast<int>(data_.size() - 1) / ncols_;
	for (fieldno = 1; fieldno <= nrows; fieldno++) {

		// find the next sepchar delimiter
		next_pos = str1.var_str.find(sepchar, start_pos);

		// not found - past end of string?
		if (next_pos == std::string::npos) {
			this->data_[fieldno] = str1.var_str.substr(start_pos);
			break;
		}

		// fill an element with a field
		this->data_[fieldno] = str1.var_str.substr(start_pos, next_pos - start_pos);

		// shift to start of next field
		start_pos = next_pos + sepsize;

	}

	//std::size_t nfields = fieldno;

	if (next_pos != std::string::npos) {

		// stuff any excess fields into the last element
		//this->data_[this->nrows_] ^= sepchar;
		this->data_[nrows].var_str.append(sepchar).append(str1.var_str.substr(start_pos));

	} else {

		// fill any remaining array elements with empty string
		++fieldno;
		for (; fieldno <= (nrows); ++fieldno)
			this->data_[fieldno] = "";
	}

	return *this;
}

dim& dim::sorter(bool reverseorder) {
	//THISIS("var dim::sorter(bool reverseorder = false)")

	// There is no different between sort and stable_sort here because we are using the default var < var operation to sort
	// std::sort(d.begin(), d.end() , [](auto x){return xxxxxx}) could be used to sort on part of each var

	// We must use dim's custom begin to skip element zero
	//note that _data[0] may be empty
	//std::cout<<nfields<<std::endl;
	//std::cout<<data_[0]<<std::endl;
	//std::sort(std::begin(data_), std::end(data_), reverseorder ? std::greater<var>{} : std::less<var>{});
	if (!reverseorder)
		//std::sort(std::begin(data_), std::end(data_));
		//std::sort(this->begin(), this->end());
		std::sort(this->begin(), this->end());
	else
		std::sort(this->begin(), this->end(), std::greater<var>());

	return *this;
}

dim& dim::reverser() {

	// We must use dim's custom begin to skip element zero
	//std::reverser(std::begin(data_), std::end(data_));
	std::reverse(this->begin(), this->end());

	return *this;
}

dim& dim::shuffler() {

	// We must use dim's custom begin to skip element zero
	//std::random_device rd;
	//std::mt19937 g(rd());
	// Create a base generator per thread on the heap. Will be destroyed on thread termination.
	if (not thread_RNG.get())
		var(0).initrnd();
	std::shuffle(this->begin(), this->end(), *thread_RNG);

	return *this;
}

bool dim::read(in filevar, in key) {

	THISIS("bool dim::read(in filevar, in key)")
	ISSTRING(filevar)
	ISSTRING(key)

	var temprecord;
	if (!temprecord.read(filevar, key))
		return false;

	// Dont use following because it redimensions the array to the actual number of fields found
	//(*this)=temprecord.split();

	// dim::splitter does not adjust the length of the array
	// If the record has fewer fields than the size of the array then the rest of the array is set to ""
	// If the record has more fields than the size of the array then they are crammed in the last element
	// Allowing usage like record(30) even though there were not 30 fields in the specific record
	this->splitter(temprecord);

	// var(nrows_).outputl("nrows now=");
	// this->join("|").outputl("read=");

	return true;
}

void dim::write(in filevar, in key) const {

	THISIS("void dim::write(in filevar, in key) const")
	ISSTRING(filevar)
	ISSTRING(key)

	var temprecord = this->join().trimlast(_FM);

	temprecord.write(filevar, key);
	return;
}

bool dim::osread(in osfilename, const char* codepage) {

	THISIS("bool dim::osread(in osfilename, const char* codepage DEFAULT_EMPTY)")
	ISSTRING(osfilename)

	var txt;
	if (not txt.osread(osfilename, codepage)) {
		var().setlasterror(osfilename.quote() ^ " cannot be osread.");
		return false;
	}

	std::string linesep = "\n";

	// Detect linesep \n or \r\n
	auto first_nl = txt.var_str.find('\n');
	if (first_nl != std::string::npos && first_nl > 0 && txt.var_str[first_nl - 1] == '\r')
		linesep = "\r\n";

	//TRACE(txt.replace("\n", "|"))
	*this = txt.split(linesep);
	//TRACE(nrows_)

	// Save the linesep in the unused element[0]
	// Will be used for oswrite
	data_[0] = linesep;

	return true;
}

bool dim::oswrite(in osfilename, const char* codepage) const {

	THISIS("bool dim::oswrite(in osfilename, const char* codepage DEFAULT_EMPTY)")
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
	//txt.var_str.append(linesep);

	return txt.oswrite(osfilename, codepage);
}

///////////
// var sort
///////////

// no speed or memory advantage since not sorting in place
// but provided for syntactical convenience avoiding need to assign output of sort()
IO   var::sorter(SV sepchar) REF {
	(*this) = this->split(sepchar).sort().join(sepchar);
	return THIS;
}

//sorting var - using temporary dim
var  var::sort(SV sepchar) const& {

	THISIS("var  var::sort(SV sepchar = FM)")
	assertString(function_sig);

	//perhaps is slower but sorts testing var < var linguistically but not natural numbers like 10a 2b

	//split into a temporary dim array for sorting
	//then join it back up into a single string
	var nrvo = this->split(sepchar).sort().join(sepchar);
	return nrvo;


}

//////////////
// var reverse
//////////////

// no speed or memory advantage since not sorting in place
// but provided for syntactical convenience avoiding need to assign output of reverse()
IO   var::reverser(SV sepchar) REF {
	(*this) = this->split(sepchar).reverse().join(sepchar);
	return THIS;
}

//reversing var - using temporary dim
var  var::reverse(SV sepchar) const& {

	THISIS("var  var::reverse(SV sepchar = FM)")
	assertString(function_sig);

	//split into a temporary dim array for reversing
	//then join it back up into a single string
	//var nrvo = this->split(sepchar).reverse().join(sepchar);

	const auto sepcharsize = sepchar.size();

	var nrvo = "";

	// Empty string
	if (var_str.empty())
		return nrvo;

	// Get a direct reference to the reversed std string for possible performance optimisation
	auto& nrvo_var_str = nrvo.var_str;

	// Prevent multiple allocations while reverse string is being built up
	nrvo_var_str.reserve(var_str.size());

	std::size_t startpos = var_str.size() - 1;

	if (sepcharsize == 1) {

		LIKELY;

		// Single byte sepchar
		//////////////////////

		const char sepchar1 = sepchar[0];

		for (;;) {

			// if sepchar _FM
			//  abc ... startpos = 2, nextpos = npos
			auto nextpos = var_str.rfind(sepchar1, startpos);
			if (nextpos == std::string::npos) {
				nrvo_var_str.append(var_str, 0, startpos + 1);
				break;
			}

			// if sepchar _FM
			//  _FM     ... startpos = 0, nextpos = 0
			//  _FM _FM ... startpos = 1, nextpos = 1

			auto nextpos2 = nextpos + 1;
			nrvo_var_str.append(var_str, nextpos2, startpos - nextpos2 + 1);

			nrvo_var_str.push_back(sepchar1);

			if (nextpos == 0)
				break;

			startpos = nextpos - 1;
		}

	} else {

		// Multi-byte sepchar
		/////////////////////

		for (;;) {

			// if sepchar _FM
			//  abc ... startpos = 2, nextpos = npos
			auto nextpos = var_str.rfind(sepchar, startpos);
			if (nextpos == std::string::npos) {
				nrvo_var_str.append(var_str, 0, startpos + 1);
				break;
			}

			// if sepchar _FM
			//  _FM     ... startpos = 0, nextpos = 0
			//  _FM _FM ... startpos = 1, nextpos = 1

			auto nextpos2 = nextpos + sepcharsize;
			nrvo_var_str.append(var_str, nextpos2, startpos - nextpos2 + 1);

			nrvo_var_str.append(sepchar);

			if (nextpos == 0)
				break;

			startpos = nextpos - 1;
		}

	}  // multibyte sepchar

	return nrvo;

}

///////////////
// var shuffler
///////////////

// No speed or memory advantage since not shuffling in place
// but provided for syntactical convenience avoiding need to assign output of shuffle()
IO   var::shuffler(SV sepchar) REF {
	(*this) = this->split(sepchar).shuffler().join(sepchar);
	return THIS;
}

// No speed or memory advantage since not shuffling in place
// but provided for syntactical convenience avoiding need to assign output of shuffle()
var  var::shuffle(SV sepchar) const& {
//	return this->split(sepchar).shuffler().join(sepchar);
	auto _ = this->split(sepchar);
	_.shuffler();
	return _.join(sepchar);
}

dim_iter dim::begin() {
	// + 1 to skip over zero'th element since dim access uses 1 based indexing
//	return &(data_[0]) + 1;
	return ++data_.begin();
}

dim_iter dim::end() {
//	return &(data_[0])+data_.size();
	return data_.end();
}

}  // namespace exo
