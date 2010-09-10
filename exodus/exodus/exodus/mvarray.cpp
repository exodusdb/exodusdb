/*
Copyright (c) 2009 Stephen John Bush

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

#ifndef MVARRAY_H
#define MVARRAY_H

#define MV_NO_NARROW

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

//based on http://geneura.ugr.es/~jmerelo/c++-faq/operator-overloading.html#faq-13.8

//one based two dimensional array but (0,0) is a separate element set or got if either or both index is zero

namespace exodus {

//was declared private to prevent it being called but somehow still "varray xyz();" still compiles although with a warning
//now public to allow usage in class variables
varray::varray()
   : rows_ (1)
   , cols_ (1)
   //data_ <--initialized below (after the 'if/throw' statement)
{
 //throw MVArrayDimensionedZero();
 data_ = new var[rows_ * cols_ + 1];
}

varray::~varray()
{
	delete[] data_;
}

varray::varray(int rows, int cols)
   : rows_ (rows)
   , cols_ (cols)
   //data_ <--initialized below (after the 'if/throw' statement)
{
  if (rows == 0 || cols == 0)
    throw MVArrayDimensionedZero();
  data_ = new var[rows * cols + 1];
}

bool varray::resize(int rows, int cols)
{
  if (rows == 0 || cols == 0)
    throw MVArrayDimensionedZero();
  rows_=rows;
  cols_=cols;
  //how exception safe is this?

  //create new data first
  var* newdata;
  newdata = new var[rows * cols + 1];

  //only then delete the old data
  delete[] data_;

  //and point to the new data
  data_ = newdata;

  return true;
}

//setting
//setting and getting is IDENTICAL except for var/var& return value
var& varray::operator() (int row, int col)
{

	//check bounds
	if (row > rows_)
		throw MVArrayIndexOutOfBounds(L"row:" ^ var(row) ^ L" > " ^ rows_);
	if (col > cols_)
		throw MVArrayIndexOutOfBounds(L"col:" ^ var(col) ^ L" > " ^ cols_);

	if (row ==0 || col == 0 )
	{
		row=0;
		col=0;
	}

	return data_[cols_*(row-1) + col];
}

//getting
//setting and getting is IDENTICAL except for var/var& return value
var varray::operator() (int row, int col) const
{

	//check bounds
	if (row > rows_)
		throw MVArrayIndexOutOfBounds(L"row:" ^ var(row) ^ L" > " ^ rows_);
	if (col > cols_)
		throw MVArrayIndexOutOfBounds(L"col:" ^ var(col) ^ L" > " ^ cols_);

	if (row ==0 || col == 0 )
	{
		row=0;
		col=0;
	}

	return data_[cols_*(row-1) + col];

}

varray& varray::init(const var& var1)
{
	int arraysize=rows_*cols_+1;
	for (int ii=0;ii<arraysize;ii++)
		data_[ii]=var1;
	return *this;
}

varray& varray::operator=(const varray& mva1)
{
	int arraysize=rows_*cols_+1;
	int arraysize2=mva1.rows_*mva1.cols_+1;
	if (arraysize2<arraysize)
		arraysize=arraysize2;
	for (int ii=0;ii<arraysize;ii++)
		data_[ii]=mva1.data_[ii];
	return *this;
}

}//of namespace exodus

#endif
