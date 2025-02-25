#ifndef VARDIM_H
#define VARDIM_H

#if EXO_MODULE
	import std;
#else
#	include <vector>
#	include <initializer_list>
#endif

#include <exodus/var.h>

namespace exo {

// Note that "for (var& var1: dim1)" with & allows direct access and update into the elements of the array dim1 via varx
// whereas "for (var var1 : dim1)" gives a copy of each element which is slower allows updating var1 without updating dim1
// Both cases are useful

// for (in v1 : d1)  ... v1 is a constant reference to the dim's vars ... efficient read only access to array elements
// for (io v1 : d1)  ... v1 is a ordinary reference to the dim's vars ... efficient read/write access to array elements
// for (var v1 : d1) ... v1 is a copy of the dim's vars               ... slower access and updating v1 does not update the array

//class dim final
class PUBLIC dim final {

friend class dim_iter;

 private:

	mutable int ncols_ = 0;
	std::vector<var> data_;

//    private:
//        std::vector<var>::iterator it_;

//	typename std::vector<var>::iterator it_;
//	using iterator = dim_iter;

 public:

	// TODO define in class for inline/optimisation?

	///////////////////////////
	// SPECIAL MEMBER FUNCTIONS
	///////////////////////////

	////////////////////////
	/// array construction :
	////////////////////////

	/////////////////////////
	// 1. Default constructor
	/////////////////////////

	// Create an undimensioned array.
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

	// Copy an array.
	dim(const dim& sourcedim) {
		// use copy assign
		*this = sourcedim;
	}

	//////////////////////
	// 4. move constructor - from rvalue
	//////////////////////

	// Save an array created elsewhere.
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

	// var& operator=(in rhs) & = default;
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

	// Create an array of vars with a fixed number of columns and rows. All elements are unassigned.
	dim(const int nrows, const int ncols = 1);

	// Resize an array to a different number of rows and columns.
	// Existing data will be retained as far as possible. Any additional elements are unassigned.
	// obj is d1
	void redim(const int nrows, const int ncols = 1);

    // Constructor from initializer_list for (int, double, cstr etc.)
	/////////////////////////////////////////////////////////////////
	template<class T>

	// Create an array from a list. All elements must be the same type, string, double or int.
	dim(std::initializer_list<T> list) {

		// Will not be called with zero elements
		//dim d {};

		//TRACE(var("dim constructor from initializer_list ") ^ int(list.size()));
		// list rows, ncols = 1
		// warning: conversion from ‘std::initializer_list<int>::size_type’ {aka ‘long int’} to ‘int’ may change value [-Wconversion]
		redim(static_cast<int>(list.size()), 1);

		// Allow arbitrary copying of element zero without throwing variable not assigned
		//data_[0].var_typ = VARTYP_STR;

		std::size_t itemno = 1;
		for (auto item : list) {
			data_[itemno++] = item;
		}
	}

	///////////////////
	// OTHER ASSIGNMENT
	///////////////////

	//=var
	// The assignment operator should always return a reference to *this.
	// cant be (in var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// contruction see also ^= etc
	void operator=(in sourcevar);
	void operator=(const int sourceint);
	void operator=(const double sourcedbl);

	/////////////////
	/// array access:
	/////////////////

	ND dim_iter begin();
	ND dim_iter end();

	// brackets operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression
	ND VARREF operator[](int rowno) {return getelementref(rowno, 1);}

	//following const version is called if we do () on a dim which was defined as const xx
	ND CVR operator[](int rowno) const {return getelementref(rowno, 1);}

	// Provide 2d version of bracket operator if c++23+
	// and deprecate the parenthesis operator
#if __cpp_multidimensional_subscript >= 202110L
    // Don't change woring without also changing it in cli/fixdeprecated
#	define DEPRECATED_PARENS [[deprecated("EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n]")]]
//#	define DEPRECATED_PARENS2
#	define DEPRECATED_PARENS2 [[deprecated("EXODUS: Replace multiple dimensioned array accessors like (x,y) with [x,y] e.g. dimarray(x,y) -> dimarray[x,y]")]]
	ND VARREF operator[](int rowno, int colno) {return getelementref(rowno, colno);}
	ND CVR operator[](int rowno, int colno) const {return getelementref(rowno, colno);}
#else
#	define DEPRECATED_PARENS [[deprecated("EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n]")]]
#	define DEPRECATED_PARENS2
#endif

	DEPRECATED_PARENS
	ND VARREF operator()(int rowno) {return getelementref(rowno, 1);}
	DEPRECATED_PARENS
	ND CVR operator()(int rowno) const {return getelementref(rowno, 1);}

	// parenthesis operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression
	//following const version is called if we do () on a dim which was defined as const xx
	DEPRECATED_PARENS2
	ND VARREF operator()(int rowno, int colno) {return getelementref(rowno, colno);}
	DEPRECATED_PARENS2
	ND CVR operator()(int rowno, int colno) const {return getelementref(rowno, colno);}

	// Transition alternative for () and [] syntax to be used in libexodus, cli, service and test.
	// Should be removed in 2028 when Ubuntu 24.04 is the oldest to be supported by exodus
	// DEPRECATED
	ND VARREF at(int rowno, int colno) {return getelementref(rowno, colno);}
	// DEPRECATED
	ND CVR at(int rowno, int colno) const {return getelementref(rowno, colno);}

	// obj is d1

	// Get the number of rows in the dimensioned array
	// Returns: A count
	ND var rows() const;

	// Get the number of columns in the dimensioned array
	// Returns: A count
	ND var cols() const;

	// Q: why is this commented out?
	// A: we dont want to COPY vars out of an array when using it in rhs expression
	// var operator() (int row, int col=1) const;

	// Joins all elements into a single delimited string
	// Returns: A string var
	ND var join(SV delimiter = _FM) const;

	///////////////////
	/// array mutation:
	///////////////////

	// Creates or updates the array from a given string.
	// If the dim array has not been dimensioned (nrows and ncols are 0), it will be dimensioned with the number of elements that the string has fields.
	// If the dim array has already been dimensioned, and has more elements than there are fields in the string, the excess array elements are initialised to "". If the record has more fields than there are elements in the array, the excess fields are all left unsplit in the final element of the array.
    // Predimensioning arrays allows efficient reuse of arrays in loops.
	// In either case, all elements of the array are updated.
	//
	// `dim d1;
	//  d1.splitter("f1^f2^f3"_var); // d1.rows() -> 3
	//  //
	//  dim d2(10);
	//  d2.splitter("f1^f2^f3"_var); // d2.rows() -> 10`
	dim& splitter(in str1, SV delimiter = _FM);

	// Sort the elements of the array in place.
	// reverseorder: Defaults to false. If true, then the order is reversed.
	dim& sorter(bool reverse = false);

	// Reverse the elements of the array in place.
	dim& reverser();

	// Randomly shuffle the order of the elements of the array in place.
	dim& shuffler();

//	dim& eraser(std::vector<var>::iterator iter1, std::vector<var>::iterator iter2) {data_.erase(iter1, iter2); return *this;}
//	dim& eraser(dim_iter dim_iter1, dim_iter dim_iter2) {data_.erase(&*dim_iter1, &*dim_iter2); return *this;}

	/////////////////////
	/// array conversion:
	/////////////////////

	// Same as sorter() but returns a new array leaving the original untouched.
	ND dim sort(bool reverseorder = false) const& {return dim(*this).sorter(reverseorder);}

	// Same as reverser() but returns a new array leaving the original untouched.
	ND dim reverse() const& {return dim(*this).reverser();}

	// Same as shuffler() but returns a new array leaving the original untouched.
	ND dim shuffle() const& {return dim(*this).shuffler();}

	// On temporaries the mutator functions are called.
	ND dim& sort(bool reverseorder = false) && {return this->sorter(reverseorder);}
	ND dim& reverse() && {return this->reverser();}
	ND dim& shuffle() && {return this->shuffler();}

	/////////////////////
	/// array read/write:
	/////////////////////

	// Writes a db file record created from an array.
	// Each element in the array becomes a separate field in the db record. Any redundant trailing FMs are suppressed.
	//
    // `dim d1 = "Client GD^G^20855^30000^1001.00^20855.76539"_var.split();
    //  let file = "xo_clients", key = "GD001";
    //  if (not deleterecord("xo_clients", "GD001")) {}; // Cleanup first
    //  d1.write(file, key);
    //  // or
    //  write(d1 on file, key);`
	void write(in dbfile, in key) const;

	// Read a db file record into an array.
	// Each field in the database record becomes a single element in the array.
	// Returns: True if the record exists or false if not,
	// If the array is predimensioned then any excess array elements are initialised to "" and any excess record fields are left unsplit in the final array element. See dim splitter for more info.
	// If the array is not predimensioned (rows and cols = 0) then it will be dimensioned to have exactly the same number of rows as there are fields in the record being read.
	//
    // `dim d1(10);
    //  let file = "xo_clients", key = "GD001";
    //  if (not d1.read(file, key)) ... // d1.join() -> "Client GD^G^20855^30000^1001.00^20855.76539"_var
    //  // or
    //  if (not read(d1 from file, key)) ...`
	ND bool read(in dbfile, in key);

	// Creates an entire os text file from an array
	// Each element of the array becomes one line in the os file delimited by \n (or \r\n if the array was originally osread with \r\n delimiter)
	// Any existing os file is overwritten and replaced.
	// codepage: Optional: Data is converted from UTF8 to the required codepage/encoding before output. If the conversion cannot be performed then return false.
	// Returns: True if successful or false if not.
	//
	// `dim d1 = "aaa=1\nbbb=2\nccc=3\n"_var.split("\n");
    //  if (not osremove("xo_conf.txt")) {}; // Cleanup first
	//  let osfilename = "xo_conf.txt";
	//  if (not d1.oswrite(osfilename)) ...
	//  // or
	//  if (not oswrite(d1 on osfilename)) ...`
	ND bool oswrite(in osfilename, const char* codepage = "") const;

	// Read an entire os text file into an array.
	// Each line in the os file, delimited by \n or \r\n, becomes a separate element in the array.
	// codepage: Optional. Data will be converted from the specified codepage/encoding to UTF8 after being read. If the conversion cannot be performed then return false.
	// Returns: True if successful or false if not.
	// If the first \n in the file is \r\n then the whole file will be split using \r\n as delimiter.
	//
	// `dim d1;
	//  let osfilename = "xo_conf.txt";
	//  if (not d1.osread(osfilename)) ... // d1.join("\n") -> "aaa=1\nbbb=2\nccc=3\n"_var0
	//  // or
	//  if (not osread(d1 from osfilename)) ...`
	ND bool osread(in osfilename, const char* codepage = "");

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
//	PUBLIC auto begin() {return ++data_.begin();}
//	PUBLIC auto end()   {return data_.end();}
//	PUBLIC std::iterator<var*, std::vector<var>> begin() {return ++data_.begin();}
//	PUBLIC __gnu_cxx::__normal_iterator<exo::var *, std::vector<exo::var>> begin() {return ++data_.begin();}
//	PUBLIC __gnu_cxx::__normal_iterator<exo::var *, std::vector<exo::var>> end()   {return data_.end();}
//	typedef typename std::vector<var>::iterator iter;
//	PUBLIC dim_iter begin() {return ++data_.begin();}
//	PUBLIC dim_iter end()   {return data_.end();}

	PUBLIC void push_back(var&& var1) {data_.push_back(std::move(var1));}

 private:

	dim& init(in var1);

	ND CVR getelementref(int row, int colno) const;
	ND VARREF getelementref(int row, int colno);

}; // class dim

} // namespace exo

#endif //VARDIM_H

