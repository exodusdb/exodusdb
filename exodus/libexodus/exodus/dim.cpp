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

// Originally based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

namespace exo {

	using RNG_typ = std::mt19937;
	extern thread_local std::unique_ptr<RNG_typ> thread_RNG;

// Copy an array (cloning)
dim::dim(const dim& rhs) : base(), ncols_(rhs.ncols_) {

	if (!rhs.ncols_)
		throw DimUndimensioned(__PRETTY_FUNCTION__);

	reserve(rhs.size());

	// Use var::clone() to copy unassigned vars too
	for (const var& v : rhs) {
		push_back(v.clone());
	}
}

// Copy assignment operator (cloning)
void dim::operator=(const dim& rhs) & {

	if (this == &rhs)
		return;

	if (!rhs.ncols_)
		throw DimUndimensioned(__PRETTY_FUNCTION__);

	clear(); // Clear existing elements
	reserve(rhs.size()); // Optimize allocation

	// Use var::clone() to copy unassigned vars too
	for (const var& v : rhs) {
		push_back(v.clone());
	}
	ncols_ = rhs.ncols_;

	return;
}

// Constructor given nrows and optional ncols
dim::dim(const int rows, const int cols)
{
	// Will throw an error if either is negative
	this->redim(rows, cols);
}

// Assign all = var
void dim::operator=(in sourcevar) {
	this->init(sourcevar);
	return;
}

// Assign all = int
void dim::operator=(const int sourceint) {
	this->init(sourceint);
	return;
}

// Assign all = double
void dim::operator=(const double sourcedbl) {
	this->init(sourcedbl);
	return;
}

// Get nrows
var  dim::rows() const {

	THISIS("var  dim::rows() const")

	if (!this->ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

	var nrows = base::size() / ncols_;

	return nrows;
}

// Get ncols
var  dim::cols() const {

	THISIS("var  dim::cols() const")

//	if (!this->ncols_) UNLIKELY
//		throw DimUndimensioned(function_sig);

	return ncols_;
}

// Redimension to new nrows and ncols, preserving data AFAP
void dim::redim(/*unsigned*/ int rows, /*unsigned*/ int cols) {

	THISIS("void dim::redim(int rows, int cols)")

	if (rows < 0)
		throw DimIndexOutOfBounds("rows:" ^ var(rows));

	if (cols < 0)
		throw DimIndexOutOfBounds("cols:" ^ var(cols));

	// how exception safe is this?
	try {
		int new_size = rows * cols;
		base::resize(new_size);
		//std::cout<< "created[] " << newdata << std::endl;
	}
	catch (const std::bad_alloc& e) {
		UNLIKELY
		throw VarOutOfMemory("redim(" ^ var(rows) ^ ", " ^ var(cols) ^ ") " ^ e.what());
	}

	ncols_ = cols;

	return;
}

// dim[] operator utility (non-const)
VARREF dim::getelementref(int rowno, int colno) {
	return const_cast<var&>(const_cast<const dim*>(this)->getelementref(rowno, colno));
}

// dim[] operator utility (const)
CVR  dim::getelementref(int rowno1, int colno1) const {

	THISIS("CVR  dim::getelementref(int rowno1, int colno1) const")

	if (!ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

    int nrows = base::size() / ncols_;

	int rowno = rowno1;
	int colno = colno1;

	// Throw error if rowno/coln not +/- nrows/ncols
	// Negative -1 -> last rown/coln
	// Convert negative into positive to access std::vector

	if (rowno < 0)
		rowno += nrows + 1;
	if (rowno < 1 || rowno > nrows)
		throw DimIndexOutOfBounds("row:" ^ var(rowno1) ^ " can be -" ^ var(nrows) ^ " to +" ^ nrows);

	if (colno < 0)
		colno += ncols_ + 1;
	if (colno < 1 || colno > ncols_)
		throw DimIndexOutOfBounds("col:" ^ var(colno1) ^ " can be -" ^ var(ncols_) ^  " to +" ^ ncols_);

	// Calculate index and throw error if out of range
	// row 1 col1 -> index 0
	int cell_index0 = ncols_ * (rowno - 1) + colno - 1;

	// Check bounds and throw DimIndexOutOfBounds
	int ncells = static_cast<int>(base::size());
	if (cell_index0 >= ncells) { UNLIKELY
		throw DimIndexOutOfBounds("row:" ^ var(rowno) ^ " col:" ^ var(colno) ^ " not within bounds (" ^ var(nrows) ^ ", " ^ var(ncols_) ^ ")");
	}

	return (base::operator[](cell_index0));
}

// Set all elements to a given var
dim& dim::init(in sourcevar) {

	THISIS("dim& dim::init(in sourcevar)")

	if (!ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

	for (var& v : *this) {
		v = sourcevar;
	}

	return *this;
}

// Return all elements joined up using a delimiter
var  dim::join(SV delimiter) const {

	THISIS("var  dim::join(SV delimiter) const");

	if (!ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

	auto delimiter_size = delimiter.size();

	// Stop at first unassigned element (var)

	// Predict total output string length
	std::size_t nchars_joined = 0;
	const var* iter0 = &(base::operator[](0));
	const var* iterx = iter0;
	const var* iend = iter0 + base::size();
	for (;iterx != iend && iterx->assigned(); ++iterx) {
		// Checking var.len() ensures var_str is available
		nchars_joined += std::size_t(iterx->len());
		nchars_joined += delimiter_size;
	}

	// The first row must be assigned but join with zero rows is OK
	if (not nchars_joined and base::size()) {
		if (not (base::operator[](0)).assigned())
			throw VarUnassigned(function_sig);
	}

	// Reserve the total number of characters required to avoid multiple resizing/mallocs
	var nrvo = "";
	nrvo.var_str.reserve(nchars_joined);

	if (delimiter_size == 1) {
		LIKELY;

		// 1. One byte delimiter

		char sepchar = delimiter[0];

		// Append any additional elements.
		for (auto iter2 = iter0; iter2 != iterx; ++iter2 ) {
			nrvo.var_str += iter2->var_str;
			nrvo.var_str.push_back(sepchar);
		}

		// Remove one trailing sepchar
		if (!nrvo.var_str.empty())
			nrvo.var_str.pop_back();;

	} else {

		// 2. Multibyte delimiter

		// Append any additional elements.
		for (auto iter2 = iter0; iter2 != iterx; ++iter2 ) {
			nrvo.var_str += iter2->var_str;
			nrvo.var_str += delimiter;
		}

		// Remove one trailing delimiter
		if (!nrvo.var_str.empty())
			nrvo.var_str.erase(nrvo.var_str.size() - delimiter_size);

	}

	return nrvo;
}

// var::split()
// d1 = v1.split()
dim  var::split(SV delimiter) const {

	// Will be dimensioned to hold the exact number of fields present
	dim nrvo;
	nrvo.splitter(*this, delimiter);
	return nrvo;
}

// d.splitter(v, sep)
void dim::splitter(in str1, SV delimiter) {

	//TODO If delimiter is "" split every utf8 chararacter?

	THISIS("dim& dim::splitter(in str1, SV delimiter)")
	ISSTRING(str1)

	// Only if not already dimensioned, dimension to the number of delimited fields present.
	// Do not always redimension always since often we are dim reading db records
	// and we may get VNA accessing array elements if too few.
	// Always return at least 1 element even if input is ""
	if (!this->ncols_) {
		this->redim(str1.count(delimiter) + 1);
		ncols_ = 1;
	}

	// Empty string just fills array with empty string
	if (str1.var_str.empty()) {
		(*this) = "";
		return;
	}

	// Start at the beginning and look for delimiter delimiters
	std::size_t start_pos = 0;
	std::size_t next_pos = 0;
	std::size_t sepsize = delimiter.size();
	int fieldno;
	int nrows = static_cast<int>(base::size()) / ncols_;
	for (fieldno = 0; fieldno < nrows; fieldno++) {

		// Find the next delimiter delimiter
		next_pos = str1.var_str.find(delimiter, start_pos);

		// Not found - past end of string?
		if (next_pos == std::string::npos) {
			base::operator[](fieldno) = str1.var_str.substr(start_pos);
			break;
		}

		// Fill an element with a field
		base::operator[](fieldno) = str1.var_str.substr(start_pos, next_pos - start_pos);

		// Shift to start of next field
		start_pos = next_pos + sepsize;

	}

	// Still more fields?
	if (next_pos != std::string::npos) {

		// Append all excess fields to the last array element without being split
		base::operator[](nrows - 1).var_str.append(delimiter).append(str1.var_str.substr(start_pos));

	} else {

		// Fill any remaining array elements with empty string
		++fieldno;
		for (; fieldno < nrows; ++fieldno)
			base::operator[](fieldno) = "";
	}

	return;
}

void dim::sorter(bool reverseorder) {

	THISIS("dim& dim::sorter(bool reverseorder)")
	if (!ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

	if (!reverseorder)
		std::sort(base::begin(), base::end());
	else
		std::sort(base::begin(), base::end(), std::greater<var>());

	return;
}

void dim::reverser() {

	THISIS("dim& dim::reverser()")
	if (!ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

	std::reverse(base::begin(), base::end());

	return;
}

void dim::shuffler() {

	THISIS("dim& dim::shuffler()")
	if (!ncols_) UNLIKELY
		throw DimUndimensioned(function_sig);

	// Create or reuse a base generator per thread on the heap.
	// Will be destroyed on thread termination.
	if (not thread_RNG.get())
		var(0).initrnd();

	std::shuffle(base::begin(), base::end(), *thread_RNG);

	return;
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

	// dim::splitter does not adjust the length of the array if it has already been dimensioned.
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

	THISIS("bool dim::osread(in osfilename, const char* codepage = "")")
	ISSTRING(osfilename)

	// Acquire the whole file as a single var.
	var txt;
	if (not txt.osread(osfilename, codepage)) {
		var().setlasterror(osfilename.quote() ^ " cannot be osread.");
		return false;
	}

	// Default to "\n" line sep
	// Detect \r\n from first \n
	std::string linesep = "\n";
	auto first_nl = txt.var_str.find('\n');
	if (first_nl != std::string::npos && first_nl > 0 && txt.var_str[first_nl - 1] == '\r')
		linesep = "\r\n";

	// Remove any previous array
	base::clear();

	// Force automatic redimensioning
	ncols_ = 0;

	this->splitter(txt.var_str, linesep);

	return true;
}

bool dim::oswrite(in osfilename, const char* codepage) const {

	THISIS("bool dim::oswrite(in osfilename, const char* codepage = "")")
	ISSTRING(osfilename)

	std::string linesep = "\n";

	var txt = this->join(linesep);

	return txt.oswrite(osfilename, codepage);
}

///////////
// var sort
///////////

// no speed or memory advantage since not sorting in place
// but provided for syntactical convenience avoiding need to assign output of sort()
IO   var::sorter(SV delimiter) REF {
	(*this) = this->split(delimiter).sort().join(delimiter);
	return THIS;
}

//sorting var - using temporary dim
var  var::sort(SV delimiter) const& {

	THISIS("var  var::sort(SV delimiter)")
	assertString(function_sig);

	//perhaps is slower but sorts testing var < var linguistically but not natural numbers like 10a 2b

	//split into a temporary dim array for sorting
	//then join it back up into a single string
	var nrvo = this->split(delimiter).sort().join(delimiter);
	return nrvo;

}

//////////////
// var reverse
//////////////

// no speed or memory advantage since not sorting in place
// but provided for syntactical convenience avoiding need to assign output of reverse()
IO   var::reverser(SV delimiter) REF {
	(*this) = this->split(delimiter).reverse().join(delimiter);
	return THIS;
}

//reversing var - not using dim
var  var::reverse(SV delimiter) const& {

	THISIS("var  var::reverse(SV delimiter)")
	assertString(function_sig);

	const auto delimitersize = delimiter.size();

	var nrvo = "";

	// Empty string
	if (var_str.empty())
		return nrvo;

	// Get a direct reference to the reversed std string for possible performance optimisation
	auto& nrvo_var_str = nrvo.var_str;

	// Prevent multiple allocations while reverse string is being built up
	nrvo_var_str.reserve(var_str.size());

	std::size_t startpos = var_str.size() - 1;

	if (delimitersize == 1) {

		LIKELY;

		// Single byte sepchar
		//////////////////////

		const char sepchar1 = delimiter[0];

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

		// Multi-byte delimiter
		/////////////////////

		for (;;) {

			// if delimiter _FM
			//  abc ... startpos = 2, nextpos = npos
			auto nextpos = var_str.rfind(delimiter, startpos);
			if (nextpos == std::string::npos) {
				nrvo_var_str.append(var_str, 0, startpos + 1);
				break;
			}

			// if delimiter _FM
			//  _FM     ... startpos = 0, nextpos = 0
			//  _FM _FM ... startpos = 1, nextpos = 1

			auto nextpos2 = nextpos + delimitersize;
			nrvo_var_str.append(var_str, nextpos2, startpos - nextpos2 + 1);

			nrvo_var_str.append(delimiter);

			if (nextpos == 0)
				break;

			startpos = nextpos - 1;
		}

	}  // multibyte delimiter

	return nrvo;

}

///////////////
// var shuffler
///////////////

// No speed or memory advantage since not shuffling in place
// but provided for syntactical convenience avoiding need to assign output of shuffle()
IO   var::shuffler(SV delimiter) REF {
	*this = this->split(delimiter).shuffle().join(delimiter);
	return THIS;
}

// No speed or memory advantage since not shuffling in place
// but provided for syntactical convenience avoiding need to assign output of shuffle()
var  var::shuffle(SV delimiter) const& {
	auto _ = this->split(delimiter);
	_.shuffler();
	return _.join(delimiter);
}

}  // namespace exo
