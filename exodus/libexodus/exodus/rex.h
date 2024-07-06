#ifndef VARREX_H
#define VARREX_H

#include <exodus/var.h>

//warning: inlining failed in call to ‘exodus::rex::~rex() noexcept’: call is unlikely and code size would grow [-Winline]
#pragma GCC diagnostic push
#if __GNUC__ <= 10
#	pragma GCC diagnostic ignored "-Winline"
#endif

namespace exodus {

// Note that "for (var& var1: dim1)" with & allows direct access and update into the elements of the array dim1 via varx
// whereas "for (var var1 : dim1)" gives a copy of each element which is slower allows updating var1 without updating dim1
// Both cases are useful

//class rex final
class PUBLIC rex final {

friend class var;

 private:

	mutable var expression_ = "";
	mutable var options_ = "";

 public:

rex(SV expression);

rex(SV expression, SV options);

//	// TODO define in class for inline/optimisation?
//
//	///////////////////////////
//	// SPECIAL MEMBER FUNCTIONS
//	///////////////////////////
//
//	/////////////////////////
//	// 1. Default constructor
//	/////////////////////////
//	//
//	// allow syntax "rex r;" to create an "unassigned" rex
//	// allow default construction for class variables later resized in class methods
//	rex() = default;
//
//	////////////////
//	// 2. Destructor
//	////////////////
//
//	// destructor to (NOT VIRTUAL to save space since not expected to be a base class)
//	// protected to prevent deriving from var since wish to save space and not provide virtual
//	// destructor http://www.gotw.ca/publications/mill18.htm
//	~rex() = default;
//
//	//////////////////////
//	// 3. Copy constructor - from lvalue
//	//////////////////////
//	//
//	rex(const rex& sourcedim) {
//		// use copy assign
//		*this = sourcedim;
//	}
//
//	//////////////////////
//	// 4. move constructor - from rvalue
//	//////////////////////
//	//
//	rex(rex&& sourcedim) {
//		// use move assign
//		*this = std::move(sourcedim);
//	}
//
//	/////////////////////
//	// 5. copy assignment - from lvalue
//	/////////////////////
//
//	// Not using copy assignment by value (copy-and-replace idiom)
//	// because Howard Hinnant recommends against in our case
//
//	// Prevent assigning to temporaries
//	void operator=(const rex& rhs) && = delete;
//
//	// var& operator=(CVR rhs) & = default;
//	// Cannot use default copy assignment because
//	// a) it returns a value allowing accidental use of "=" instead of == in if statements
//	// b) doesnt check if rhs is assigned
//	void operator=(const rex& rhs) &;
//
//	/////////////////////
//	// 6. move assignment - from rvalue
//	/////////////////////
//
//	// Prevent assigning to temporaries
//	void operator=(rex&& rhs) && noexcept = delete;
//
//	// Cannot use default move assignment because
//	// a) it returns a value allowing accidental use of "=" in if statements instead of ==
//	// b) doesnt check if rhs is assigned (less important for temporaries which are rarely unassigned)
//	//var& operator=(TVR rhs) & noexcept = default;
//	void operator=(rex&& rhs) &;
//
//	/////////////////////
//	// Other constructors
//	/////////////////////
//
//	// Constructor with number of rows and optional number of columns
//	/////////////////////////////////////////////////////////////////
//	rex(const SV expression, SV options);
//
//	///////////////////
//	// OTHER ASSIGNMENT
//	///////////////////
//
//	//=var
//	// The assignment operator should always return a reference to *this.
//	// cant be (CVR var1) because seems to cause a problem with var1=var2 in function
//	// parameters unfortunately causes problem of passing var by value and thereby unnecessary
//	// contruction see also ^= etc
//	void operator=(CVR sourcevar);
//
//	////////////
//	// ACCESSORS
//	////////////
//
//	var rex();

}; // class rex

// User defined literal "_rex"

// "[a-z]"_rex
PUBLIC rex operator""_rex(const char* cstr, std::size_t size);


} // namespace exodus

#pragma GCC diagnostic pop

#endif //VARREX_H

