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

// Inheriting from std::vector which doesnt have a virtual destructor
// and adding a data member is dangerous if a dim is converted to its base class for destruction.
// Therefore do not convert dims to std::vector<var>.

//class dim final
class PUBLIC dim final : public std::vector<var> {
//class PUBLIC dim final : public std::vector<var_base<var_mid<exo::var>>> {

friend class dim_iter;

 private:

	mutable int ncols_ = 0;
//	std::vector<var> data_;

//    private:
//        std::vector<var>::iterator it_;

//	typename std::vector<var>::iterator it_;
//	using iterator = dim_iter;

	// Alias for the base class
	using base = std::vector<var>;
//	using base = std::vector<var_base<var_mid<exo::var>>>;

 public:

    // New Unpack function to allow "auto [a,b,c] = d1.unpack<3>();"
    // Lvalue version: Copy strings (const MyVec&)
    template <size_t N>
    auto unpack() const& {
        auto fill_tuple = [this]<size_t... Is>(std::index_sequence<Is...>) {
            return std::make_tuple(
                (Is < base::size() ? base::operator[](Is) : var(""))... // Copy
            );
        };
        return fill_tuple(std::make_index_sequence<N>{});
    }
    // Rvalue version: Move strings (MyVec&&)
    template <size_t N>
    auto unpack() && {
        auto fill_tuple = [this]<size_t... Is>(std::index_sequence<Is...>) {
            return std::make_tuple(
                (Is < base::size() ? std::move(base::operator[](Is)) : var(""))... // Move
            );
        };
        return fill_tuple(std::make_index_sequence<N>{});
    }


	// inherit constructors
//	using base::vector;

	// TODO define in class for inline/optimisation?

	///////////////////////////
	// SPECIAL MEMBER FUNCTIONS
	///////////////////////////

	//////////////////////////////////////
	///// dimensioned array construction :
	//////////////////////////////////////

	/////////////////////////
	// 1. Default constructor
	/////////////////////////

	// Create an undimensioned array of vars.
	// Pending actual dimensions by redim, read, osread or split.
	//
	// `dim d1;`
	//
	dim() = default;

	// Default constructor
	//dim() : base(), ncols_(0) {}

	// FIXED CONSTRUCTOR

	// Create a dimensioned array of vars.
	// Specify a fixed number of columns and rows. All vars are unassigned.
	//
	// `dim d1(10);
	//  dim d2(10, 3);`
	//
	dim(const int nrows, const int ncols = 1);

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

//	dim(const dim& sourcedim) {
//		// use copy assign
//		TRACE(__PRETTY_FUNCTION__)
//		*this = sourcedim;
//	}

	// Copy a dimensioned array.
	// Unassigned elements are copied as is.
	//
	// `dim d1 = {2, 4, 6, 8};
	//  dim d2 = d1;`
	//
	dim(const dim& rhs);

	//////////////////////
	// 4. move constructor - from rvalue
	//////////////////////

//	dim(dim&& sourcedim) {
//		// use move assign
//		*this = std::move(sourcedim);
//	}

	// Save a dimensioned array created elsewhere.
	// Use C++ "move" semantics.
	//
	// `dim d1 = "f1^f2^f3"_var.split();`
	//
	dim(dim&& rhs) noexcept : base(std::move(rhs)), ncols_(rhs.ncols_) {

		// noexcept so we will tolerate dim d = dim();
		//if (!rhs.ncols_)
		//	throw(__PRETTY_FUNCTION__);

		rhs.ncols_ = 0; // Reset moved-from state
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
	// void operator=(const dim& rhs) &;

//	dim& operator=(const dim& rhs) {

	// Copy assignment operator (using var::clone())
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
//	void operator=(dim&& rhs) &;

//	dim& operator=(dim&& rhs) noexcept & {

	// Move assignment operator (using swap)
	void operator=(dim&& rhs) & noexcept {
		if (this != &rhs) {
			swap(rhs); // Swap all members
		}
//		return *this;
		return;
	}

//	// Iterator overrides
//	// TODO all throw errors if used on undimensioned arrays
	// Needed for range based for to compile: "for (var& v1 : d1) {"
//	auto begin() { return base::begin();}
//	auto end() { return base::end();}
//	auto begin() const { return base::begin();}
//	auto end() const { return base::end();}
//	auto cbegin() const { return base::cbegin();}
//	auto cend() const { return base::cend();}
	dim_iter begin() { return dim_iter(base::begin()); }
	dim_iter end() { return dim_iter(base::end()); }
	dim_const_iter begin() const { return dim_const_iter(base::begin()); }
	dim_const_iter end() const { return dim_const_iter(base::end()); }
	dim_const_iter cbegin() const { return dim_const_iter(base::cbegin()); }
	dim_const_iter cend() const { return dim_const_iter(base::cend()); }

	/////////////////////
	// Other constructors
	/////////////////////

    // Constructor from initializer_list for (int, double, cstr etc.)
	/////////////////////////////////////////////////////////////////
	template<class T>

	// Create a dimensioned array from a literal list.
	// All elements must be the same type, var, string, double, int, etc. All end up as vars which are flexible.
	//
	// `dim d1 = {1, 2, 3, 4, 5};
	//  dim d2 = {"A", "B", "C"};`
	//
	dim(std::initializer_list<T> list) {
//TRACE("------ INITIALIZER LIST ------")
		ncols_ = 1;
		// Will not be called with zero elements
		//dim d {};

		//TRACE(var("dim constructor from initializer_list ") ^ int(list.size()));
		// list rows, ncols = 1
		// warning: conversion from ‘std::initializer_list<int>::size_type’ {aka ‘long int’} to ‘int’ may change value [-Wconversion]
//		redim(static_cast<int>(list.size()), 1);

		// Allow arbitrary copying of element zero without throwing variable not assigned
		//data_[0].var_typ = VARTYP_STR;

		reserve(list.size()); // Optimize allocation

//		std::size_t itemno = 0;
		for (auto item : list) {
			//base::operator[](itemno++) = item;
			base::push_back(item); // Deep copy via clone()
		}

	}
//	template<class T>
//        // Constructor with initializer list
//        dim(std::initializer_list<T> init) : base(init), ncols_(1) {}


	///////////////////
	// OTHER ASSIGNMENT
	///////////////////

	//=var
	// The assignment operator should always return a reference to *this.
	// cant be (in var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// construction see also ^= etc

	// Initialise all elements of an dimensioned array.
	// To some single value. e.g. a var,  "", 0 etc.
	//
	// `dim d1(10);
	//  d1 = "";`
	//
	void operator=(in v1);

	// Undocumented
	void operator=(const int sourceint);

	// Undocumented
	void operator=(const double sourcedbl);

	// Resize a dimensioned array.
	// To a different number of rows and columns.
	// Existing data will be retained. Any additional elements are unassigned.
	// Resizing rows to 0 clears all data.
	// Resizing cols to 0 clears all data and changes its status to "undimensioned".
	// obj is d1
	//
	// `dim d1;
	//  d1.redim(10, 3);`
	//
	void redim(const int nrows, const int ncols = 1);

	// Swap member (handles both base and ncols_)

	// Swap two arrays.
	// Either or both may be undimensioned.
	// obj is d1
	//
	// `dim d1(5);
	//  dim d2(10);
	//  d1.swap(d2);`
	//
	void swap(dim& d2) noexcept {
		base::swap(d2);
		std::swap(ncols_, d2.ncols_);
	}

	///////////////////
	///// array access:
	///////////////////

////	ND dim_iter begin();
////	ND dim_iter end();
//	// Custom iterators for range-based for
//	dim_iter begin() { return dim_iter(base::begin()); }
//	dim_iter end() { return dim_iter(base::end()); }
//	dim_const_iter begin() const { return dim_const_iter(base::begin()); }
//	dim_const_iter end() const { return dim_const_iter(base::end()); }
//	dim_const_iter cbegin() const { return dim_const_iter(base::cbegin()); }
//	dim_const_iter cend() const { return dim_const_iter(base::cend()); }

//	// Implicit conversion to std::vector<var>&
//	operator std::vector<var>&() { return data_; }
//	operator const std::vector<var>&() const { return data_; }

//
//	ND const dim_iter begin() const;
//	ND const dim_iter end() const;

//    // Mutable iterators
//    dim_iter begin() { return dim_iter(base::begin()); }
//    dim_iter end() { return dim_iter(base::end()); }
//
//    // Const iterators
//    dim_const_iter begin() const { return dim_const_iter(base::begin()); }
//    dim_const_iter end() const { return dim_const_iter(base::end()); }
//
//    // Explicit const iterators (cbegin/cend)
//    dim_const_iter cbegin() const { return dim_const_iter(base::begin()); }
//    dim_const_iter cend() const { return dim_const_iter(base::end()); }

	// brackets operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression

	// [row] Access and update dimensioned array elements.
	// Two dimensioned arrays can be traversed, columnwise then rowwise, using one dimension array access.
	//
	// `dim d1 = {1, 2, 3, 4, 5};
	//  d1[3] = "X";
	//  let v1 = d1[3]; // "X"`
	//
	ND VARREF operator[](int rowno) {return getelementref(rowno, 1);}

	//following const version is called if we do () on a dim which was defined as const xx
	// Undocumented
	ND CVR operator[](int rowno) const {return getelementref(rowno, 1);}

	// Provide 2d version of bracket operator if c++23+
	// and deprecate the parenthesis operator
#if __cpp_multidimensional_subscript >= 202110L
    // Don't change wording without also changing it in cli/fixdeprecated
#	define DEPRECATED_PARENS [[deprecated("EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n]")]]
//#	define DEPRECATED_PARENS2
#	define DEPRECATED_PARENS2 [[deprecated("EXODUS: Replace multiple dimensioned array accessors like (x,y) with [x,y] e.g. dimarray(x,y) -> dimarray[x,y]")]]

	// [row, col] Access and update dimensioned array elements.
	//
	// `dim d1(10, 5);
	//  d1 = "";
	//  d1[3, 4] = "X";
	//  let v1 = d1[3, 4]; // "X"`
	//
	ND VARREF operator[](int rowno, int colno) {return getelementref(rowno, colno);}

	// Undocumented
	ND CVR operator[](int rowno, int colno) const {return getelementref(rowno, colno);}
#else
#	define DEPRECATED_PARENS [[deprecated("EXODUS: Replace single dimensioned array accessors like () with [] e.g. dimarray(n) -> dimarray[n]")]]
#	define DEPRECATED_PARENS2
#endif

	DEPRECATED_PARENS
	// Undocumented
	ND VARREF operator()(int rowno) {return getelementref(rowno, 1);}
	DEPRECATED_PARENS
	// Undocumented
	ND CVR operator()(int rowno) const {return getelementref(rowno, 1);}

	// parenthesis operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression
	//following const version is called if we do () on a dim which was defined as const xx
	DEPRECATED_PARENS2
	// Undocumented
	ND VARREF operator()(int rowno, int colno) {return getelementref(rowno, colno);}
	DEPRECATED_PARENS2
	// Undocumented
	ND CVR operator()(int rowno, int colno) const {return getelementref(rowno, colno);}

	// Transition alternative for () and [] syntax to be used in libexodus, cli, service and test.
	// Should be removed in 2028 when Ubuntu 24.04 is the oldest to be supported by exodus
	// [[deprecated
	ND VARREF at(int rowno, int colno) {return getelementref(rowno, colno);}
	// [[deprecated
	ND CVR at(int rowno, int colno) const {return getelementref(rowno, colno);}

	// obj is d1

	// Get the number of rows in the dimensioned array
	// return: A count. Can be zero, indicating an empty or undimensioned array.
	//
	// `dim d1(5,3);
	// let v1 = d1.rows(); // 5`
	//
	ND var rows() const;

	// Get the number of columns in the dimensioned array
	// return: A count.  0 if the array is undimensioned.
	//
	// `dim d1(5,3);
	// let v1 = d1.cols(); // 3`
	//
	ND var cols() const;

	// Q: why is this commented out?
	// A: we dont want to COPY vars out of a dimensioned array when using it in rhs expression
	// var operator() (int row, int col=1) const;

	// Get a delimited string concatenating all elements of a dimensioned array.
	// Unassigned elements on the end are omitted.
	// delimiter: Default is FM.
	// return: A string var.
	//
	// `dim d1 = {"f1", "f2", "f3"};
	//  let v1 = d1.join(); // "f1^f2^f3"_var`
	//
	ND var join(SV delimiter = _FM) const;

	/////////////////////
	///// array mutation:
	/////////////////////

	// Create or update a dimensioned array using a string with delimiters.
	// If the dim array is undimensioned it will be dimensioned with the number of elements that the string has fields.
	// If the dim array is dimensioned and has more elements than there are fields in the string, the excess array elements are initialised to "". If the record has more fields than there are elements in the array, the excess fields are all left unsplit in the final element of the array.
    // Predimensioning arrays allows the efficient reuse of arrays in loops and ensures that all elements are assigned values, useful when reading records from DB files.
	// Using undimensioned arrays allows the efficient handling of arrays with a very variable number of elements. e.g. OS text files.
	//
	// `dim d1;
	//  d1.splitter("f1^f2^f3"_var); // d1.rows() -> 3  //// Automatically dimensioned.
	//  //
	//  dim d2(10);
	//  d2.splitter("f1^f2^f3"_var); // d2.rows() -> 10 /// Predimensioned. Excess elements become ""`
	//
	void splitter(in str1, SV delimiter = _FM);

	// Sort a dimensioned array.
	// The order of the elements is adjusted so that each element is <= the next.
	// reverse: If true, then the order is reversed. The default is false.
	//
	// `dim d1 = "2,20,10,1"_var.split(",");
	//  d1.sorter();
	//  let v1 = d1.join(","); // "1,2,10,20"_var`
	//
	void sorter(bool reverse = false);

	// Reverse a dimensioned array.
	// The order of the elements is reversed.
	//
	// `dim d1 = "2,20,10,1"_var.split(",");
	//  d1.reverser();
	//  let v1 = d1.join(","); // "1,10,20,2"_var`
	//
	void reverser();

	// Randomise a dimensioned array.
	// The order of the elements is randomised.
	//
	// `dim d1 = "2,20,10,1"_var.split(",");
	//  d1.randomizer();
	//  let v1 = d1.join(","); // random`
	//
	void randomizer();

//	dim& eraser(std::vector<var>::iterator iter1, std::vector<var>::iterator iter2) {base::erase(iter1, iter2); return *this;}
//	dim& eraser(dim_iter dim_iter1, dim_iter dim_iter2) {base::erase(&*dim_iter1, &*dim_iter2); return *this;}

	PUBLIC void push_back(var&& var1) {base::push_back(std::move(var1));}

	///////////////////////
	///// array conversion:
	///////////////////////

	// Get a sorted copy of a dimensioned array.
	ND dim sort(bool reverse = false) const& {dim d1(*this); d1.sorter(reverse); return d1;}

	// Get a reversed copy of a dimensioned array;
	ND dim reverse()                  const& {dim d1(*this); d1.reverser();      return d1;}

	// Get a randomised copy of a dimensioned array.
	ND dim randomize()                const& {dim d1(*this); d1.randomizer();    return d1;}

	// On temporaries the mutator functions are called.
	ND dim sort(bool reverse = false) &&     {               sorter(reverse);    return std::move(*this);}
	ND dim reverse()                  &&     {               reverser();         return std::move(*this);}
	ND dim randomize()                &&     {               randomizer();       return std::move(*this);}

	///////////////////
	///// array DB I/O:
	///////////////////

	// Write a DB file record created from a dimensioned array.
	// Each element in the array becomes a separate field in the DB record.
	// Redundant trailing FMs are omitted.
	//
    // `dim d1 = "Client GD^G^20855^30000^1001.00^20855.76539"_var.split();
    //  let file = "xo_clients", key = "GD001";
    //  if (not deleterecord("xo_clients", "GD001")) {}; // Cleanup first
    //  d1.write(file, key);
    //  // or
    //  write(d1 on file, key);`
	//
	void write(in dbfile, in key) const;

	// Read a DB file record into a dimensioned array.
	// Each field in the database record becomes a single element in the array.
	// return: True if the record exists or false if not,
	// If the array is predimensioned then any excess array elements are initialised to "" and any excess record fields are left unsplit in the final array element. See dim splitter for more info.
	// If the array is not predimensioned (rows and cols = 0) then it will be dimensioned to have exactly the same number of rows as there are fields in the record being read.
	//
    // `dim d1(10);
    //  let file = "xo_clients", key = "GD001";
    //  if (not d1.read(file, key)) ... // d1.join() -> "Client GD^G^20855^30000^1001.00^20855.76539^^^^"_var
    //  // or
    //  if (not read(d1 from file, key)) ...`
	//
	ND bool read(in dbfile, in key);

	///////////////////
	///// array OS I/O:
	///////////////////

	// Create an entire OS text file from a dimensioned array
	// Each element of the array becomes one line in the OS file delimited by \n
	// Any existing OS file is overwritten and replaced.
	// codepage: Optional: Data is converted from UTF8 to the required codepage/encoding before output. If the conversion cannot be performed then return false.
	// return: True if successful or false if not.
	//
	// `dim d1 = "aaa=1\nbbb=2\nccc=3\n"_var.split("\n");
    //  if (not osremove("xo_conf.txt")) {}; // Cleanup first
	//  let osfilename = "xo_conf.txt";
	//  if (not d1.oswrite(osfilename)) ...
	//  // or
	//  if (not oswrite(d1 on osfilename)) ...`
	//
	ND bool oswrite(in osfilename, const char* codepage = "") const;

	// Read an entire OS text file into a dimensioned array.
	// Each line in the OS file, delimited by \n or \r\n, becomes a separate element in the array.
	// Existing data in the array is lost and the array is redimensioned to the number of lines in the input data.
	// codepage: Optional. Data will be converted from the specified codepage/encoding to UTF8 after being read. If the conversion cannot be performed then return false.
	// return: True if successful or false if not.
	// If the first \n in the file is \r\n then the whole file will be split using \r\n as delimiter.
	//
	// `dim d1;
	//  let osfilename = "xo_conf.txt";
	//  if (not d1.osread(osfilename)) ... // d1.join("\n") -> "aaa=1\nbbb=2\nccc=3\n"_var0
	//  // or
	//  if (not osread(d1 from osfilename)) ...`
	//
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

 private:

	dim& init(in var1);

	ND CVR getelementref(int row, int colno) const;
	ND VARREF getelementref(int row, int colno);

}; // class dim

// Free swap function
inline void swap(dim& a, dim& b) noexcept {
	a.swap(b);
}

} // namespace exo

#endif //VARDIM_H

