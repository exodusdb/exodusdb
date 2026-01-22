#ifndef LIBEXODUS_RANGE_H_
#define LIBEXODUS_RANGE_H_

//#include <exodus/exoimpl.h>

namespace exo {

// class range
// provides "for (const var i : range(1,10))"
// Represents an iterable range of ints (int_iter)
// Lower case class name so we can use in syntax like
// "for (int i : range(1 to 10))"
// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
// defined in header to be inlined for performance which is critical
class PUBLIC range {
	using int_t = int;

	int begin_;
	int end_;

 public:

	// Represent an incrementable and comparable int
	class Iterator {
		int int_;

	public:

		// Construct from an int
		Iterator(int_t int1) : int_(int1) {}

		// Return our int
		int_t operator*() const { return int_; }

		// Increment our int
		Iterator& operator++() { int_++; return *this; }

		//Iterator operator++(int_t) { Iterator tmp = *this; ++(*this); return tmp; }

//		// Allow checking if an iterator is AT OR BEYOND the end of the range
//		// WARNING declare "operator!=" but actually use the "<" operator to compare!
//		friend bool operator!= (const Iterator& a, const Iterator& b) {
//			//var (a.int_).errputl("A ");
//			//var (b.int_).errputl("B ");
//			// Once the moving int is equal to OR GREATER THAN the target then the result is TRUE ie NOT EQUAL
//			// This caters for the situation "for (var x : (10, 8)" which would otherwise loop incorrectly
//			return a.int_ < b.int_; }

		friend bool operator!= (const Iterator& a, const Iterator& b) {
			//var (a.int_).errputl("A ");
			//var (b.int_).errputl("B ");
			return a.int_ != b.int_; }

	};

	// Construct a range from two ints
	// Prevent first > last
	range(int_t first, int_t last) : begin_(first <= last ? first : last + 1), end_(last + 1){}

	// Return our begin int
	Iterator begin() { return Iterator(begin_); }

	// Return our end int (last plus 1)
	Iterator end()   { return Iterator(end_); }

};

// Inefficient? implementation of reverse_range pending full c++20 implementation of ranges in g++11/Ubuntu 22.04
//
// Simply substracts the usual ascending int from the end_ int (last plus 1)
//
// class reverse_range
// provides "for (const var i : reverse_range(1,10))"
// Represents an iterable reverse_range of ints (int_iter)
// Lower case class name so we can use in syntax like
// "for (int i : reverse_range(1 to 10))"
// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
// defined in header to be inlined for performance which is critical
class PUBLIC reverse_range {
	using int_t = int;

	int begin_;
	int end_;

 public:

	// Represent an incrementable and comparable int
	class Iterator {
		int int_;
		//int end_;

	public:

		// Construct from an int /*and an end int*/
		Iterator(int_t int1, int_t /*end*/) : int_(int1)/*, end_(end)*/ {}

		// Return our int (adjusted to be a reverse int)
		int_t operator*() const { return int_; }

		// Increment our int (ACTUAL DECREMENT)
		Iterator& operator++() { int_--; return *this; }

		//Iterator operator++(int_t) { Iterator tmp = *this; ++(*this); return tmp; }

//		// Allow checking if an iterator is AT OR BEYOND the end of the reverse_range
//		// WARNING declare "operator!=" but actually use the "<" operator to compare!
//		friend bool operator!= (const Iterator& a, const Iterator& b) {
//			//var (a.int_).errputl("A ");
//			//var (b.int_).errputl("B ");
//			// Once the moving int is equal to OR GREATER THAN the target then the result is TRUE ie NOT EQUAL
//			// This caters for the situation "for (var x : (10, 8)" which would otherwise loop incorrectly
//			return a.int_ < b.int_; }

		friend bool operator!= (const Iterator& a, const Iterator& b) {
			//var (a.int_).errputl("A ");
			//var (b.int_).errputl("B ");
			return a.int_ != b.int_; }

	};

	// Construct a reverse_range from two ints
	// If first > last then prevent entering loop by setting begin == end == 0
	//reverse_range(int_t first, int_t last) : begin_(first <= last ? last : first), end_(first <= last ? first - 1 : last - 1){}
	reverse_range(int_t first, int_t last) : begin_(first <= last ? last : 0), end_(first <= last ? first - 1 : 0){}

	// Return our begin int
	Iterator begin() { return Iterator(begin_, end_); }

	// Return our end int (last plus 1)
	Iterator end()   { return Iterator(end_, end_); }

};

} // namespace exo

#endif // LIBEXODUS_RANGE_H_
