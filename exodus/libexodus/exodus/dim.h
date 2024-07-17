#ifndef VARDIM_H
#define VARDIM_H

import std;//#include <vector>
//module #include <initializer_list>;

#include <exodus/var.h>

namespace exo {

// Note that "for (var& var1: dim1)" with & allows direct access and update into the elements of the array dim1 via varx
// whereas "for (var var1 : dim1)" gives a copy of each element which is slower allows updating var1 without updating dim1
// Both cases are useful

//class dim final
class PUBLIC dim final {

friend class var;

 private:

	mutable /*unsigned*/ int nrows_, ncols_;
	std::vector<var> data_;
	bool initialised_ = false;

 public:

	// TODO define in class for inline/optimisation?

	///////////////////////////
	// SPECIAL MEMBER FUNCTIONS
	///////////////////////////

	/////////////////////////
	// 1. Default constructor
	/////////////////////////
	//
	// allow syntax "dim d;" to create an "unassigned" dim
	// allow default construction for class variables later resized in class methods
	dim() = default;

	////////////////
	// 2. Destructor
	////////////////

	// destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	// protected to prevent deriving from var since wish to save space and not provide virtual
	// destructor http://www.gotw.ca/publications/mill18.htm
	~dim() = default;

	//////////////////////
	// 3. Copy constructor - from lvalue
	//////////////////////
	//
	dim(const dim& sourcedim) {
		// use copy assign
		*this = sourcedim;
	}

	//////////////////////
	// 4. move constructor - from rvalue
	//////////////////////
	//
	dim(dim&& sourcedim) {
		// use move assign
		*this = std::move(sourcedim);
	}

	/////////////////////
	// 5. copy assignment - from lvalue
	/////////////////////

	// Not using copy assignment by value (copy-and-replace idiom)
	// because Howard Hinnant recommends against in our case

	// Prevent assigning to temporaries
	void operator=(const dim& rhs) && = delete;

	// var& operator=(CVR rhs) & = default;
	// Cannot use default copy assignment because
	// a) it returns a value allowing accidental use of "=" instead of == in if statements
	// b) doesnt check if rhs is assigned
	void operator=(const dim& rhs) &;

	/////////////////////
	// 6. move assignment - from rvalue
	/////////////////////

	// Prevent assigning to temporaries
	void operator=(dim&& rhs) && noexcept = delete;

	// Cannot use default move assignment because
	// a) it returns a value allowing accidental use of "=" in if statements instead of ==
	// b) doesnt check if rhs is assigned (less important for temporaries which are rarely unassigned)
	//var& operator=(TVR rhs) & noexcept = default;
	void operator=(dim&& rhs) &;

	/////////////////////
	// Other constructors
	/////////////////////

	// Constructor with number of rows and optional number of columns
	/////////////////////////////////////////////////////////////////
	dim(const /*unsigned*/ int nrows, const /*unsigned*/ int ncols = 1);

	bool redim(const /*unsigned*/ int nrows, const /*unsigned*/ int ncols = 1);

    // Constructor from initializer_list for (int, double, cstr etc.)
	/////////////////////////////////////////////////////////////////
	template<class T>
	dim(std::initializer_list<T> list) {

		// Will not be called with zero elements
		//dim d {};

		//TRACE(var("dim constructor from initializer_list ") ^ int(list.size()));
		// list rows, ncols = 1
		// warning: conversion from ‘std::initializer_list<int>::size_type’ {aka ‘long /*unsigned*/ int’} to ‘/*unsigned*/ int’ may change value [-Wconversion]
		redim(static_cast</*unsigned*/ int>(list.size()), 1);

		// Allow arbitrary copying of element zero without throwing variable not assigned
		//data_[0].var_typ = VARTYP_STR;

		int itemno = 1;
		for (auto item : list) {
			data_[itemno++] = item;
		}
	}

	///////////////////
	// OTHER ASSIGNMENT
	///////////////////

	//=var
	// The assignment operator should always return a reference to *this.
	// cant be (CVR var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// contruction see also ^= etc
	void operator=(CVR sourcevar);
	void operator=(const int sourceint);
	void operator=(const double sourcedbl);

	////////////
	// ACCESSORS
	////////////

	ND var join(SV sepchar = _FM) const;

	// brackets operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression
	ND VARREF operator[](/*unsigned*/ int rowno) {return getelementref(rowno, 1);}

	//following const version is called if we do () on a dim which was defined as const xx
	ND CVR operator[](/*unsigned*/ int rowno) const {return getelementref(rowno, 1);}

	// Provide 2d version of bracket operator if c++23+
	// and deprecate the parenthesis operator
#if __cpp_multidimensional_subscript >= 202110L
    // Don't change woring without also changing it in cli/convdeprecated
#	define DEPRECATED_PARENS [[deprecated("EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n]")]]
	ND VARREF operator[](/*unsigned*/ int rowno, /*unsigned*/ int colno) {return getelementref(rowno, colno);}
	ND CVR operator[](/*unsigned*/ int rowno, /*unsigned*/ int colno) const {return getelementref(rowno, colno);}
#else
//#	define DEPRECATED_PARENS
#	define DEPRECATED_PARENS [[deprecated("EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n]")]]
#endif

	DEPRECATED_PARENS ND VARREF operator()(/*unsigned*/ int rowno) {return getelementref(rowno, 1);}
	DEPRECATED_PARENS ND CVR operator()(/*unsigned*/ int rowno) const {return getelementref(rowno, 1);}

	// parenthesis operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression
	//following const version is called if we do () on a dim which was defined as const xx
	//DEPRECATED_PARENS
	ND VARREF operator()(/*unsigned*/ int rowno, /*unsigned*/ int colno) {return getelementref(rowno, colno);}
	//DEPRECATED_PARENS
	ND CVR operator()(/*unsigned*/ int rowno, /*unsigned*/ int colno) const {return getelementref(rowno, colno);}

	ND var rows() const;
	ND var cols() const;

	// Q: why is this commented out?
	// A: we dont want to COPY vars out of an array when using it in rhs expression
	// var operator() (int row, int col=1) const;

	ND dim sort(bool reverseorder = false) const& {return dim(*this).sorter(reverseorder);}
	ND dim& sort(bool reverseorder = false) && {return this->sorter(reverseorder);}

	ND dim reverse() const& {return dim(*this).reverser();}
	ND dim& reverse() && {return this->reverser();}

	ND dim shuffle() const& {return dim(*this).shuffler();}
	ND dim& shuffle() && {return this->shuffler();}

	///////////
	// MUTATORS
	///////////

	dim& sorter(bool reverse = false);
	dim& reverser();
	dim& shuffler();

	dim& splitter(CVR str1, SV sepchar = _FM);
	dim& eraser(std::vector<var>::iterator iter1, std::vector<var>::iterator iter2) {data_.erase(iter1, iter2); return *this;}

	/////////////
	// READ/WRITE
	/////////////

	// db
	ND bool read(CVR filevar, CVR key);
	bool write(CVR filevar, CVR key) const;

	// os
	ND bool osread(CVR osfilename, const char* codepage DEFAULT_EMPTY);
	ND bool oswrite(CVR osfilename,const char* codepage DEFAULT_EMPTY) const;

	////////////
	// ITERATORS
	////////////

	// Allow use of std algorithms
	//
	// Note that remove, remove_if, unique and other algorithms operate on a range of elements denoted by two forward iterators,
	// and have no knowledge of the underlying container or collection. Therefore they only move "removed" elements
	// to the end of the container. They return an iter to the first "removed" element so vector::erase can actually do the removal.
	//
	// begin() needs to skip the first vector element 0,0
	// because dim uses 1 based indexing
	// but we still allow use of vestigial dim(0)/dim(0, 0)
	//
	PUBLIC auto begin() {return ++data_.begin();}
	PUBLIC auto end()   {return data_.end();}

 private:

	dim& init(CVR var1);

	ND CVR getelementref(int row, int colno) const;
	ND VARREF getelementref(int row, int colno);

}; // class dim

} // namespace exo

#endif //VARDIM_H

