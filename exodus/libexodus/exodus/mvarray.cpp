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

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)

#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

// based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

// one based two dimensional array but (0,0) is a separate element set or got if either or both
// index is zero

namespace exodus
{

// was declared private to prevent it being called but somehow still "dim xyz();" still compiles
// although with a warning now public to allow usage in class variables
dim::dim() : nrows_(1), ncols_(1), initialised_(false)
// data_ <--initialized below (after the 'if/throw' statement)
{
	// throw MVArrayDimensionedZero();
	data_ = new var[nrows_ * ncols_ + 1];
}

dim::~dim() { delete[] data_; }

// move contructor
dim::dim(dim&& sourcedim) noexcept
{
	nrows_ = sourcedim.nrows_;
	ncols_ = sourcedim.ncols_;
	initialised_ = sourcedim.ncols_;
	data_ = sourcedim.data_;
}

dim::dim(int rows, int cols) : nrows_(rows), ncols_(cols), initialised_(true)
// data_ <--initialized below (after the 'if/throw' statement)
{
	if (rows == 0 || cols == 0)
		throw MVArrayDimensionedZero();
	data_ = new var[rows * cols + 1];
}

// dim split is defined in mvmv.cpp
// var dim::split(const var& str1)

bool dim::read(const var& filehandle, const var& key)
{
	THISIS("bool dim::matread(const var& filehandle, const var& key)")
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

bool dim::write(const var& filehandle, const var& key) const
{
	THISIS("bool dim::matwrite(const var& filehandle, const var& key) const")
	ISSTRING(filehandle)
	ISSTRING(key)

	var temprecord = this->join();
	return temprecord.write(filehandle, key);
}

bool dim::redim(int rows, int cols)
{

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

	// 2. only then delete the old data
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
var& dim::operator()(int rowno, int colno)
{

	// check bounds
	if (rowno > nrows_)
		throw MVArrayIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ nrows_);
	if (colno > ncols_)
		throw MVArrayIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ ncols_);

	if (rowno == 0 || colno == 0)
		return data_[0];

	return data_[ncols_ * (rowno - 1) + colno];
}

var& dim::operator()(int rowno, int colno) const
{

	// check bounds
	if (rowno > nrows_ || rowno < 0)
		throw MVArrayIndexOutOfBounds("row:" ^ var(rowno) ^ " > " ^ nrows_);
	if (colno > ncols_ || colno < 0)
		throw MVArrayIndexOutOfBounds("col:" ^ var(colno) ^ " > " ^ ncols_);

	if (rowno == 0 || colno == 0)
	{
		return (data_)[0];
	}

	return data_[ncols_ * (rowno - 1) + colno];
}

dim& dim::init(const var& sourcevar)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	int arraysize = nrows_ * ncols_ + 1;
	for (int ii = 0; ii < arraysize; ii++)
		data_[ii] = sourcevar;
	return *this;
}

dim& dim::operator=(const dim& sourcedim)
{
	// cannot copy an undimensioned array
	if (!sourcedim.initialised_)
		throw MVArrayNotDimensioned();

	this->redim(sourcedim.nrows_, sourcedim.ncols_);

	int ncells = nrows_ * ncols_ + 1;
	for (int celln = 0; celln < ncells; ++celln)
		data_[celln] = sourcedim.data_[celln].clone();
	return *this;
}

dim& dim::operator=(const var& sourcevar)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	init(sourcevar);
	return *this;
}

dim& dim::operator=(const int sourceint)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	init(sourceint);
	return *this;
}

dim& dim::operator=(const double sourcedbl)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	init(sourcedbl);
	return *this;
}

var dim::join(const var& sepchar) const
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	int arraysize = nrows_ * ncols_;
	if (!arraysize)
		return "";

	// find last element with any data
	int nn;
	for (nn = arraysize; nn > 0; --nn)
	{
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
	if (sepchar.length()==1)
	{
		char sepbyte=*sepchar[1].data();

		// append any additional elements
		for (int ii = 2; ii <= nn; ++ii)
		{
			output.var_str.push_back(sepbyte);
			output ^= data_[ii];
		}
	}
	else
	{
		std::string sepstring=sepchar;

		// append any additional elements
		for (int ii = 2; ii <= nn; ++ii)
		{
			output.var_str += sepstring;
			output ^= data_[ii];
		}
	}

	return output;
}

} // namespace exodus
