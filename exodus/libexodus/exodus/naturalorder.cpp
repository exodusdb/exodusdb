// copied in pgexodus and exodus - please keep in sync 1
#include <cstring>
#include <sstream>

#include <cassert>

namespace exodus {

// select exodus_extract_text(data,1,0,0), exodus_extract_sort(data,1,0,0)
// from test_alphanum order by exodus_extract_sort(data,1,0,0)

/* lexicographical sorting of numbers

We wish to be able to sort a report on an alphanumeric column and for the numbers to be sorted
numerically and the non-numerics to be sorted alphabetically eg 2,10,A,B. We expect the classic pick
behaviour and not be forced into alphabetic sorting of numbers eg 10,2.

If possible we also wish mixed alphanumerics to be sorted reasonably eg A2,A10 (ie not A10,A2).

NB the following is based on 32 bit ints and floats but the same techniques can be applied to 64 bit
doubles, mapping them to some 64-bit ints like long long int.

long int http://en.wikipedia.org/wiki/Long_integer
long long int is from C99 and is required to be at least 64 bits

http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm

The IEEE float and double formats were designed so that the numbers are lexicographically ordered,
which in the words of IEEE architect William Kahan means if two floating-point numbers in the same
format are ordered (say x < y ), then they are ordered the same way when their bits are
reinterpreted as Sign-Magnitude integers.

This means that if we take two floats in memory, interpret their bit pattern as integers, and
compare them, we can tell which is larger, without doing a floating point comparison. In the C/C++
language this comparison looks like this:

 if (*(int*)&f1 < *(int*)&f2)

This charming syntax means take the address of f1, treat it as an integer pointer, and dereference
it. All those pointer operations look expensive, but they basically all cancel out and just mean
treat f1 as an integer. Since we apply the same syntax to f2 the whole line means compare f1 and f2,
using their in-memory representations interpreted as integers instead of floats.

Kahan says that we can compare them if we interpret them as sign-magnitude integers. Thats
unfortunate because most processors these days use twos-complement integers. Effectively this means
that the comparison only works if one or more of the floats is positive. If both floats are negative
then the sense of the comparison is reversed the result will be the opposite of the equivalent float
comparison. Later we will see that there is a handy technique for dealing with this inconvenience.

    int aInt = *(int*)&A;
    // Make aInt lexicographically ordered as a twos-complement int     (or 0x8000000000000000 for
double) if (aInt < 0)
	  //nb 0x800000000000000000LL on 64bit
      aInt = 0x80000000 - aInt;

Compiler issues

In our last version of AlmostEqualUlps we use pointers and casting to tell the compiler to treat the
in-memory representation of a float as an int. There are a couple of things that can go wrong with
this. One risk is that int and float might not be the same size. A float should be 32 bits, but an
int can be almost any size. This is certainly something to be aware of, but every modern compiler
that I am aware of has 32-bit ints. If your compiler has ints of a different size, find a 32-bit
integral type and use it instead.

Another complication comes from aliasing optimizations. Strictly speaking the C/C++ standard says
that the compiler can assume that different types do not overlap in memory (with a few exceptions
such as char pointers). For instance, it is allowed to assume that a pointer to an int and a pointer
to a float do not point to overlapping memory. This opens up lots of worthwhile optimizations, but
for code that violates this rulewhich is quite commonit leads to undefined results. In particular,
some versions of g++ default to very strict aliasing rules, and dont like the techniques used in
AlmostEqualUlps.

Luckily g++ knows that there will be a problem, and it gives this warning:

 warning: dereferencing type-punned pointer will break strict-aliasing rules

There are two possible solutions if you encounter this problem. Turn off the strict aliasing option
using the -fno-strict-aliasing switch, or use a union between a float and an int to implement the
reinterpretation of a float as an int. The documentation for -fstrict-aliasing gives more
information.

Examples:

hex int representation of some doubles - after inverting negatives so that "large" negatives
sort before small negatives - and inverting the high bit so all negatives sort before all positives.

	// -5.0 3fec000000000000
	// -4.0 3ff0000000000000
	// -3.0 3ff8000000000000
	// -2.0 4000000000000000
	// -1.0 4010000000000000
	//  0.0 8000000000000000
	//  1.0 bff0000000000000
	//  2.0 c000000000000000
	//  3.0 c008000000000000
	//  4.0 c010000000000000
	//  5.0 c014000000000000

*/

/* strategy

The problem isnt just about sorting 10A and 2B correctly, it is about correctly sorting simple cases
like 10, 2, A, and B as 2, 10, A, B ie mixed data but not mixed within any one field. All picks can
do that because they have a type-less comparison operator > that works "correctly" on text and
textual numbers.

To avoid requiring the implementation of a proper pick comparison operator in every sql database
that exodus uses in future, a function is provided that preprocesses a bit of text into a text that,
when compared as ascii/unicode, produces the right result. Essentially this involves replacing all
the numbers found in a string with a 16 hex digit representation of the number. This function must
be called around all sorting and indexing fields and also on all filtering values passed like 'ABC'
in "with xyz > 'ABC'". Since exodus build up the sql queries before each selection then the function
can be built into the sql command. Note that selection on indexes built with natural order function
in them must have the function matching in field and data when using the index.

Note that in the conversion to 16 hex digits that all surplus zeroes are lost losing informaition. A
proper comparison operator would be able to sort 001 before 1 but the current strategy cannot allow
for this.

NB all pick databases do text to floating point conversions on the fly since they store numbers as
strings. This conversion-to-natural-order routine does a floating point conversion (but maybe for
performance it should use the cpu's fp unit to do the job) The comparison then doesnt require an
additional conversion to fp so the performance is perhaps ok for sorting pure numbers.

Trailing E for exponent not supported at the moment.

2	<	10
.10	<	.2
-20	<	-1

2.0	=	2
02	=	2

ABC	<	BC

Flat 2B	<	Flat 10A

A-2	<	A-10		dots and dashes within text dont imply negative/decimal points
A -10	<	A -2        unless prefixed by space
A.2	<	A.10
A .10	<	A .2

A1.10	=	A1.1		excess zeros are ignored in sorting
A010  =	A10
A1.0  =	A1

A1-2	<	A01-10	multi-level number sort

*/

void appenddouble2sortstring(const double& double1, std::ostringstream& stringstream1) {
	assert(sizeof(double) == sizeof(long long int));

	// view the double as an 64 bit integer
	long long int integer1 = *(long long int*)&double1;

	if (integer1 < 0)
		// invert the negative numbers and clear the high bit so that
		// lexicographically they sort in the right order and before the positive numbers
		integer1 = 0x8000000000000000LL - integer1 + 0x8000000000000000LL;
	else
		// set the high bit so that positive numbers appear lexicographically after the
		// negative numbers
		integer1 -= 0x8000000000000000LL;

	// make leftmost digit a decimal so it sorts as a number instead of any chance hex a-f
	// put a 9 which sorts the same as 0 and removes the need for fill/width formatting below
	integer1 = integer1 >> 4;
	integer1 &= 0x9fffffffffffffffLL;

	// hex output
	stringstream1.flags(std::ios::right | std::ios::hex);

	// needed if leftmost digit is zero
	// stringstream1.fill('0');
	// stringstream1.width(16);

	stringstream1 << integer1;
}

std::string naturalorder(const std::string& string1) {

	// using goto here to break out of multiple embedded loops
	// since c/c++ break/continue is unable to do this

	std::ostringstream ostringstream1;

	std::string::const_iterator iter = string1.begin();
	std::string::const_iterator iterend = string1.end();
	double val;
	double power;

naturalorderbegin:

	// must have one digit to pass as number
	// ie - + . -. +. are all not numbers
	bool hasdigit = false;

	// record the current position. in case the following characters dont constitute a proper
	// number then they will all be copied to the output as plain text
	std::string::const_iterator iterbegin = iter;

	// skip white space
	for (;;) {
		if (iter == iterend)
			goto naturalorderexit;
		if (!isspace(*iter))
			break;
		++iter;
	}

	// determine positive or negative and skip one leading + or -
	int sign;
	if (*iter == '-') {
		sign = -1;
		++iter;
	} else if (*iter == '+') {
		sign = 1;
		++iter;
	} else
		sign = 1;

	// get integer part
	val = 0.0;
	power = 1.0; // here in case there is no fractional part
	for (;;) {
		if (iter == iterend)
			goto naturalorderexit;
		if (!isdigit(*iter))
			break;
		hasdigit = true;
		val = 10.0 * val + (*iter - '0');
		++iter;
	}

	// skip one decimal point if present after integer part
	if (*iter == '.')
		++iter;

	// continue appending digits to the integer but recording progressive division by 10 for
	// each one
	for (;;) {
		if (iter == iterend)
			goto naturalorderexit;
		if (!isdigit(*iter))
			break;
		hasdigit = true;
		val = 10.0 * val + (*iter - '0');
		power *= 10;
		++iter;
	}

naturalorderexit:

	if (hasdigit) {
		// leading zero to ensure that numbers sort in the usual sequence
		//(since the 16 character representation is hex with letters)
		// do this in the 16 digit
		// ostringstream1<<"0";

		// append the 16 hex digit lexical sortable representation of the number
		appenddouble2sortstring(sign * val / power, ostringstream1);
	} else {
		// copy any characters that looked like they might be a number but in the end were
		// not
		while (iterbegin != iter)
			ostringstream1 << *iterbegin++;
	}

	// append any trailing non-numerical data
	while (iter != iterend) {

		// go back and possibly output another number
		// NB - and . are NOT candidates to go back. this prevent strange effect like A-10 <
		// A-2 22.22.22 appearing like 22.22 and .22 22-22-22 appearing like 22 -22 -22
		// check low digits first since they are much commoner in practice as the starting
		// digit of numbers
		const char* ptr = (strchr("1234567890 \"'+", *iter));
		if (ptr != 0) {

			// also allow . - (or digits) after single or double quote to be considered
			// as a number.
			if (*ptr == '"' || *ptr == '\'') {
				ostringstream1 << *iter++;
				if (iter == iterend)
					break;
				if (strchr("1234567890 .-+\"'", *iter) != 0)
					goto naturalorderbegin;
			}

			else
				goto naturalorderbegin;
		}

		ostringstream1 << *iter++;
	}
	//	std::cout<<string1<<" --- "<<ostringstream1.str()<<std::endl;
	return ostringstream1.str();
}

} // namespace exodus
