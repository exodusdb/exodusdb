#include <stdio.h>
#include <string.h>

#ifndef FM
#define FM_ '\x1E'
#define VM_ '\x1D'
#define SM_ '\x1C'
#endif

void extract_v3(char * instring, int inlength, int fn, int vn, int sn, int* outstart, int* outlength) noexcept {

//	printl("---");
//	printl(s1);

	*outstart = 0;
	*outlength = 0;

	// Empty string always returns empty string
	if (not inlength)
		return;

	//Negative indexes always return empty string
	if (fn < 0 || vn < 0 || sn < 0)
		return;

	int fn0 = fn ? fn - 1 : fn;
	int vn0 = vn ? vn - 1 : vn;
	int sn0 = sn ? sn - 1 : sn;

	// Find the beginning of the desired field, value or subvalue
	auto cptr = instring;
	auto cend = cptr + inlength;
//	printl("cptr1", cptr - s1.begin());
//	printl("cend1", cend - s1.begin());

	while ((fn0 + vn0 + sn0) != 0) {

//		printl("char", var(char(*cptr)));

		// Skip the usual case of non-delimiter characters ASAP
		if (*cptr > FM_) {
			++cptr;
			// If we run off the end of the string then return
			if (cptr == cend)
				return;
			continue;
		}

		switch (*cptr) {
			case FM_:
				if (--fn0 < 0)
					return;
				++cptr;
				break;
			case VM_:
				if (fn0 == 0 && --vn0 < 0)
					return;
				++cptr;
				break;
			case SM_:
				if (fn0 == 0 && vn0 == 0 && --sn0 < 0)
					return;
				++cptr;
				break;
			default:
				// Control characters from 0 and below SM_
				++cptr;
				break;
		}

		// If we run off the end of the string then return
		if (cptr == cend)
			return;
	}

//	printl("cptr2", cptr - s1.begin());
//	printl("cend2", cend - s1.begin());

	// Find the end of desired field, value or subvalue
	char target;
	if (sn > 0)
		target = SM_;
	else if (vn > 0)
		target = VM_;
	else if (fn > 0)
		target = FM_;
	else {
//		TRACE("All 0, 0, 0")
		*outstart = 0;
		*outlength = inlength;
		return;
	}

	// Record the start of the result
	auto start = cptr;

	// Find the target or higher separator
	while (cptr != cend) {

		// Skip the common case of characters higher than FM_
		if (*cptr > FM_ ) {
			++cptr;
			continue;
		}

		// Found the target delimiter or higher delimiter
		if (*cptr >= target) {
			break;
		}

		// control characters from 0 to below SM_
		++cptr;
	}
	*outstart = (int)(start - instring);
	*outlength = (int)(cptr - start);
}
