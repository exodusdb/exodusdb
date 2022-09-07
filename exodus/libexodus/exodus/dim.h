#ifndef VARDIM_H
#define VARDIM_H

#include <exodus/var.h>

namespace exodus {

class dim_iter;

//class dim final
class PUBLIC dim final {

   private:
	unsigned int nrows_, ncols_;
	// NOTE: trying to implement data_ as boost smart array pointer (boost::scoped_array<var>
	// data_;) raises warning: as dim is PUBLIC, boost library should have DLL interface.
	// Choices: 1) leave memory allocation as is (refuse from scoped_array, or
	// 2) use pimpl metaphor with slight decrease in performance.
	// Constructors of dim are very simple, no exception expected between 'new' and return from
	// constructor As such, choice (1).

	// all redimensioning of this array (eg when copying arrays)
	// seem to be using ::redim() to accomplish redimensioning
	// so only the redim code is dangerous (danger in one place is manageable)
	// we choose NOT to implement 2) above (pimpl) in order
	// to provide exodus programmer greater/easier visibility into dimensiorned arrays when
	// debugging (cannot use boost scoped pointer here because mv.h is required by exodus
	// programmer who should not need boost)
	var* data_ = nullptr;
	bool initialised_ = false;

   public:

	// TODO define in class for inline/optimisation

	///////////////////////////
	// SPECIAL MEMBER FUNCTIONS
	///////////////////////////

	/////////////////////////
	// 1. Default constructor
	/////////////////////////
	//
	// allow syntax "dim d;" to create an "unassigned" dim
	// allow default construction for class variables later resized in class methods
	dim();

	////////////////
	// 2. Destructor
	////////////////

	// destructor to (NOT VIRTUAL to save space since not expected to be a base class)
	// protected to prevent deriving from var since wish to save space and not provide virtual
	// destructor http://www.gotw.ca/publications/mill18.htm
	~dim();

	//////////////////////
	// 3. Copy constructor - from lvalue
	//////////////////////
	//
	dim(const dim& sourcedim);

	//////////////////////
	// 4. move constructor - from rvalue
	//////////////////////
	//
	dim(dim&& sourcedim);

	/////////////////////
	// 5. copy assignment - from lvalue
	/////////////////////

	// Not using copy assignment by value (copy-and-swap idiom)
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


	// Constructor with number of rows and optional number of columns
	/////////////////////////////////////////////////////////////////
	dim(const unsigned int nrows, const unsigned int ncols = 1);

	bool redim(const unsigned int nrows, const unsigned int ncols = 1);

    // Constructor from initializer_list for (int, double, cstr etc.)
	/////////////////////////////////////////////////////////////////
	template<class T>
	dim(std::initializer_list<T> list) {
		//std::clog << "iizer " << list.size() << std::endl;
		redim(list.size(), 1);
		int itemno = 1;
		for (auto item : list) {
			this->data_[itemno++] = item;
		}
	}

	ND var join(SV sepchar = _FM) const;

	// parenthesis operators often come in pairs
	// returns a reference to one var of the array
	// and so allows lhs assignment like d1(1,2) = "x";
	// or if on the rhs then use as a normal expression
	ND VARREF operator()(unsigned int rowno, unsigned int colno = 1);

	//following const version is called if we do () on a dim which was defined as const xx
	ND CVR operator()(unsigned int rowno, unsigned int colno = 1) const;

	ND var rows() const;
	ND var cols() const;

	// Q: why is this commented out?
	// A: we dont want to COPY vars out of an array when using it in rhs expression
	// var operator() (int row, int col=1) const;

	//=var
	// The assignment operator should always return a reference to *this.
	// cant be (CVR var1) because seems to cause a problem with var1=var2 in function
	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
	// contruction see also ^= etc
	void operator=(CVR sourcevar);
	void operator=(const int sourceint);
	void operator=(const double sourcedbl);

	// see also var::split
	// return the number of fields
	var split(CVR var1, SV sepchar = _FM);
	dim& sort(bool reverse = false);

	bool read(CVR filehandle, CVR key);
	bool write(CVR filehandle, CVR key) const;

	bool osread(CVR osfilename, const char* codepage DEFAULT_EMPTY);
	bool oswrite(CVR osfilename,const char* codepage DEFAULT_EMPTY) const;

	// following is implemented on the dim class now
	// dim dimarray2();
	//

	friend class dim_iter;

	//BEGIN - free function to create an iterator -> begin
	PUBLIC ND friend dim_iter begin(const dim& d);

	//END - free function to create an interator -> end
	PUBLIC ND friend dim_iter end(const dim& d);

   private:

	dim& init(CVR var1);

}; // class dim

// Note that "for (var& var1: dim1)" with & allows direct access and update into the elements of the array dim1 via varx
// whereaS "for (var var1 : dim1)" gives a copy of each element which is slower allows updating var1 without updating dim1
// Both cases are useful

//class dim_iter
// defined in header to be inlined for performance which is critical
class PUBLIC dim_iter {

   private:

	const dim* pdim_;

	// Start from 1 ignoring element 0
	unsigned int index_ = 1;

   public:

	// Default constructor
	dim_iter() = default;

	// Construct from dim
	dim_iter(const dim& d1);

	// Check iter != iter (i.e. iter != string::npos)
	bool operator!=(const dim_iter& dim_iter1);

	// Dereference iter to a var&
	// return a reference to the actual dim element so it can be updated
	// iif use var& instead of var in "for (var& : dim1)"
	//operator var*();
	var& operator*();

	//iter++
	dim_iter operator++();

	//iter--
	dim_iter operator--();

	void end();

}; // class dim_iter

} // namespace exodus

#endif //VARDIM_H

