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

#define MV_NO_NARROW

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

//based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

//one based two dimensional array but (0,0) is a separate element set or got if either or both index is zero

namespace exodus {

//was declared private to prevent it being called but somehow still "dim xyz();" still compiles although with a warning
//now public to allow usage in class variables
dim::dim()
   : nrows_ (1)
   , ncols_ (1)
   , initialised_(false)
   //data_ <--initialized below (after the 'if/throw' statement)
{
 //throw MVArrayDimensionedZero();
 data_ = new var[nrows_ * ncols_ + 1];
}

dim::~dim()
{
	delete[] data_;
}

dim::dim(int rows, int cols)
	: nrows_ (rows)
	, ncols_ (cols)
	, initialised_(true)
	//data_ <--initialized below (after the 'if/throw' statement)
{
	if (rows == 0 || cols == 0)
		throw MVArrayDimensionedZero();
	data_ = new var[rows * cols + 1];
}

bool dim::read(const var& filehandle, const var& key)
{
	THISIS(L"bool dim::matread(const var& filehandle, const var& key)")
	ISSTRING(filehandle)
	ISSTRING(key)

	var temprecord;
	if (!temprecord.read(filehandle,key))
		return false;
	(*this).parse(temprecord);
	return true;
}

bool dim::write(const var& filehandle, const var& key) const
{
	THISIS(L"bool dim::matwrite(const var& filehandle, const var& key) const")
	ISSTRING(filehandle)
	ISSTRING(key)

	var temprecord=(*this).unparse();
	return temprecord.write(filehandle,key);
}

bool dim::redim(int rows, int cols)
{

  if (rows == 0 || cols == 0)
    throw MVArrayDimensionedZero();

  //how exception safe is this?

  //create new data first
  var* newdata;
  newdata = new var[rows * cols + 1];

  //only then delete the old data
  if (initialised_)
	  delete[] data_;

  //and point to the new data
  initialised_=true;
  data_ =  newdata;

  nrows_=rows;
  ncols_=cols;

  return true;

}

//the same () function is called regardless of being on LHS or RHS
//second version is IDENTICAL except for lack of const (used only on "const dim")
var& dim::operator() (int rowno, int colno)
{

	//check bounds
	if (rowno > nrows_)
		throw MVArrayIndexOutOfBounds(L"row:" ^ var(rowno) ^ L" > " ^ nrows_);
	if (colno > ncols_)
		throw MVArrayIndexOutOfBounds(L"col:" ^ var(colno) ^ L" > " ^ ncols_);

	if (rowno ==0 || colno == 0 )
		return data_[0];

	return data_[ncols_*(rowno-1) + colno];
}

var& dim::operator() (int rowno, int colno) const
{

	//check bounds
	if (rowno > nrows_)
		throw MVArrayIndexOutOfBounds(L"row:" ^ var(rowno) ^ L" > " ^ nrows_);
	if (colno > ncols_)
		throw MVArrayIndexOutOfBounds(L"col:" ^ var(colno) ^ L" > " ^ ncols_);

	if (rowno ==0 || colno == 0 )
	{
		return (data_)[0];
	}

	return data_[ncols_*(rowno-1) + colno];

}

dim& dim::init(const var& var1)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	int arraysize=nrows_*ncols_+1;
	for (int ii=0;ii<arraysize;ii++)
		data_[ii]=var1;
	return *this;
}

dim& dim::operator=(const dim& dim2)
{

	//cannot copy an undimensioned array
	if (!dim2.initialised_)
		throw MVArrayNotDimensioned();

	//can copy to an undimensioned array (duplicates the dimensions)
	if (!initialised_)
	{
		(*this).redim(dim2.nrows_,dim2.ncols_);

		//fast copy without rows and cols
		int ncells=nrows_*ncols_+1;
		for (int celln=0;celln<ncells;++celln)
				(data_[celln]).clone(dim2.data_[celln]);
		return *this;

	}

	//element 0,0 is extra in the 1 based world of mv dimensioned arrays
	(data_[0]).clone(dim2.data_[0]);

	int maxrown=dim2.nrows_;
	int maxcoln=dim2.ncols_;
	for (int rown=0;rown<nrows_;++rown)
	{
		int index=rown*ncols_;
		for (int coln=1;coln<=ncols_;++coln)
		{
			++index;
			if (rown<maxrown&&coln<maxcoln)
			{
				int fromindex=rown*maxcoln+1;
				(data_[index]).clone(dim2.data_[fromindex]);
			}
			else data_[index]=L"";
		}

	}

	return *this;
}

dim& dim::operator=(const var& var1)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	init(var1);
	return *this;
}

dim& dim::operator=(const int int1)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	init(int1);
	return *this;
}

dim& dim::operator=(const double dbl1)
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	init(dbl1);
	return *this;
}

var dim::unparse() const
{
	if (!initialised_)
		throw MVArrayNotDimensioned();
	int arraysize=nrows_*ncols_;
	if (!arraysize)
		return L"";

	var output=data_[1];

	for (int ii=2;ii<=arraysize;++ii)
	{
		output.var_mvstr.push_back(FM_);
		output^=data_[ii];
	}

	return output;
}

}//of namespace exodus
