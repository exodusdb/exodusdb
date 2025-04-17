/*
Copyright (c) 2009 steve.bush@neosys.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#if EXO_MODULE
#else
//#	include <string>
#	include <algorithm>  //for dim::sort
#	include <cstring>	  //for strlen strstr
#endif

#ifdef __has_include
#	if __has_include(<version>)
#		include <version>
#	endif
#endif

#include <exodus/varimpl.h>

void extract_v2(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength);
void extract_v3(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength);
void extract_v4(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength);

namespace exo {

using let = const var;

// includes dim::split

// and var::field,field2,locate,extract,remove,update,insert,substr,paste,remove

//////////////////////////////
// REMOVE was PickOS "DELETE()"
//////////////////////////////

// Remove in place mutator
#include "varremover.cpp"

///////////////////////////////////////////
// UPDATE int int int var
///////////////////////////////////////////

#include "varreplacer.cpp"

///////////////////////////////////////////
// INSERT int int int var
///////////////////////////////////////////

//in-place - given everything
#include "varinserter.cpp"

////////
// FIELD
////////

// var.field(separator,fieldno,nfields)
var  var::field(SV delimiter, const int fieldnx, const int nfieldsx) const {

	THISIS("var  var::field(SV delimiter, const int fieldnx, const int nfieldsx) const")
	assertString(function_sig);

	if (delimiter.empty())
		UNLIKELY
		throw VarError("delimiter cannot be blank in field()");

	// fieldno 0 -> 1
	int fieldno;
	if (fieldnx > 0) {
		fieldno = fieldnx;
	} else if (fieldnx < 0) {

		// field -1 is quick
		if (fieldnx == -1) {
			std::size_t pos = var_str.rfind(delimiter);
			if (pos == std::string::npos)
				return var_str;
			return var_str.substr(pos + 1);
		}

		fieldno = this->count(delimiter) + 1 + fieldnx + 1;

		if (fieldno <= 0)
			fieldno = 1;

	} else {
		// fieldno 0 -> fieldno 1
		fieldno = 1;
	}

	// nfields 0 -> nfields 1
	int nfields = nfieldsx > 0 ? nfieldsx : 1;

	// separator might be multi-byte ... esp. for non-ASCII
	std::size_t len_separator = delimiter.size();

	// FIND FIELD

	// find the starting position of the field or return ""
	std::size_t pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		pos = var_str.find(delimiter, pos);
		// past of of string?
		if (pos == std::string::npos)
			return "";
		// pos++;
		pos += len_separator;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::size_t end_pos = pos;
	int pastfieldn = fieldno + nfields;
	while (fieldn2 < pastfieldn) {
		end_pos = var_str.find(delimiter, end_pos);
		// past of of string?
		if (end_pos == std::string::npos) {
			//return var_str.substr(pos, var_str.size() - pos);
			return var_str.substr(pos);
		}
		// end_pos++;
		end_pos += len_separator;
		fieldn2++;
	}
	// backup to first character if closing separator in case multi-byte separator
	end_pos -= (len_separator - 1);

	return var_str.substr(pos, end_pos - pos - 1);
}

/////////////
// FIELDSTORE
/////////////

//// var.fieldstore(separator,fieldno,nfields,replacement)
//var  var::fieldstore(SV separator, const int fieldnx, const int nfieldsx, in replacementx) const& {
//	return var(*this).fieldstorer(separator, fieldnx, nfieldsx, replacementx);
//}

//// Constant
//ND var  var::fieldstore(SV separator, const int fieldno, const int nfields, in replacement) const& {
//	let nrvo = this->clone();
//	nrvo.fieldstorer(separator, fieldno, nfields, replacement);
//	return nrvo;
//}

// Mutator
IO   var::fieldstorer(SV separator, const int fieldnx, const int nfieldsx, in replacementx) REF {

	THISIS("void var::fieldstorer(SV separator0, const int fieldnx, const int nfieldsx, in replacementx) &")
	assertStringMutator(function_sig);

	if (separator.empty())
		UNLIKELY
		throw VarError("separator cannot be blank in fieldstorer()");

	// handle multibyte/non-ASCII separators
	std::size_t separator_len = separator.size();

	int fieldno;
	if (fieldnx > 0)
		fieldno = fieldnx;
	else if (fieldnx < 0)
		fieldno = this->count(separator) + 1 + fieldnx + 1;
	else
		fieldno = 1;

	int nfields = nfieldsx > 0 ? nfieldsx : -nfieldsx;

	// pad replacement if required
	var replacement;
	if (nfieldsx >= 0) {
		int nreplacementfields = replacementx.count(separator) + 1;
		// pad to correct number of fields
		if (nreplacementfields < nfields) {
			replacement = replacementx;
			for (; nreplacementfields < nfields; nreplacementfields++)
				replacement ^= separator;
		} else if (nfieldsx && nreplacementfields > nfields) {
			replacement = replacementx.field(separator, 1, nfields);
		} else
			replacement = replacementx;
	} else {
		replacement = replacementx;
	}

	// FIND FIELD

	// find the starting position of the field or return ""
	std::size_t pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		pos = var_str.find(separator, pos);
		// past of of string?
		if (pos == std::string::npos) {
			do {
				var_str += separator;
				fieldn2++;
			} while (fieldn2 < fieldno);
			var_str += replacement.var_str;
			return THIS;
		}
		// pos++;
		pos += separator_len;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::size_t end_pos = pos;
	int pastfieldn = fieldno + nfields;
	while (fieldn2 < pastfieldn) {
		end_pos = var_str.find(separator, end_pos);
		// past of of string?
		if (end_pos == std::string::npos) {
			var_str.replace(pos, std::string::npos, replacement.var_str);
			return THIS;
		}
		// end_pos++;
		end_pos += separator_len;
		fieldn2++;
	}

	// backup to first byte of end of field sep
	end_pos -= (separator_len - 1);

	// insert or replace
	if (end_pos == pos) {
		//insert
		if (nfields == 0)
			replacement.var_str += separator;
		var_str.insert(pos, replacement.var_str);
	} else {
		var_str.replace(pos, end_pos - pos - 1, replacement.var_str);
	}

	return THIS;
}

/////////
// LOCATE
/////////

// hardcore string locate function given a section of a string and all parameters

static bool locateat(SV sv_source, SV sv_target, const char* ordercode, SV usingchar, out setting) {

	// Use vars in AR/DR for numerical ordering
	var var_target;

	// AL localeAwareCompare requires a std::string
	std::string str_target;

	// Analyse order code "", "AL", "AR", "DL", "DR" -> order mode 0 - 4
	int ordermode = 0;
	if (*ordercode) {

		// Locate the order code in a list of the four possible order codes
		// and throw if not found
		const char* ordercodes = "ALARDLDR";
		const char* orderindex = std::strstr(ordercodes, ordercode);
		if (orderindex == nullptr)
			UNLIKELY
			throw VarError("locateby('" ^ var(ordercode) ^ "') is invalid");

		// Convert the memory address to the char position within the codes
		ordermode = static_cast<int>(orderindex - ordercodes);
		// add two and divide by two to get the order no AL=1 AR=2 DL=3 DR=4
		ordermode = (ordermode + 2) >> 1;

		// Initial pass to return position REGARDLESS of order IF target can be found
		// pickos strangeness? locate even if out of order
		// if this is the pickos standard and needs to be implemented properly it should be
		// implemented as continue to search to end instead of search twice like this this will
		// probably be switched off as unnecessary and slow behaviour for EXODUS applications
		// TODO implement as additional check on remainder of fields if returning false in the middle
		//////////////////////////////////////////////////////
		// FIRST PASS
		//////////////////////////////////////////////////////
		if (locateat(sv_source, sv_target, "", usingchar, setting))
			return true;
		//////////////////////////////////////////////////////

		// Use if AR/DR for numerical comparison.
		var_target = sv_target;

		// Use for AL localeAwareCompare
		str_target = sv_target;
	}

	// Initialise positions
	std::size_t pos = 0;
	std::size_t end_pos = sv_source.size();

	// find null in a null field
	if (pos >= end_pos) {
		setting = 1;
		return sv_target.empty();
	}

	// May be searching for multi byte separators
	std::size_t usingchar_len = usingchar.size();

	// Find the starting position of the value or return ""
	// using pos and end_pos of
	auto targetsize = sv_target.size();
	int valuen2 = 1;
	do {

		int comp;
		std::size_t nextpos = sv_source.find(usingchar, pos);

		// if sep not found then next field conceptually starts one after the end of the string
		if (nextpos >= end_pos) {
			nextpos = end_pos;
		}

		SV sv_value = sv_source.substr(pos, nextpos - pos);

		switch (ordermode) {

			// No ordermode
			case '\x00': {

				// Compare using sv for binary sort
				if (sv_value == sv_target) {
					setting = valuen2;
					return true;
				}
				break;
			}

			// AL Ascending Left Justified (ALPHABETIC)
			case '\x01': {

				// Unicode sort?
				auto str_value = std::string(sv_value);
				auto three_way_comparison = var::localeAwareCompare(str_value, str_target);
				if (three_way_comparison >= 0) {
					setting = valuen2;
					if (three_way_comparison == 0)
						// Should not happen since PASS1 also checks strings not vars
						return true;
					else
						return false;
				}
				break;
			}

			// AR Ascending Right Justified (NUMERIC)
			case '\x02': {

				// Compare using var for proper numerical comparison
				let var_value = sv_value;
				if (var_value >= var_target) {
					setting = valuen2;
					if (var_value == var_target)
						// Should not happen but PASS1 only checks strings not vars
						return true;
					else
						return false;
				}
				break;
			}

			// DL Descending Left Justified (ALPHABETIC)
			case '\x03': {

				// Compare using sv
				// TODO use same as AL sort?
				if (sv_value <= sv_target) {
					setting = valuen2;
					if (sv_value == sv_target)
						// Should not happen since PASS1 also checks strings not vars
						return true;
					else
						return false;
				}
				break;
			}

			// DR Descending Right Justified (NUMERIC)
			case '\x04': {

				// Compare using var for proper numerical comparison
				let var_value = sv_value;
				if (var_value <= var_target) {
					setting = valuen2;
					if (var_value == var_target)
						// Should not happen but PASS1 only checks strings not vars
						return true;
					else
						return false;
				}
				break;
			}

			default:
				UNLIKELY
				throw VarError("locateat() invalid ordermode" ^ var(ordermode));

		}

		// If we just processed the last field then quit with false and point to n + 1 field number
		if (nextpos == end_pos) {
			setting = valuen2 + 1;
			return false;
		}

		// Skip over the sep character
		pos = nextpos + usingchar_len;

		valuen2++;

	} while (true);
}

// locate within extraction
static bool locatex(SV source, SV target, const char* ordercode, SV usingchar, out setting, int fieldno, int valueno, const int subvalueno) {
	// private - assume everything is defined/assigned correctly

	// any negatives at all returns ""
	// done inline since unusual
	// if (fieldno<0||valueno<0||subvalueno<0) return ""

	// zero means all, negative return ""
	// if (fieldno<=0)     (but locate x<0> using VM should work too
	if (fieldno <= 0) {
		// locate negative field number always returns false and setting 1
		if (fieldno < 0) {
			setting = 1;
			return !target.size();
		}

		if (valueno || subvalueno)
			fieldno = 1;
		else {
			return locateat(source, target, ordercode, usingchar, setting);
		}
	}

	// find the starting position of the field or return ""
	std::size_t pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		pos = source.find(FM_, pos);
		// past of of string?
		if (pos == std::string::npos) {
			// if (valueno||subvalueno) setting=1;
			// else setting=fieldn2+1;
			setting = 1;
			return !target.size();
		}
		pos++;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::size_t field_end_pos;
	field_end_pos = source.find(FM_, pos);
	if (field_end_pos == std::string::npos)
		field_end_pos = source.size();

	// FIND VALUE

	if (pos >= field_end_pos) {
		setting = 1;
		return !target.size();
	}

	// zero means all, negative return ""
	if (valueno <= 0) {
		if (valueno < 0) {
			setting = 1;
			return !target.size();
		}
		if (subvalueno)
			valueno = 1;
		else
			return locateat(SV(source.data() + pos, field_end_pos - pos), target, ordercode, usingchar, setting);
	}

	// find the starting position of the value or return ""
	// using pos and end_pos of
	int valuen2 = 1;
	SV sv1 = SV(source.data(), field_end_pos);
	while (valuen2 < valueno) {
		pos = sv1.find(VM_, pos);
		// past end of string?
		if (pos == std::string::npos) {
			// if (subvalueno) setting=1;
			// else setting=valuen2+1;
			setting = 1;
			return !target.size();
		}
		pos++;
//		// past end of field?
//		if (pos > field_end_pos) {
//			// setting=valuen2+1;
//			setting = 1;
//			return !target.size();
//		}
		valuen2++;
	}

	// find the end of the value (or string)
	std::size_t value_end_pos;
	value_end_pos = sv1.find(VM_, pos);
//	if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
	if (value_end_pos > field_end_pos)
		value_end_pos = field_end_pos;

	// FIND SUBVALUE

	if (pos >= value_end_pos) {
		setting = 1;
		return !target.size();
	}

	if (subvalueno <= 0) {

		// zero means all
		if (subvalueno == 0)
			return locateat(SV(source.data() + pos, value_end_pos - pos), target, ordercode, usingchar, setting);

		// negative means ""
		else {
			setting = 1;
			return !target.size();
		}
	}

	// find the starting position of the subvalue or return ""
	// using pos and end_pos of
	int subvaluen2 = 1;
	SV sv2 = SV(source.data(), value_end_pos);
	while (subvaluen2 < subvalueno) {
		pos = sv2.find(SM_, pos);
		// past end of string?
		if (pos == std::string::npos) {
			// setting=subvaluen2+1;
			setting = 1;
			return !target.size();
		}
		pos++;
//		// past end of value?
//		if (pos > value_end_pos) {
//			// setting=subvaluen2+1;
//			setting = 1;
//			return !target.size();
//		}
		subvaluen2++;
	}

	// find the end of the subvalue (or string)
	std::size_t subvalue_end_pos;
	subvalue_end_pos = sv2.find(SM_, pos);
//	if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos) {
	if (subvalue_end_pos > value_end_pos) {
		return locateat(SV(source.data() + pos, value_end_pos - pos), target, ordercode, usingchar, setting);
	}

	return locateat(SV(source.data() + pos, subvalue_end_pos - pos), target, ordercode, usingchar, setting);
}

bool var::locate(in target, out setting, const int fieldno, const int valueno /*=0*/) const {

	THISIS("bool var::locate(in target, out setting, const int fieldno, const int valueno) const")
	assertString(function_sig);
	ISSTRING(target)
	ISVAR(setting)

	std::string usingchar;
	if (valueno != 0)
		usingchar = _SM;
	else if (fieldno != 0)
		usingchar = _VM;
	else
		usingchar = _FM;
	// if (fieldno<=0) *usingchar=FM_;
	// else if (valueno<=0) *usingchar=VM_;
	// else *usingchar=SM_;

	return locatex(var_str, target.var_str, "", usingchar, setting, fieldno, valueno, 0);
}

//// default locate using VM
//bool var::locate(in target, out setting) const {
//
//	THISIS("bool var::locate(in target, out setting, const int fieldno, const int valueno) const")
//	assertString(function_sig);
//	ISSTRING(target)
//	ISVAR(setting)
//
//	return locatex(var_str, target.var_str, "", _VM, setting, 0, 0, 0);
//}
//
//// without setting
//bool var::locate(in target) const {
//
//	THISIS("bool var::locate(in target) const")
//	assertString(function_sig);
//	ISSTRING(target)
//
//	let setting;
//	return locatex(var_str, target.var_str, "", _VM, setting, 0, 0, 0);
//}

//// without setting
//bool var::locate(in target) const {
//
//	THISIS("bool var::locate(in target) const")
//	assertString(function_sig);
//	ISSTRING(target)
//
//	// locate "" always false
//	if (!target.var_str.size())
//		return false;
//
//	std::size_t pos = 0;
//	std::size_t size = var_str.size();
//
//	while (pos < var_str.size()) {
//
////		printl();
////		TRACE(pos)
//
//		std::size_t nextpos = var_str.find_first_of("\x1a\x1b\x1c\x1d\x1e\x1f", pos);
////		TRACE(nextpos)
//
//		if (nextpos == std::string::npos) {
////			TRACE(var_str.substr(pos, size - pos))
//			return (target.var_str == var_str.substr(pos, size - pos));
//		}
//
////		TRACE(var_str.substr(pos, nextpos - pos))
//
//		// nextpos - pos = 6 - 4 = 2
//		if (target.var_str == var_str.substr(pos, nextpos - pos))
//			return true;
//
//		pos = nextpos;
//		pos++;
//	}
//
//	// find anything in "" always false
//	return false;
//}

// Utility to count any and all field marks in string
static inline std::size_t count_all_field_marks(SV sv1) {

	// Count any of the field marks before our found substr
	auto iter = sv1.begin();
	auto end2 = sv1.end();
	std::size_t result = 0;

	while (iter != end2) {
		if ((*iter) <= RM_ && (*iter) >= ST_) {
			result++;
		}
		iter++;
	}
	return result;
}

// locate with setting.
// Returns nfields + 1 in setting if not found
bool var::locate(in target, out setting) const {

//	THISIS("bool var::locate(in target, out setting) const")
//	assertString(function_sig);
//	ISSTRING(target)
//	ISVAR(setting)
//	return locatex(var_str, target.var_str, "", _VM, setting, 0, 0, 0);

	setting = this->locate(target);

	if (setting)
		return true;

	// If not found then return next number (nfields + 1) in setting
	setting = count_all_field_marks(var_str);
	if (not var_str.empty())
		setting++;
	setting++;

	return false;
}

// hard coded utility to find any of the field marks instead of find_first_of
//pos = var_str.find_first_of("\x1a\x1b\x1c\x1d\x1e\x1f", pos2);
//pos = find_first_fm(var_str, pos2);
inline std::size_t find_first_fm(const std::string& s1, std::size_t pos) {
	auto iter = s1.c_str() + pos;
	for (;;) {
		if (*iter <= RM_) {
			if (*iter >= ST_)
				return iter - s1.c_str();
			if (*iter == 0)
				return std::string::npos;
		}
		iter++;
	}
}

// locate without setting.
// Returns field/value number etc. if found else 0
var var::locate(in target) const {

	THISIS("var  var::locate(in target) const")
	assertString(function_sig);
	ISSTRING(target)

	const std::size_t target_size = target.var_str.size();

//	if (!target_size)
//		return 0;
//	// Find empty multivalues.
//	if (!target_size) {
//		int result = 0;
//		pos = find_first_fm(var_str, pos2);
//		if (pos == std::string::npos)
//			return 0;
//		return 0;
//	}

	std::size_t target_pos = 0;
	const std::size_t source_size = var_str.size();
	while (target_pos < source_size) {

//TRACE("Searching");
//TRACE(target_pos)

		target_pos = var_str.find(target.var_str, target_pos);

		// Fail if not found
		if (target_pos == std::string::npos) {
//TRACE("not found")
			return 0;
		}
//TRACE(target_pos)

		std::size_t target_end = target_pos + target_size;
		std::size_t result;
		char c;

		// Skip if target_pos > 0 and prior char is not any one of the field marks
		if (target_pos > 0) {
			c = var_str[target_pos - 1];
			if (c > RM_ || c < ST_) {
				goto next_search;
			}
		}

		// At this point, target_pos is at the beginning of the source or just after a field mark

		// Success if at the end
		if (target_end >= source_size) {
found:
			return count_all_field_marks(SV(var_str.data(), target_pos)) + 1;
		}

//		TRACE(target_end)

		// Success if the following char is any one of the field marks
		c = var_str[target_end];
		if (c <= RM_ && c >= ST_)
			goto found;

next_search:
		// Search for any one of the field marks and fail if not found
		// TODO Can we start next search from target_end to jump over our matched string?
		// target_pos = var_str.find_first_of("\x1a\x1b\x1c\x1d\x1e\x1f", target_pos);
//		target_pos = var_str.find_first_of("\x1a\x1b\x1c\x1d\x1e\x1f", target_end);
		target_pos = find_first_fm(var_str, target_end);
//		TRACE(target_pos)
		if (target_pos == std::string::npos)
			return 0;

		// Start the next search one char after the next field mark found
		target_pos++;

	}

	// find anything in "" always false
	// BUT find "" in "" is TRUE
	//TRACE(target_size)
	if (target_size == 0) {
		if (var_str.size() == 0)
			return 1;
		char last_char = var_str.back();
		if (last_char <= RM_ && last_char >= ST_)
			return count_all_field_marks(SV(var_str)) + 1;
	}
	return 0;
}

////////////
// LOCATE BY
////////////

//// 1. rare syntax where the order is given as a variable
//bool var::locateby(in ordercode, in target, out setting, const int fieldno, const int valueno /*=0*/) const {
//	return locateby(ordercode.toString().c_str(), target, setting, fieldno, valueno);
//}

//// 2. no fieldno or valueno means locate using character VM
//// caters for the rare syntax where the order is given as a variable
//bool var::locateby(in ordercode, in target, out setting) const {
//	return locateby(ordercode.toString().c_str(), target, setting);
//}

// 3. no fieldno or valueno means locate using character VM
// specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locateby(const char* ordercode, in target, out setting) const {

	THISIS("bool var::locateby(const char* ordercode, in target, out setting) const")
	assertString(function_sig);
	ISSTRING(target)
	ISVAR(setting)

	// TODO either make a "locatefrom" version of the above where the locate STARTS its search
	// from the last numbered subvalue (add a new parameter), value or field. OR possibly modify
	// this function to understand a negative number as "start from" instead of "within this"

	// determine locate by field, value or subvalue depending on the parameters as follows:
	// if value number is given then locate in subvalues of that value
	// if field number is given then locate in values of that field
	// otherwise locate in fields of the string

//	return locatex(var_str, target.var_str, ordercode, _VM, setting, 0, 0, 0);
	return locateat(var_str, target, ordercode, _VM, setting);
}

// 4. specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locateby(const char* ordercode, in target, out setting, const int fieldno, const int valueno /*=0*/) const {

	THISIS("bool var::locateby(const char* ordercode, in target, out setting, const int fieldno, const int valueno) const")
	assertString(function_sig);
	ISSTRING(target)
	ISVAR(setting)

	// TODO either make a "locatefrom" version of the above where the locate STARTS its search
	// from the last numbered subvalue (add a new parameter), value or field. OR possibly modify
	// this function to understand a negative number as "start from" instead of "within this"

	// determine locate by field, value or subvalue depending on the parameters as follows:
	// if value number is given then locate in subvalues of that value
	// if field number is given then locate in values of that field
	// otherwise locate in fields of the string
	std::string usingchar;
	if (valueno != 0)
		usingchar = SM_;
	else if (fieldno != 0)
		usingchar = VM_;
	else
		usingchar = FM_;
	// if (fieldno<=0) usingchar=FM_;
	// else if (valueno<=0) usingchar=VM_;
	// else usingchar=SM_;

	return locatex(var_str, target.var_str, ordercode, usingchar, setting, fieldno, valueno, 0);
}

///////////////////
// LOCATE BY, USING
///////////////////

bool var::locatebyusing(const char* ordercode, const char* usingchar, in target, out setting, const int fieldno /*=0*/, const int valueno /*=0*/, const int subvalueno /*=0*/) const {

	THISIS("bool var::locatebyusing(const char* ordercode, const char* usingchar, in target, out setting, const int fieldno, const int valueno, const int valueno) const")
	assertString(function_sig);
	ISSTRING(target)
	ISVAR(setting)

	// TODO either make a "locatefrom" version of the above where the locate STARTS its search
	// from the last numbered subvalue (add a new parameter), value or field. OR possibly modify
	// this function to understand a negative number as "start from" instead of "within this"

	// determine locate by field, value or subvalue depending on the parameters as follows:
	// if value number is given then locate in subvalues of that value
	// if field number is given then locate in values of that field
	// otherwise locate in fields of the string

	return locatex(var_str, target.var_str, ordercode, usingchar, setting, fieldno, valueno, subvalueno);
}

///////////////
// LOCATE USING
///////////////

// 1. simple version
bool var::locateusing(const char* usingchar, in target) const {

	THISIS("bool var::locateusing(const char* usingchar, in target) const")
	assertString(function_sig);
	ISSTRING(target)

	var setting = "";
//	return locatex(var_str, target.var_str, "", usingchar, setting, 0, 0, 0);
	return locateat(var_str, target, "", usingchar, setting);

}

// 2. specify field/value/subvalue and return position
bool var::locateusing(const char* usingchar, in target, out setting, const int fieldno /*=0*/, const int valueno /*=0*/, const int subvalueno /*=0*/) const {

//	THISIS("bool var::locateusing(const char* usingchar, in target, out setting, const int fieldno, const int valueno, const int subvalueno) const")
	THISIS("bool var::locateusing(const char* usingchar, in target, out setting, const int fieldno, const int valueno, const int subvalueno) const")
	assertString(function_sig);
	ISSTRING(target)
	ISVAR(setting)

	return locatex(var_str, target.var_str, "", usingchar, setting, fieldno, valueno,
				   subvalueno);
}

//////////
// EXTRACT
//////////

// Old "extract()" function in either procedural or OO style.
//     xxx = extract(yyy, 1, 2, 3)
//  or xxx = yyy.extract(1, 2, 3)
//
//var  var::extract(const int argfieldn, const int argvaluen/*=0*/, const int argsubvaluen/*=0*/) const {
//	return this->f(argfieldn, argvaluen, argsubvaluen);
//}

// Abbreviated xxxx.f(1,2,3) syntax. PickOS angle bracket syntax (xxx<1,2,3>) not possible in C++
//     xxx = yyy.f(1,2,3)
//
var  var::f(const int argfieldn, const int argvaluen/*=0*/, const int argsubvaluen/*=0*/) const {

	THISIS("var  var::f(const int argfieldn, const int argvaluen, const int argsubvaluen) const")
	assertString(function_sig);

#if 0
	// Trying "improved" versions of extraction on random smallish records with few fields/values/subvalues
	// Very strange because the original version redundantly searches for the end of the field before looking for values etc.
	// -original- min:  7ns avg: 14ns - the original is the winner ... strange (additional function call?)
	// extract_v2 min: 11ns avg: 26ns
	// extract_v3 min: 11ns avg: 20ns
	// extract_v4 min: 11ns avg: 19ns
	int outstart;
	int outlength;
	extract_v4(var_str.data(), int(var_str.size()), argfieldn, argvaluen, argsubvaluen, &outstart, &outlength);
	return var_str.substr(outstart, outlength);
#else
	// any negatives at all returns ""
	// done inline since unusual
	// if (fieldno<0||valueno<0||subvalueno<0) return ""

	// FIND FIELD

	int fieldno = argfieldn;
	int valueno = argvaluen;
	int subvalueno = argsubvaluen;
	// zero means all, negative return ""
	if (fieldno <= 0) {
		if (fieldno < 0)
			return "";
		if (valueno || subvalueno)
			fieldno = 1;
		else
			return var_str;
	}

	// unless extracting field 1,
	// find the starting position of the field or return ""
	std::size_t pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		pos = var_str.find(FM_, pos);
		// past of of string?
		if (pos == std::string::npos)
			return "";
		pos++;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::size_t field_end_pos;
	field_end_pos = var_str.find(FM_, pos);
	if (field_end_pos == std::string::npos)
		field_end_pos = var_str.size();

	// FIND VALUE

	// zero means all, negative return ""
	if (valueno <= 0) {
		if (valueno < 0)
			return "";
		if (subvalueno)
			valueno = 1;
		else
			return var_str.substr(pos, field_end_pos - pos);
	}

	// unless extracting value 1,
	// find the starting position of the value or return ""
	// using pos and end_pos of
	int valuen2 = 1;
	SV sv1 = SV(var_str.data(), field_end_pos);
	while (valuen2 < valueno) {
		pos = sv1.find(VM_, pos);
		// past end of string?
		if (pos == std::string::npos)
			return "";
		pos++;
//		// past end of field?
//		if (pos > field_end_pos)
//			return "";
		valuen2++;
	}

	// find the end of the value (or string)
	std::size_t value_end_pos;
	value_end_pos = sv1.find(VM_, pos);
//	if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
	if (value_end_pos > field_end_pos)
		value_end_pos = field_end_pos;

	// FIND SUBVALUE

	// zero means all, negative means ""
	if (subvalueno == 0)
		return var_str.substr(pos, value_end_pos - pos);
	if (subvalueno < 0)
		return "";

	// unless extracting subvalue 1,
	// find the starting position of the subvalue or return ""
	// using pos and end_pos of
	int subvaluen2 = 1;
	SV sv2 = SV(var_str.data(), value_end_pos);
	while (subvaluen2 < subvalueno) {
		pos = sv2.find(SM_, pos);
		// past end of string?
		if (pos == std::string::npos)
			return "";
		pos++;
//		// past end of value?
//		if (pos > value_end_pos)
//			return "";
		subvaluen2++;
	}

	// find the end of the subvalue (or string)
	std::size_t subvalue_end_pos;
	subvalue_end_pos = sv2.find(SM_, pos);
//	if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
	if (subvalue_end_pos > value_end_pos)
		return var_str.substr(pos, value_end_pos - pos);

	return var_str.substr(pos, subvalue_end_pos - pos);
#endif

}

///////////////
//STARTS / ENDS
///////////////

//starting is equivalent to x::index(y) == 1
//contains is equivalent to x::index(y) != 0

bool var::starts(SV prefix) const {

	THISIS("bool var::starts(SV prefix) const")
	assertString(function_sig);

	// Differ from c++, javascript, python3 - see comment on var::contains
	if (prefix.empty()) {
		var(function_sig).errputl();
//		VarError e(__PRETTY_FUNCTION__);
//		e.message.errput();
//		e.stack(1).f(1).errputl();
		return false;
	}

	return var_str.starts_with(prefix);
}

bool var::ends(SV suffix) const {

	THISIS("bool var::ends(SV suffix) const")
	assertString(function_sig);

	// DIFFERS from c++, javascript, python3 - see comment on var:contains
	if (suffix.empty()) {
		var(function_sig).errputl();
//		VarError e(__PRETTY_FUNCTION__);
//		e.message.errput();
//		e.stack(1).f(1).errputl();
		return false;
	}

	return var_str.ends_with(suffix);
}

bool var::contains(SV substr) const {

	THISIS("bool var::contains(SV substr) const")
	assertString(function_sig);

	// DIFFERS from c++, javascript, python3
	//
	// Human logic: "" is not equal to "x" therefore x does not contain "".
	//
	// Human logic: Check each item (character) in the list for equality with what I am looking for and return success if any are equal.
	//
	// Programmer logic: Compare as many characters as are in the search string for presence in the list of characters and return success if there are no failures.
	//
	if (substr.empty()) {
		return false;
	}

#ifdef __cpp_lib_string_contains
	//C++23
	return var_str.contains(substr);
#else
	return var_str.find(substr) != std::string::npos;
#endif

}

////////
// FIRST
////////

//[1,1]
// .substr(1,1)
var  var::first() const& {

	THISIS("var  var::first() const")
	assertString(function_sig);

	// Return "" if empty
	if (var_str.empty()) {
		UNLIKELY
		return "";
	}

	// Return the first char only
	return var_str.front();

}

//[1,1]
// .substr(1,1)
IO   var::firster() REF {

	THISIS("void var::firster() &")
	assertStringMutator(function_sig);

	// Reduce the size of this string to max 1
	if (var_str.size() > 1) {
		var_str.resize(1);
	}

	return THIS;
}

var  var::first(const std::size_t  length) const& {

	THISIS("var  var::first(const std::size_t length) const")
	assertString(function_sig);

	// Assume high half of std::size_t is c++ unblockable conversion
	// of negative ints to std::size_t. Runtime error
	if (length > std::string::npos >> 1)
		UNLIKELY
		throw VarNonPositive(__PRETTY_FUNCTION__);

	// Return a new var with the required number of chars from this or all
	return var(this->var_str.data(), std::min(length, this->var_str.size()));
}


//[1,y]
// var.substr(1,length)
IO   var::firster(const std::size_t length) REF {

	THISIS("void var::firster(const std::size_t length) &")
	assertStringMutator(function_sig);

	// Assume high half of std::size_t is c++ unblockable conversion
	// of negative ints to std::size_t. Runtime error
	if (length > std::string::npos >> 1)
		UNLIKELY
		throw VarNonPositive(__PRETTY_FUNCTION__);

	// Reduce the size of this string if necessary
	if (length < this->var_str.size()) {
		this->var_str.resize(length);
	}

	return THIS;
}

///////
// LAST
///////

// [-1]
// .substr(1, 1)
var  var::last() const& {

	THISIS("var  var::last() const")
	assertString(function_sig);

	if (var_str.empty()) {
		UNLIKELY
		return "";
	}

	return var_str.back();

}

//[-1]
// .substr(-1,1)
IO   var::laster() REF {

	THISIS("void var::laster() &")
	assertStringMutator(function_sig);

	// Leave only the last char
	if (var_str.size() > 1)
		var_str = var_str.back();

	return THIS;
}


var  var::last(const std::size_t  length) const& {

	THISIS("var  var::last(const std::size_t length) const")
	assertString(function_sig);

	// Assume high half of std::size_t is c++ unblockable conversion
	// of negative ints to std::size_t. Runtime error
	if (length > std::string::npos >> 1)
		UNLIKELY
		throw VarNonPositive(__PRETTY_FUNCTION__);

	// Example "abc".last(2)
	// min of 2, 3 -> 2 for copylen
	// copy start = data() +3 -2 = data+1, copylen 2

//	std::size_t copylen = std::min(length, this->var_str.size());
//
//	// Construct a new var with the required number of chars from this
//	let nrvo(this->var_str.data() + this->var_str.size() - copylen, copylen);
//
//	return nrvo;
	if (length >= var_str.size())
		return var_str;
	else
		return var_str.substr(var_str.size() - length);
}
//__cpp_lib_string_contains

//[-y]
// var.s(-length) substring
IO   var::laster(const std::size_t length) REF {

	THISIS("void var::laster(const std::size_t length) &")
	assertStringMutator(function_sig);

	// Assume high half of std::size_t is c++ unblockable conversion
	// of negative ints to std::size_t. Runtime error
	if (length > std::string::npos >> 1)
		UNLIKELY
		throw VarNonPositive(__PRETTY_FUNCTION__);

	// Example "abc".last(2)
	// 2 < 3
	// erase 0, 3 - 2 = 1

	// Erase the first part of this string if necessary
	if (length < this->var_str.size()) {
		this->var_str.erase(0, this->var_str.size() - length);
	}

	return THIS;
}


//////
// CUT
//////

// Pick OS equivalent
// var[1,length] = ""         cut first n bytes
// var[-length, length] = ""  cut last n bytes

var  var::cut(const int length) const& {

	THISIS("var  var::cut(const int length) const")
	assertString(function_sig);

	// Positive or zero. Trim first n bytes

	if (length >= 0) {
		LIKELY

		if (static_cast<size_t>(length) >= var_str.length()) {

			// Number of bytes to trim matches or exceeds string length.
			// Example "ab".cut(2) return ""
			// Example "ab".cut(3) return ""
			return "";
		}

		// Example "ab".cut(0) return "ab"
		// Example "ab".cut(1) return "b"
		return var_str.substr(length);

	}

	// Negative. Trim last n bytes

	size_t nbytes_to_trim = static_cast<size_t>(-length);

	// Number of bytes to trim matches or exceeds string length.
	if (nbytes_to_trim >= var_str.length()) {
		// Example "ab".cut(-2) return ""
		// Example "ab".cut(-3) return ""
		return "";
	}

	// Example "ab".cut(-1) return "a"
	return var_str.substr(0, var_str.length() - nbytes_to_trim);

}

// Pick OS equivalent
// x[1, length] = ""
// x[-length, length] = ""

IO   var::cutter(const int length) REF {

	THISIS("void var::cutter(const int length) &")
	assertStringMutator(function_sig);

	if (length >= 0) {
		LIKELY

		// Positive or zero. Trim first n bytes

		if (static_cast<size_t>(length) >= var_str.length()) {

			// Number of bytes to trim matches or exceeds string length.
			// Example "ab".cutter(2) return ""
			// Example "ab".cutter(3) return ""
			var_str.clear();

		} else {

			// Example "ab".cutter(0) return "ab"
			// Example "ab".cutter(1) return "b"
			var_str.erase(0, length);
		}

	} else {

		// Negative. Trim last n bytes

		size_t nbytes_to_trim = static_cast<size_t>(-length);
		if (nbytes_to_trim >= var_str.length()) {

			// Number of bytes to trim matches or exceeds string length.
			// Example "ab".cutter(-2) return ""
			// Example "ab".cutter(-3) return ""
			var_str.clear();

		} else {

			// Example "ab".cutter(-1) return "a"
			var_str.erase(var_str.length() - nbytes_to_trim);
//			var_str.resize(var_str.length() - nbytes_to_trim);
		}
	}

	return THIS;
}

// Old versions

//// var[1,length] = ""         cut first n bytes
//// var[-length, length] = ""  cut last n bytes
//var  var::cut(const int length) const& {
//
//	THISIS("var  var::cut(const int length) const")
//	assertString(function_sig);
//
//	let nrvo;
//	nrvo.var_typ = VARTYP_STR;
//
//	// Assume var_str size is <= max int
//
//	if (length >= static_cast<int>(var_str.size())) {
//		// Example "ab".cut(2) return ""
//		// Example "ab".cut(3) return ""
//		// return empty string
//	}
//
//	else if (length >= 0) {
//		// Positive - Copy from middle to end
//		// Example "ab".cut(0) , append from pos 0, return "ab"
//		// Example "ab".cut(1) , append from pos 1, return "b"
//		nrvo.var_str.append(var_str, length, std::string::npos);
//
//	} else {
//		// Negative = Copy first n chars
//		// Example "ab".cut(-1) copyn = -1 + 2 = 1, return "a"
//		// Example "ab".cut(-2) copyn = -2 + 2 = 0, return ""
//		// Example "ab".cut(-3) copyn = -3 + 2 = -1, return ""
//		auto copyn = length + static_cast<int>(var_str.size());
//		if (copyn > 0)
//			nrvo.var_str.append(var_str, 0, copyn);
//	}
//
//	return nrvo;
//}

//// x[1, length] = ""
//// x[-length, length] = ""
//io   var::cutter(const int length) {
//
//	THISIS("io   var::cutter(const int length)")
//	assertStringMutator(function_sig);
//
//	if (length >= 0 ) {
//
//		// Positive - cut first n chars. Erase from first char.
//		// Example "ab".cutter(0) , erase 0, return "ab"
//		// Example "ab".cutter(1) , erase 1, return "b"
//		// Example "ab".cutter(2) , erase 2, return ""
//		// Example "ab".cutter(3) , erase 3, return ""
//		var_str.erase(0, length);
//
//	}
//	// warning: comparison of integer expressions of different signedness: ‘int’ and ‘std::__cxx11::basic_string<char>::size_t’ {aka ‘long unsigned int’} [-Wsign-compare]
//	else if (-length >= static_cast<int>(var_str.size())) {
//
//		// Negative = cut last n chars. Erase from middle to end.
//		// Example "ab".cutter(-2) pos = -2 + 2 = erase_pos 0, return ""
//		// Example "ab".cutter(-3) pos = -3 + 2 = erase_pos -1, return ""
//		var_str.clear();
//
//	} else {
//
//		// Negative = cut last n chars. Erase from middle to end.
//		// Example "ab".cutter(-1) pos = -1 + 2 = erase_pos 1, return "a"
//		auto erase_pos = length + var_str.size();
//		var_str.erase(erase_pos, std::string::npos);
//	}
//
//	return *this;
//}


/////////
// SUBSTR
/////////

////ND var substr(const int pos1, const int length) const&; // byte pos1, length
//ND var  var::substr(const int pos1, const int length) const& {
//	let nrvo = this->clone();
//	nrvo.substrer(pos1, length);
//	return nrvo;
//}
//ND var substr(const int pos1) const&;                   // byte pos1
//ND var  var::substr(const int pos1) const& {
//	let nrvo = this->clone();
//	nrvo.substrer(pos1);
//	return nrvo;
//}

//[x,y]
// var.s(start,length) substring
IO   var::substrer(const int pos1, const int length) REF {

	THISIS("void var::substrer(const int pos1, const int length) &")
	assertStringMutator(function_sig);

	// return "" for ""
	int max = static_cast<int>(var_str.size());
	if (max == 0) {
		var_str.clear();
		return THIS;
	}

	int start = pos1;

	// negative length means reverse the string
	if (length <= 0) {
		if (length == 0) {
			var_str.clear();
			return THIS;
		}

		// sya
		if (start < 1) {
			if (start == 0) {
				var_str.clear();
				return THIS;
			}
			start = max + start + 1;
			if (start < 1) {
				var_str.clear();
				return THIS;
			}
		} else if (start > max) {
			start = max;
		}

		int stop = start + length + 1;
		if (stop < 1)
			stop = 1;

		std::string result;
		for (int ii = start; ii >= stop; ii--)
			result += var_str[ii - 1];

		var_str = result;
		return THIS;
	}

	if (start < 1) {
		if (start == 0)
			start = 1;
		else {
			start = max + start + 1;
			if (start < 1)
				start = 1;
		}
	} else if (start > max) {
		var_str.clear();
		return THIS;
	}
	int stop = start + length;

	// wcout<<start-1<<stop-start<<endl;

	// TODO use erase for speed instead of copying whole string
	var_str = var_str.substr(start - 1, stop - start);

	return THIS;
}

//////
/// AT
//////
//    ND var at(const int pos1) const;

template<> PUBLIC RETVAR VARBASE1::at(const int charno) const {

	THISIS("var  var::at(const int charno) const")
	assertString(function_sig);

	int nchars = static_cast<int>(var_str.size());

	// beyond end of string return ""
	// get this test out of the way first since it only has to be done later on anyway
	if (charno > nchars)
		return "";

	// within string return the character
	// handle positive indexing first for speed on the assumption
	// that it is commoner than negative indexing
	if (charno > 0)
		return var_str[charno - 1];

	// character 0 return the first character or "" if none
	// have to get this special case out of the way first
	// despite it being unusual
	// since it only has to be done later anyway
	if (charno == 0) {
		if (nchars)
			return var_str[0];
		else
			return "";
	}

	// have to check this later so check it now
	if (!nchars)
		return "";

	// convert negative index to positive index
	int charno2 = nchars + charno;

	// if index is now 0 or positive then return the character
	if (charno2 >= 0)
		return var_str[charno2];	// no need for -1 here

	// otherwise so negative as to point before beginning of string
	// and rule is to return the first character in that case
	return var_str[0];
}

template<> PUBLIC RETVAR VARBASE1::operator[](const int pos1) const {
	return this->at(pos1);
}

/////////////////////////////////////
// MV (multivalued +++ --- *** /// :::
/////////////////////////////////////

// performs an operation + - * / : on two multivalued strings in parallel
// returning a multivalued string of the results
var  var::mv(const char* opcode, in var2) const {

	THISIS("var  var::multivalued(const char* opcode, in var2) const")
	assertString(function_sig);
	ISSTRING(var2)

	var nrvo = "";
	var mv1;
	var mv2;
	char separator1 = VM_;
	char separator2 = VM_;

	// pointers into var_str
	// p1a and p1b are zero based indexes of first and last+1 characters of a value in var1
	// (this)
	std::size_t p1a = 0;
	std::size_t p1b;

	// pointers into var2.var_str
	// p2a and p2b are zero based indexes of first and last+1 characters of a value in var2
	std::size_t p2a = 0;
	std::size_t p2b;

	while (true) {

		char separator1_prior = separator1;

		// find the end of a value in var1 (this)
		if (separator1 <= separator2) {
getnextp1:
//			p1b = var_str.find_first_of(_RM _FM _VM _SM _TM _ST, p1a);
			p1b = find_first_fm(var_str, p1a);
			if (p1b == std::string::npos) {
				separator1 = RM_ + 1;
			} else {
				separator1 = var_str[p1b];
			}
			mv1 = var(var_str.substr(p1a, p1b - p1a));  //.outputl("mv1=");
			p1a = p1b;
		}

		// find the end of a value in var1 (this)
		if (separator2 <= separator1_prior) {
getnextp2:
//			p2b = var2.var_str.find_first_of(_RM _FM _VM _SM _TM _ST, p2a);
			p2b = find_first_fm(var2.var_str, p2a);
			if (p2b == std::string::npos) {
				separator2 = RM_ + 1;
			} else {
				separator2 = var2.var_str[p2b];
			}
			mv2 = var(var2.var_str.substr(p2a, p2b - p2a));	 //.outputl("mv2=");
			p2a = p2b;
		}

		switch (opcode[0]) {

			case '+':
				nrvo ^= mv1 + mv2;
				break;

			case '-':
				nrvo ^= mv1 - mv2;
				break;

			case '*':
				nrvo ^= mv1 * mv2;
				break;

			case '/':
				// if mv is anything except empty or zero
				// OR if mv is empty and mv2 is not empty or zero
				// may trigger non-numeric or div-by-zero errors
				// 1. if both empty then result is empty
				// 2. empty or zero, divided by zero, is empty or zero
				if (mv1)
					mv1 = mv1 / mv2;
				else
					mv1 = 0;
				nrvo ^= mv1;
				break;

			case ':':
				nrvo ^= mv1 ^ mv2;
				break;
			default:
				// TODO should this be a different exception since it is not caused by application programmer?
				throw VarError("EXODUS: Error in " ^ var(__PRETTY_FUNCTION__) ^ " opcode " ^ var(opcode[0]) ^ "is not valid. SHould be one of +, -, *, / or :");
		}

		if (separator1 == separator2) {

			// if both pointers past end of their strings then we are done
			if (separator1 > RM_)
				break;

			nrvo ^= separator1;
			// nrvo.convert(_VM _FM, "]^").outputl("= nrvo=");
			p1a++;
			p2a++;
		} else if (separator1 < separator2) {
			nrvo ^= separator1;
			// nrvo.convert(_VM _FM, "]^").outputl("< nrvo=");
			mv2 = "";
			p1a++;
			separator1_prior = separator1;
			goto getnextp1;
		} else {
			nrvo ^= separator2;
			// nrvo.convert(_VM _FM, "]^").outputl("> nrvo=");
			mv1 = "";
			p2a++;
			goto getnextp2;
		}
	}

	return nrvo;
}

////////
// SUBSTR upto any specified characters - similar to var::substr3
////////

// version 3 returns the characters up to the next delimiter
// also returns the index of the next delimiter discovered or 1 after the string if none (like
// COL2() in pickos) NOTE pos1 is 1 based not 0. anything less than 1 is treated as 1
var  var::substr(const int pos1, SV delimiterchars, out pos2) const {

	THISIS("var  var::substr(const int pos1, SV delimiterchars, out pos2) const")
	assertString(function_sig);
//	ISSTRING(delimiterchars)

	std::size_t pos0;

	// domain check min pos1
	// handle before start of string
	// pos1 arg is 1 based per mv/pick standard
	// remove treats anything below 1 as 1
	// pos variable is zero based standard c++ logic
	if (pos1 > 0)
		pos0 = pos1 - 1;
	else
		pos0 = 0;

	// domain check max pos1
	// handle after end of string
	if (pos0 >= var_str.size()) {
		pos2 = static_cast<int>(var_str.size() + 1);
		return "";
	}

	// find the end of the field (or string)
	std::size_t end_pos;
	if (delimiterchars.size() == 1)
		end_pos = var_str.find(delimiterchars[0], pos0);
	else
		end_pos = var_str.find_first_of(delimiterchars, pos0);
//		end_pos = var_str.find(delimiterchars, pos0);

	// past of of string?
	if (end_pos == std::string::npos) {
		pos2 = static_cast<int>(var_str.size() + 1);
		//return var_str.substr(pos, var_str.size() - pos);
		return var_str.substr(pos0);
	}

	// return the index of the dicovered delimiter
	// unlike remove which returns the index of one AFTER the discovered delimiter
	pos2 = static_cast<int>(end_pos + 1);

	// extract and return the substr as well
	return var_str.substr(pos0, end_pos - pos0);
}

////////
// SUBSTR2 - similar to substr(startindex,delimiterchars) was called remove() in pickos
////////

// returns the characters up to the next delimiter
// delimiter returned as numbers RM=1F=1 FM=1E=2, VM=1D=3 SM=1C=4 TM=1B=5 to ST=1A=6 or 0 if not found
// NOTE pos1 is 1 based not 0. anything less than 1 is treated as 1
var  var::substr2(io pos1, out delimiterno) const {

	THISIS("var  var::substr2(io pos1, out delimiterno) const")
	assertString(function_sig);
	ISNUMERIC(pos1)
	ISVAR(delimiterno)

	int startindex0 = pos1.toInt() - 1;
	std::size_t pos = (startindex0 >= 0) ? startindex0 : 0;

	//var returnable = "";

	// domain check
	// handle before start of string
	// pos1 arg is 1 based per mv/pick standard
	// treats anything below 1 as 1
	// pos variable is zero based standard c++ logic
	// pos cannot be < 0
//	if (static_cast<long>(pos) < 0)
//		pos = 0;

	// domain check
	// handle after end of string
	if (pos >= var_str.size()) {
		delimiterno = 0;
		return "";
		//return returnable;
	}

	// find the end of the field (or string)
	std::size_t end_pos;
//	end_pos = var_str.find_first_of(_RM _FM _VM _SM _TM _ST, pos);
	end_pos = find_first_fm(var_str, pos);

	// past of of string?
	if (end_pos == std::string::npos) {
		// wont work if string is the maximum string length but that cant occur
//		pos1 = static_cast<int>(var_str.size() + 2);
		pos1 = static_cast<int>(var_str.size() + 1);
		delimiterno = 0;
		return var_str.substr(pos, var_str.size() - pos);
		//returnable = (var_str.substr(pos, var_str.size() - pos));
		//return returnable;
	}

	// delimiters returned as numbers RM=1F=1 FM=1E=2, VM=1D=3 SM=1C=4 TM=1B=5 to ST=1A=6
	// delimiterno=int(LASTDELIMITERCHARNOPLUS1-var_str[end_pos]);
	delimiterno = static_cast<int>(*_RM) - static_cast<int>(var_str[end_pos]) + 1;

	// point AFTER the found separator or TWO after the length of the string (TODO shouldnt this
	// be one??/bug in pickos) wont work if string is the maximum string length but that cant
	// occur
	pos1 = static_cast<int>(end_pos + 2);

	// extract and return the substr as well
	return var_str.substr(pos, end_pos - pos);
	//returnable = (var_str.substr(pos, end_pos - pos));
	//return returnable;
}

//#if 0
//
////class var_brackets_proxy
//class PUBLIC var_brackets_proxy {
//   public:
//	var& var_;
//	int index_;
//
//	// Constructor from a var and an index
//	var_brackets_proxy(io var1, int index) : var_(var1), index_(index) {
//	}
//
//	// Implicit conversion to var
//	operator var() const {
//		return var_.at(index_);
//	}
//
////	// Implicit conversion to char
////	operator char() const {
////		return var_.at(index_).toChar();
////	}
//
//	// Operator assign
//	void operator=(const char char1) {
//		var_.paster(index_, 1, char1);
//	}
//
//	// Operator ==
//	template<class T>
//	bool operator==(T compare) {
//		return var_.at(index_) == compare;
//	}
//
//	// Operator !=
//	template<class T>
//	bool operator!=(T compare) {
//		return var_.at(index_) != compare;
//	}
//
//	// Operator ^
//	template<class T>
//	let operator^(T appendage) {
//		return var_.at(index_) ^ appendage;
//	}
//
//}
//
//// could be used if var_brackets_proxy holds a string perhaps for performance
//
////implicit conversion to var
//var_brackets_proxy::operator var() const {
//	//before first character?
//	if (index_ < 1) {
//		index_ += str_.size();
//		//overly negative - return ""
//		if (index_ < 1)
//			index_ = 1;
//	}
//	//off end - return ""
//	else if (static_cast<unsigned int>(index_) > str_.size())
//		return "";
//
//	//within range
//	return str_[index_ - 1];
//}
//
////operator assign a char
//void var_brackets_proxy::operator=(char char1) {
//	//before first character?
//	if (index_ < 1) {
//		index_ += str_.size();
//		//overly negative - return ""
//		if (index_ < 1)
//			index_ = 1;
//	}
//	//off end - return ""
//	else if (static_cast<unsigned int>(index_) > str_.size()) {
//		str_.push_back(char1);
//	} else {
//		str_[index_ - 1] = char1;
//	}
//
//	return;
//}
//#endif

//////
// SUM
//////
var  var::sumall() const {

	THISIS("var  var::sumall() const")
	assertString(function_sig);

	// Add up all numbers regardless of separators or levels (multilevel)
	// Limit the number of decimal places in returned value to the max found in the input
	// assert(sum("2245000900.76" _FM "102768099.9" _VM "-2347769000.66") == 0);

	var nrvo = 0;
	var start = 0;
	var subfield, term;
	std::size_t maxdecimals = 0;
	while (true) {
		//this extracts a substring up to any separator charactor ST-RM
		subfield = (*this).substr2(start, term);

		std::size_t subfieldsize = subfield.var_str.size();
		if (subfieldsize) {
			//for clarity of error message,
			//throw any error here instead of leaving it up to the +=
			if (!subfield.isnum())
				UNLIKELY
				throw VarNonNumeric("sumall() " ^ subfield.first(128).quote());

			nrvo += subfield;
			std::size_t n = subfield.var_str.find('.');
			if (n) {
				n = subfieldsize - n;
				if (n > maxdecimals)
					maxdecimals = n;
			}
		}
		if (not term)
			break;
	}
	return nrvo.round(static_cast<int>(maxdecimals));
}

var  var::sum() const {

	THISIS("var  var::sum() const")
	assertString(function_sig);

	// Limit the number of decimal places in returned value to the max found in the input
	// assert(sum("2245000900.76" _VM "102768099.9" _VM "-2347769000.66") == 0);

	var part;	  //num
	var nextsep;  //num
	var accum;	  //num

	let min_sep = ST.ord();  //26
	let max_sep = RM.ord();	  //31

	var min_sep_present;
	for (min_sep_present = min_sep; min_sep_present <= max_sep; ++min_sep_present) {
		if (this->index(this->chr(min_sep_present)))
			break;
	}
	if (min_sep_present > max_sep) {

		//for clarity of error message,
		//throw any error here instead of leaving it up to the +=
		if (!this->isnum())
			UNLIKELY
			throw VarNonNumeric("sum() " ^ this->first(128).quote());

		return (*this) + 0;
	}
	min_sep_present = 1 + max_sep - min_sep_present;
	var inpos = 1;
	var flag = 0;
	var nrvo = "";

	//std::clog << (*this) << std::endl;

	std::size_t maxndecimals = 0;  //initialise only to avoid warning
	do {

		//extract the next field and get the nextsep field number 1-6 or 0 if none
		part = this->substr2(inpos, nextsep);

		//for clarity of error message,
		//throw any error here instead of leaving it up to the +=
		if (!part.isnum())
			UNLIKELY
			throw VarNonNumeric("sum() " ^ part.first(128).quote());

		if (flag) {

			//accum = (accum + part).round(maxdecimals);
			accum += part;

			//record maximum decimal places on input
			std::size_t pos = part.var_str.find('.');
			if (pos != std::string::npos) {
				pos = part.var_str.size() - pos - 1;
				if (pos > maxndecimals)
					maxndecimals = pos;
			}

		} else {

			accum = part;

			//record maximum decimal places on input
			std::size_t pos = part.var_str.find('.');
			if (pos != std::string::npos) {
				maxndecimals = part.var_str.size() - pos - 1;
			} else
				maxndecimals = 0;
		}

		if (nextsep >= min_sep_present) {
			flag = 1;

		} else {

			ISSTRING(accum)
			if (not accum.var_str.empty()) {

				// Fix decimal places
				accum = accum.round(static_cast<int>(maxndecimals));

				// Check round returned a string
				ISSTRING(accum)

				// Remove trailing zeros if floating point is present
                if (accum.var_str.find('.') != std::string::npos) {

                    //remove trailing zeros
                    while (accum.var_str.back() == '0')
                        accum.var_str.pop_back();

                    //reinstate trailing zero after decimal point
                    if (accum.var_str.back() == '.')
                        accum.var_str.pop_back();
                }

				nrvo.var_str.append(accum.var_str);

			}

			if (nextsep) {
				nrvo ^= this->chr(1 + max_sep - nextsep);
			}

			flag = 0;
		}

	} while (nextsep);

	return nrvo;
}

var  var::sum(SV separator) const {

	THISIS("var  var::sum(SV separator) const")
	assertString(function_sig);

	var nrvo = 0;
	int nn = this->fcount(separator);

	// static var allseparators=_ST _TM _SM _VM _FM _RM;
	// var separatorn=allseparators.contains(separator);
	// if (!separatorn) return var1*var2;

	// TODO make this faster using remove or index?
	var temp;
	for (int ii = 1; ii <= nn; ++ii) {
		temp = (*this).field(separator, ii);
		if (temp.isnum())
			nrvo += temp;
		else
			//nrvo += (*this).sum(temp);
			nrvo += temp.sum();
	}
	return nrvo;	//NRVO hopefully since single named return
}

var  var::stddev() const {

	THISIS("var  var::stddev() const")
	assertString(function_sig);

	// Handle any field delimiters
	var strvar = this->convert(_VM _SM _TM _ST _RM, _FM _FM _FM _FM _FM);

	// n, tot, mean
	int n = strvar.fcount(_FM);
	if (not n)
		return "";
	double tot = 0;
	for (var v1 : strvar)
		tot += v1.toDouble();
	double mean = tot / n;

	// sumsqdiff, min, max
	double sumsqdiff = 0.0;
	var min = 1.0e308;
	var max = -1.0e307;
	for (var v1 : strvar) {
		double diff = v1 - mean;
		sumsqdiff += diff * diff;
		if (v1 < min)
			min = v1;
		if (v1 > max)
			max = v1;
	}

	var stddev = var(sumsqdiff / n).sqrt();

	var nrvo = n ^ FM ^ tot ^ FM ^ min ^ FM ^ max ^ FM ^ mean ^ FM ^ stddev;

	return nrvo;
}

}  // namespace exo
