#include <stdio.h>
#include <string.h>

#ifndef FM
#define FM_ '\x1E'
#define VM_ '\x1D'
#define SM_ '\x1C'
#endif

/*
TODO algorithm could be improved for value and subvalue extraction
by not searching for the end of the field before starting to look for the multivalue
and for extracting subvalues, and not searching for the end of the multivalue
before starting to look for the subvalue.
*/

/* See also extract.c which uses indexes instead of pointer/iterators */
/* This version extract2.c is slightly faster 120/40ns per op compared to 130/45 ns/op for extract.c */
void extract_v2(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength) noexcept {
	char* cptr;
	char* instr_end_pos;

	int fieldn2;
	char* field_end_pos;

	int valuen2;
	char* value_end_pos;

	int subvaluen2;
	char* subvalue_end_pos;

	/*assert(assigned());*/

	/*
	any negatives at all returns "" but since this is unexpected
	do it inline instead of special case to avoid wasting time
	if (fieldno<0||valueno<0||subvalueno<0) return;
	*/

	/*FIND FIELD*/

	/*default to returning nothing*/
	*outstart = 0;
	*outlength = 0;

	/*zero means all, negative return ""*/
	if (fieldno <= 0)
	{
		if (fieldno < 0)

			// Negative field number always returns empty string
			return;

		if (valueno || subvalueno) {

			// Field 0 with any value number or subvaluen
			fieldno = 1;

		} else {

			// field 0, value 0, subvalue 0 returns the whole original string
			*outlength = inlength;
			return;
		}
	}

	// Start at the first character of the string
	cptr = instring;

	/* End before the character after the string*/
	instr_end_pos = instring + inlength;

	/* FIELD */

	/* Find the start position or return empty string*/
	fieldn2 = fieldno - 1;
	while (fieldn2)
	{

		/* Find the next FM_*/
		for (;;) {
			/* Past end of string? - return empty string */
			if (cptr >= instr_end_pos)
				return;
			if (*cptr == FM_)
				break;
			cptr++;
		}

		// Skip over the FM_ character
		cptr++;

		// Count down to the desired field number
		fieldn2--;
	}

	/* Find the end of the current field */
	field_end_pos = cptr;
	while (field_end_pos < instr_end_pos && *field_end_pos != FM_) {
		field_end_pos++;
	}

	/*FIND VALUE*/

	/*zero means all, negative return ""*/
	if (valueno <= 0)
	{
		if (valueno < 0)
			return;
		if (subvalueno)
			valueno = 1;
		else
		{
			*outstart = static_cast<int>(cptr - instring);
			*outlength = static_cast<int>(field_end_pos - cptr);
			return;
		}
	}

	/*
	find the starting position of the value or return ""
	using cptr and end_pos of
	*/
	valuen2 = valueno - 1;
	while (valuen2)
	{

		/* Find the next VM_*/
		while (cptr < field_end_pos && *cptr != VM_) {
			cptr++;
		}

		/* Past end of field? return empty string */
		if (cptr >= field_end_pos)
			return;

		/* Skip over VM_ */
		cptr ++;

		/* Count down to the desired value */
		valuen2--;
	}

	/*find the end of the current value */
	value_end_pos = cptr;
	while (value_end_pos < field_end_pos && *value_end_pos != VM_) {
		value_end_pos++;
	}

	/*FIND SUBVALUE*/

	/*zero means all, negative means ""*/
	if (subvalueno <= 0) {
		if (subvalueno < 0)
			return;
		*outstart = static_cast<int>(cptr - instring);
		*outlength = static_cast<int>(value_end_pos - cptr);
	}

	/*
	find the starting position of the subvalue or return ""
	using cptr and end_pos of
	*/
	subvaluen2 = subvalueno - 1;
	while (subvaluen2) {

		/* Find the next SM_*/
		while (cptr < value_end_pos && *cptr != SM_) {
			cptr++;
		}

		/* Past end of value? return empty string */
		if (cptr >= value_end_pos)
			return;

		/* Skip over the SM_ */
		cptr++;

		/* Count down to the desired subvalue */
		subvaluen2--;
	}

	/*find the end of the current subvalue*/
	subvalue_end_pos = cptr;
	while (subvalue_end_pos < value_end_pos && *subvalue_end_pos != SM_) {
		subvalue_end_pos++;
	}

	/* Return the start and length of the current subvalue */
	*outstart = static_cast<int>(cptr - instring);
	*outlength = static_cast<int>(subvalue_end_pos - cptr);

	return;
}

