#ifndef EXODUS_LIBEXODUS_EXODUS_RANGE_H_
#define EXODUS_LIBEXODUS_EXODUS_RANGE_H_

namespace exodus {

// class range
// provides "for (const var i : range(1,10))"
// Represents an interable range of ints (int_iter)
// Lower case class name so we can use in syntax like
// "for (int i : range(1 to 10))"
// https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
// defined in header to be inlined for performance which is critical
#define INT_T int
class PUBLIC range {
	using int_t = INT_T;

	int first_;
	int last_;

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

		// Allow checking if an iterator is AT OR BEYOND the end of the range
		// WARNING declare "operator!=" but actually use the "<" operator to compare!
		friend bool operator!= (const Iterator& a, const Iterator& b) {
			//var (a.int_).errputl("A ");
			//var (b.int_).errputl("B ");
			// Once the moving int is equal to OR GREATER THAN the target then the result is TRUE ie NOT EQUAL
			// This caters for the situation "for (var x : (10, 8)" which would otherwise loop incorrectly
			return a.int_ < b.int_; }

	};

	// Construct a range from two ints
	range(int_t first, int_t last) : first_(first), last_(last + 1) {}

	// Return our first int
	Iterator begin() { return Iterator(first_); }

	// Return our last int
	Iterator end()   { return Iterator(last_); }

};

} // namespace exodus

#endif // EXODUS_LIBEXODUS_EXODUS_RANGE_H_
