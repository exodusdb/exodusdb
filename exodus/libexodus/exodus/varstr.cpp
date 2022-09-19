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

#include <string>

#include <algorithm>  //for dim::sort
#include <cstring>	  //for strlen strstr

#include <exodus/varimpl.h>

namespace exodus {

// includes dim::split

// and var::field,field2,locate,extract,remove,pickreplace,insert,substr,paste,remove

///////////////
// FIELD/FIELD2
///////////////

// FIELD2()
var var::field2(SV separator, const int fieldno, const int nfields) const {
	if (fieldno >= 0)
		return field(separator, fieldno, nfields);

	return field(separator, count(separator) + 1 + fieldno + 1, nfields);
}

// FIELD()
// var.field(separator,fieldno,nfields)
var var::field(SV separatorx, const int fieldnx, const int nfieldsx) const {

	THISIS("var var::field(SV separatorx,const int fieldnx,const int nfieldsx) const")
	assertString(function_sig);

	if (separatorx.empty()) {
		//return "";
		throw VarError("separator cannot be blank in field()");
	}

	int fieldno = fieldnx > 0 ? fieldnx : 1;
	int nfields = nfieldsx > 0 ? nfieldsx : 1;

	// separator might be multi-byte ... esp. for non-ASCII
	std::string::size_type len_separator = separatorx.size();

	// FIND FIELD

	// find the starting position of the field or return ""
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		start_pos = var_str.find(separatorx, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return "";
		// start_pos++;
		start_pos += len_separator;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type end_pos = start_pos;
	int pastfieldn = fieldno + nfields;
	while (fieldn2 < pastfieldn) {
		end_pos = var_str.find(separatorx, end_pos);
		// past of of string?
		if (end_pos == std::string::npos) {
			//return var_str.substr(start_pos, var_str.size() - start_pos);
			return var_str.substr(start_pos);
		}
		// end_pos++;
		end_pos += len_separator;
		fieldn2++;
	}
	// backup to first character if closing separator in case multi-byte separator
	end_pos -= (len_separator - 1);

	return var_str.substr(start_pos, end_pos - start_pos - 1);
}

/////////////
// FIELDSTORE
/////////////

// var.fieldstore(separator,fieldno,nfields,replacement)
var var::fieldstore(SV separator, const int fieldnx, const int nfieldsx, CVR replacementx) const& {
	return var(*this).fieldstorer(separator, fieldnx, nfieldsx, replacementx);
}

// in-place
VARREF var::fieldstorer(SV separator, const int fieldnx, const int nfieldsx, CVR replacementx) {

	THISIS(
		"VARREF var::fieldstorer(SV separator0,const int fieldnx,const int nfieldsx, "
		"CVR replacementx)")
	assertStringMutator(function_sig);

	if (separator.empty()) {
		//// *this = "";
		//var_str.clear();
		//var_typ = VARTYP_STR;
		//return *this;
		throw VarError("separator cannot be blank in fieldstorer()");
	}

	// handle multibyte/non-ASCII separators
	std::string::size_type separator_len = separator.size();

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
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		start_pos = var_str.find(separator, start_pos);
		// past of of string?
		if (start_pos == std::string::npos) {
			do {
				var_str += separator;
				fieldn2++;
			} while (fieldn2 < fieldno);
			var_str += replacement.var_str;
			return *this;
		}
		// start_pos++;
		start_pos += separator_len;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type end_pos = start_pos;
	int pastfieldn = fieldno + nfields;
	while (fieldn2 < pastfieldn) {
		end_pos = var_str.find(separator, end_pos);
		// past of of string?
		if (end_pos == std::string::npos) {
			var_str.replace(start_pos, std::string::npos, replacement.var_str);
			return *this;
		}
		// end_pos++;
		end_pos += separator_len;
		fieldn2++;
	}

	// backup to first byte of end of field sep
	end_pos -= (separator_len - 1);

	// insert or replace
	if (end_pos == start_pos) {
		//insert
		if (nfields == 0)
			replacement.var_str += separator;
		var_str.insert(start_pos, replacement.var_str);
	} else {
		var_str.replace(start_pos, end_pos - start_pos - 1, replacement.var_str);
	}

	return *this;
}

/////////
// LOCATE
/////////

// hardcore string locate function given a section of a string and all parameters
inline bool locateat(const std::string& var_str, const std::string& target, size_t start_pos, size_t end_pos, const char order, SV usingchar, VARREF setting) {
	// private - assume everything is defined/assigned correctly

	//
	// if (target.size()==0&&start_pos==end_pos)
	// if (target.size()==0)
	//{
	//	setting=1;
	//	return true;
	//}

	// pickos strangeness? locate even if out of order
	// if this is the pickos standard and needs to be implemented properly it should be
	// implemented as continue to search to end instead of search twice like this this will
	// probably be switched off as unnecessary and slow behaviour for EXODUS applications
	if (order) {
		// THISIS(...)
		// ISSTRING(usingchar)
		//bool result = locateat(var_str, target, start_pos, end_pos, 0, usingchar, setting);
		//if (result)
		//	return result;
		if (locateat(var_str, target, start_pos, end_pos, 0, usingchar, setting))
			return true;
	}

	// find null in a null field
	if (start_pos >= end_pos) {
		setting = 1;
		return !target.size();
	}

	size_t usingchar_len = usingchar.size();

	// for use in AR/DR;
	var value;
	var target2;
	target2 = target;

	// find the starting position of the value or return ""
	// using start_pos and end_pos of
	int targetlen = static_cast<int>((target.size()));
	int valuen2 = 1;
	do {

		size_t nextstart_pos = var_str.find(usingchar, start_pos);
		// past end of string?
		// if (nextstart_pos==string::npos)
		//{
		//	setting=valuen2+1;
		//	return false;
		//}
		// nextstart_pos++;
		// past end of field?
		int comp;
		if (nextstart_pos >= end_pos) {
			nextstart_pos = end_pos;
			switch (order) {
				// No order
				case '\x00':
					if (var_str.substr(start_pos, end_pos - start_pos) == target) {
						setting = valuen2;
						return true;
					}
					setting = valuen2 + 1;
					return false;
					//break;

				// AL Ascending Left Justified
				case '\x01':
					//if (var_str.substr(start_pos, nextstart_pos - start_pos) >= target)
					comp = var::localeAwareCompare(var_str.substr(start_pos, nextstart_pos - start_pos), target);
					if (comp == 1) {
						setting = valuen2;
						//if (var_str.substr(start_pos, nextstart_pos - start_pos) ==
						//    target)
						if (comp == 0)
							return true;
						else
							// pickos error strangeness? empty is not greater than
							// any target except empty
							//						if
							//(start_pos==end_pos) setting+=1;
							return false;
					}
					setting = valuen2 + 1;
					return false;
					//break;

				// AR Ascending Right Justified
				case '\x02':
					value = var_str.substr(start_pos, nextstart_pos - start_pos);
					if (value >= target2) {
						if (value == target2) {
							setting = valuen2;
							//setting = valuen2 + (value.size() < target2.size());
							return true;
						} else {
							setting = valuen2;
							return false;
						}
					}
					setting = valuen2 + 1;
					return false;
					//break;

				// DL Descending Left Justified
				case '\x03':
					if (var_str.substr(start_pos, nextstart_pos - start_pos) <= target) {
						setting = valuen2;
						if (var_str.substr(start_pos, nextstart_pos - start_pos) ==
							target)
							return true;
						else
							return false;
					}
					setting = valuen2 + 1;
					return false;
					//break;

				// DR Descending Right Justified
				case '\x04':
					value = var_str.substr(start_pos, nextstart_pos - start_pos);
					if (value <= target2) {
						setting = valuen2;
						if (value == target2)
							return true;
						else
							return false;
					}
					setting = valuen2 + 1;
					return false;
					//break;

				default:
					throw VarError("locateat() invalid mode " ^ var(order));
			}
		}

		// if (var_str.substr(start_pos,nextstart_pos-start_pos)==target)
		// should only test for target up to next sep
		// but pickos (by accidental error?) at least checks for the whole target
		// even if the target contains the sep character
		// if (var_str.substr(start_pos,nextstart_pos-start_pos)==target)
		//static_cast<int> is to avoid warning of unsigned integer
		switch (order) {
			// No order
			case '\x00':
				if (var_str.substr(start_pos, targetlen) == target) {
					bool x = static_cast<int>(nextstart_pos - start_pos) <= targetlen;
					if (x) {
						setting = valuen2;
						return true;
					}
				}
				break;

			// AL Ascending Left Justified
			case '\x01':
				//				//pickos strangeness? to locate a field whereever
				//it is regardless of order even ""? if
				// (!targetlen&&nextstart_pos==start_pos) break;

				//if (var_str.substr(start_pos, nextstart_pos - start_pos) >= target)
				comp = var::localeAwareCompare(var_str.substr(start_pos, nextstart_pos - start_pos), target);
				if (comp == 1) {
					setting = valuen2;
					//if (var_str.substr(start_pos, nextstart_pos - start_pos) == target)
					if (comp == 0)
						return true;
					else
						return false;
				}
				break;

			// DL Descending Left Justified
			case '\x02':
				//				//pickos strangeness? to locate a field whereever
				//it is regardless of order even ""? if
				// (!targetlen&&nextstart_pos==start_pos) break;

				value = var_str.substr(start_pos, nextstart_pos - start_pos);
				if (value >= target) {
					//if (value >= target2) {
					setting = valuen2;
					if (value == target)
						//if (value == target2)
						return true;
					else
						return false;
				}
				break;

			// AR Ascending Right Justified
			case '\x03':
				//				//pickos strangeness? to locate a field whereever
				//it is regardless of order even ""? if
				// (!targetlen&&nextstart_pos==start_pos) break;

				if (var_str.substr(start_pos, nextstart_pos - start_pos) <= target) {
					setting = valuen2;
					if (var_str.substr(start_pos, nextstart_pos - start_pos) == target)
						return true;
					else
						return false;
				}

				/*
				value = var_str.substr(start_pos, nextstart_pos - start_pos);
				if (value <= target2) {
					if (value == target2) {
						//setting = valuen2;
						setting = valuen2;//(value.size() < target2.size());
						return true;
					}
					else {
						setting = valuen2;
						return false;
					}
				}
				*/
				break;

			// DR Descending Right Justified
			case '\x04':
				//				//pickos strangeness? to locate a field whereever
				//it is regardless of order even ""? if
				// (!targetlen&&nextstart_pos==start_pos) break;

				value = var_str.substr(start_pos, nextstart_pos - start_pos);
				if (value <= target2) {
					setting = valuen2;
					if (value == target2)
						return true;
					else
						return false;
				}
				break;

			default:
				throw VarError("locateat() invalid order" ^ var(order));
		}
		// skip over any sep character
		start_pos = nextstart_pos + usingchar_len;
		valuen2++;
	} while (true);
}

// locate within extraction
inline bool locatex(const std::string& var_str, const std::string& target, const char* ordercode, SV usingchar, VARREF setting, int fieldno, int valueno, const int subvalueno) {
	// private - assume everything is defined/assigned correctly

	// any negatives at all returns ""
	// done inline since unusual
	// if (fieldno<0||valueno<0||subvalueno<0) return ""

	char ordermode;
	if (strlen(ordercode) == 0)
		ordermode = 0;
	else {
		// locate the order code in a list of the four possible order codes
		// and throw if not found
		const char* ordercodes = "ALARDLDR";
		const char* orderindex = strstr(ordercodes, ordercode);
		if (orderindex == nullptr)
			throw VarError("locateby('" ^ var(ordercode) ^ "') is invalid");

		// convert the memory address to the char position within the codes
		ordermode = static_cast<int>(orderindex - ordercodes);
		// add two and divide by two to get the order no AL=1 AR=2 DL=3 DR=4
		ordermode = (ordermode + 2) >> 1;
	}

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
			return locateat(var_str, target, (size_t)0, var_str.size(), ordermode,
							usingchar, setting);
		}
	}

	// find the starting position of the field or return ""
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		start_pos = var_str.find(FM_, start_pos);
		// past of of string?
		if (start_pos == std::string::npos) {
			// if (valueno||subvalueno) setting=1;
			// else setting=fieldn2+1;
			setting = 1;
			return !target.size();
		}
		start_pos++;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type field_end_pos;
	field_end_pos = var_str.find(FM_, start_pos);
	if (field_end_pos == std::string::npos)
		field_end_pos = var_str.size();

	// FIND VALUE

	if (start_pos >= field_end_pos) {
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
			return locateat(var_str, target, start_pos, field_end_pos, ordermode,
							usingchar, setting);
	}

	// find the starting position of the value or return ""
	// using start_pos and end_pos of
	int valuen2 = 1;
	while (valuen2 < valueno) {
		start_pos = var_str.find(VM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos) {
			// if (subvalueno) setting=1;
			// else setting=valuen2+1;
			setting = 1;
			return !target.size();
		}
		start_pos++;
		// past end of field?
		if (start_pos > field_end_pos) {
			// setting=valuen2+1;
			setting = 1;
			return !target.size();
		}
		valuen2++;
	}

	// find the end of the value (or string)
	std::string::size_type value_end_pos;
	value_end_pos = var_str.find(VM_, start_pos);
	if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
		value_end_pos = field_end_pos;

	// FIND SUBVALUE

	if (start_pos >= value_end_pos) {
		setting = 1;
		return !target.size();
	}

	if (subvalueno <= 0) {

		// zero means all
		if (subvalueno == 0)
			return locateat(var_str, target, start_pos, value_end_pos, ordermode, usingchar,
							setting);

		// negative means ""
		else {
			setting = 1;
			return !target.size();
		}
	}

	// find the starting position of the subvalue or return ""
	// using start_pos and end_pos of
	int subvaluen2 = 1;
	while (subvaluen2 < subvalueno) {
		start_pos = var_str.find(SM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos) {
			// setting=subvaluen2+1;
			setting = 1;
			return !target.size();
		}
		start_pos++;
		// past end of value?
		if (start_pos > value_end_pos) {
			// setting=subvaluen2+1;
			setting = 1;
			return !target.size();
		}
		subvaluen2++;
	}

	// find the end of the subvalue (or string)
	std::string::size_type subvalue_end_pos;
	subvalue_end_pos = var_str.find(SM_, start_pos);
	if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos) {
		return locateat(var_str, target, start_pos, value_end_pos, ordermode, usingchar,
						setting);
	}

	return locateat(var_str, target, start_pos, subvalue_end_pos, ordermode, usingchar,
					setting);
}

// default locate using VM
bool var::locate(CVR target, VARREF setting) const {

	THISIS(
		"bool var::locate(CVR target, VARREF setting, const int fieldno/*=0*/,const "
		"int valueno/*=0*/) const")
	assertString(function_sig);
	ISSTRING(target)
	ISDEFINED(setting)

	return locatex(var_str, target.var_str, "", _VM, setting, 0, 0, 0);
}

bool var::locate(CVR target, VARREF setting, const int fieldno, const int valueno /*=0*/) const {

	THISIS(
		"bool var::locate(CVR target, VARREF setting, const int fieldno/*=0*/,const "
		"int valueno/*=0*/) const")
	assertString(function_sig);
	ISSTRING(target)
	ISDEFINED(setting)

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

// without setting
bool var::locate(CVR target) const {

	THISIS("bool var::locate(CVR target")
	assertString(function_sig);
	ISSTRING(target)

	var setting;
	return locatex(var_str, target.var_str, "", _VM, setting, 0, 0, 0);
}

////////////
// LOCATE BY
////////////

//// 1. rare syntax where the order is given as a variable
//bool var::locateby(CVR ordercode, CVR target, VARREF setting, const int fieldno, const int valueno /*=0*/) const {
//	return locateby(ordercode.toString().c_str(), target, setting, fieldno, valueno);
//}

//// 2. no fieldno or valueno means locate using character VM
//// caters for the rare syntax where the order is given as a variable
//bool var::locateby(CVR ordercode, CVR target, VARREF setting) const {
//	return locateby(ordercode.toString().c_str(), target, setting);
//}

// 3. no fieldno or valueno means locate using character VM
// specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locateby(const char* ordercode, CVR target, VARREF setting) const {

	THISIS("bool var::locateby(const char* ordercode, CVR target, VARREF setting) const")
	assertString(function_sig);
	ISSTRING(target)
	ISDEFINED(setting)

	// TODO either make a "locatefrom" version of the above where the locate STARTS its search
	// from the last numbered subvalue (add a new parameter), value or field. OR possibly modify
	// this function to understand a negative number as "start from" instead of "within this"

	// determine locate by field, value or subvalue depending on the parameters as follows:
	// if value number is given then locate in subvalues of that value
	// if field number is given then locate in values of that field
	// otherwise locate in fields of the string

	return locatex(var_str, target.var_str, ordercode, _VM, setting, 0, 0, 0);
}

// 4. specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locateby(const char* ordercode, CVR target, VARREF setting, const int fieldno, const int valueno /*=0*/) const {

	THISIS(
		"bool var::locateby(const char* ordercode, CVR target, VARREF setting, const "
		"int fieldno, const int valueno/*=0*/) const")
	assertString(function_sig);
	ISSTRING(target)
	ISDEFINED(setting)

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

bool var::locatebyusing(const char* ordercode, const char* usingchar, CVR target, VARREF setting, const int fieldno /*=0*/, const int valueno /*=0*/, const int subvalueno /*=0*/) const {

	THISIS(
		"bool var::locatebyusing(const char* ordercode, const char* usingchar, CVR "
		"target, VARREF setting, const int fieldno=0, const int valueno=0, const int valueno=0) const")
	assertString(function_sig);
	ISSTRING(target)
	ISDEFINED(setting)

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
bool var::locateusing(const char* usingchar, CVR target) const {

	THISIS("bool var::locateusing(const char* usingchar, CVR target) const")
	assertString(function_sig);
	ISSTRING(target)

	var setting = "";
	return locatex(var_str, target.var_str, "", usingchar, setting, 0, 0, 0);
}

// 2. specify field/value/subvalue and return position
bool var::locateusing(const char* usingchar, CVR target, VARREF setting, const int fieldno /*=0*/, const int valueno /*=0*/, const int subvalueno /*=0*/) const {

	THISIS(
		"bool var::locateusing(const char* usingchar, CVR target, VARREF setting, const "
		"int fieldno/*=0*/, const int valueno/*=0*/, const int subvalueno/*=0*/) const")
	assertString(function_sig);
	ISSTRING(target)
	ISDEFINED(setting)

	return locatex(var_str, target.var_str, "", usingchar, setting, fieldno, valueno,
				   subvalueno);
}

//////////
// EXTRACT
//////////

// const var(fn,vn,sn) extract var
//
//var var::operator()(int fieldno, int valueno/*=0*/, int subvalueno/*=0*/) const {
//	return f(fieldno, valueno, subvalueno);
//}

// non-const xxxx(fn,vn,sn) returns a proxy that can be aasigned to or implicitly converted to a var
//
var_proxy1 var::operator()(int fieldno) {
	return var_proxy1(this, fieldno);
}
var_proxy2 var::operator()(int fieldno, int valueno) {
	return var_proxy2(this, fieldno, valueno);
}
var_proxy3 var::operator()(int fieldno, int valueno, int subvalueno) {
	return var_proxy3(this, fieldno, valueno, subvalueno);
}

// Old "extract()" function in either procedural or OO style.
//     xxx = extract(yyy, 1, 2, 3)
//  or xxx = yyy.extract(1, 2, 3)
//
var var::extract(const int argfieldn, const int argvaluen/*=0*/, const int argsubvaluen/*=0*/) const {
	return this->f(argfieldn, argvaluen, argsubvaluen);
}

// Abbreviated xxxx.f(1,2,3) syntax. PickOS angle bracket syntax (xxx<1,2,3>) not possible in C++
//     xxx = yyy.f(1,2,3)
//
var var::f(const int argfieldn, const int argvaluen/*=0*/, const int argsubvaluen/*=0*/) const {

	THISIS(
		"var var::f(const int argfieldn, const int argvaluen, const int argsubvaluen) "
		"const")
	assertString(function_sig);

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
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno) {
		start_pos = var_str.find(FM_, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return "";
		start_pos++;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type field_end_pos;
	field_end_pos = var_str.find(FM_, start_pos);
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
			return var_str.substr(start_pos, field_end_pos - start_pos);
	}

	// unless extracting value 1,
	// find the starting position of the value or return ""
	// using start_pos and end_pos of
	int valuen2 = 1;
	while (valuen2 < valueno) {
		start_pos = var_str.find(VM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos)
			return "";
		start_pos++;
		// past end of field?
		if (start_pos > field_end_pos)
			return "";
		valuen2++;
	}

	// find the end of the value (or string)
	std::string::size_type value_end_pos;
	value_end_pos = var_str.find(VM_, start_pos);
	if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
		value_end_pos = field_end_pos;

	// FIND SUBVALUE

	// zero means all, negative means ""
	if (subvalueno == 0)
		return var_str.substr(start_pos, value_end_pos - start_pos);
	if (subvalueno < 0)
		return "";

	// unless extracting subvalue 1,
	// find the starting position of the subvalue or return ""
	// using start_pos and end_pos of
	int subvaluen2 = 1;
	while (subvaluen2 < subvalueno) {
		start_pos = var_str.find(SM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos)
			return "";
		start_pos++;
		// past end of value?
		if (start_pos > value_end_pos)
			return "";
		subvaluen2++;
	}

	// find the end of the subvalue (or string)
	std::string::size_type subvalue_end_pos;
	subvalue_end_pos = var_str.find(SM_, start_pos);
	if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
		return var_str.substr(start_pos, value_end_pos - start_pos);

	return var_str.substr(start_pos, subvalue_end_pos - start_pos);
}

//////////////////////////////
// REMOVE was PickOS "DELETE()"
//////////////////////////////

// 1. return a temporary
var var::remove(const int fieldno, const int valueno, const int subvalueno) const {
	return var(*this).remover(fieldno, valueno, subvalueno);
}

// 2. remove in place
VARREF var::remover(int fieldno, int valueno, int subvalueno) {

	THISIS("VARREF var::remover(int fieldno,int valueno,int subvalueno)")
	assertStringMutator(function_sig);

	// return "" if replacing 0,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0) {
		// functionmode return "";//var(var1);
		// proceduremode
		var_str.clear();
		var_typ = VARTYP_STR;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::string::size_type start_pos = 0;
	std::string::size_type field_end_pos;

	// negative means remove nothing
	if (fieldno < 0) {
		return *this;

	} else {

		// find the starting position of the field or quit
		int fieldn2 = 1;
		while (fieldn2 < fieldno) {
			start_pos = var_str.find(FM_, start_pos);
			// past of of string?
			if (start_pos == std::string::npos) {
				return *this;
			}
			start_pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		field_end_pos = var_str.find(FM_, start_pos);
		if (field_end_pos == std::string::npos)
			field_end_pos = var_str.size();
	}

	////////////// FIND VALUE ///////////////////
	std::string::size_type value_end_pos;

	// zero means remove all values, negative means remove nothing
	if (valueno < 0) {
		return *this;

	} else if (valueno == 0 && subvalueno == 0) {
		if (fieldno > 1)
			start_pos--;
		else if (field_end_pos < var_str.size())
			field_end_pos++;
		var_str.erase(start_pos, field_end_pos - start_pos);
		return *this;

	} else {

		// find the starting position of the value or quit
		int valuen2 = 1;
		while (valuen2 < valueno) {
			start_pos = var_str.find(VM_, start_pos);
			// past end of string or field?
			if (start_pos >= field_end_pos || start_pos == std::string::npos) {
				return *this;
			}
			start_pos++;
			valuen2++;
		}

		// find the end of the value (or string)
		value_end_pos = var_str.find(VM_, start_pos);
		if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
			value_end_pos = field_end_pos;
	}

	////////// FIND SUBVALUE  //////////////////////
	std::string::size_type subvalue_end_pos;

	// zero means remove all subvalues, negative means remove nothing
	if (subvalueno < 0) {
		return *this;

	} else if (subvalueno == 0) {
		if (valueno > 1)
			start_pos--;
		else if (value_end_pos < field_end_pos)
			value_end_pos++;
		var_str.erase(start_pos, value_end_pos - start_pos);
		return *this;

	} else {
		// find the starting position of the subvalue or quit
		int subvaluen2 = 1;
		while (subvaluen2 < subvalueno) {
			start_pos = var_str.find(SM_, start_pos);
			// past end of string or value
			if (start_pos >= value_end_pos || start_pos == std::string::npos) {
				return *this;
			}
			start_pos++;
			subvaluen2++;
		}

		// find the end of the subvalue (or string)
		subvalue_end_pos = var_str.find(SM_, start_pos);
		if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
			subvalue_end_pos = value_end_pos;
	}
	// wcout<<start_pos<<" "<<subvalue_end_pos<<" "<<subvalue_end_pos-start_pos<<endl;

	if (subvalueno > 1)
		start_pos--;
	else if (subvalue_end_pos < value_end_pos)
		subvalue_end_pos++;
	var_str.erase(start_pos, subvalue_end_pos - start_pos);

	return *this;
}

///////////////////////////////////////////
// PICKREPLACE int int int var
///////////////////////////////////////////

var var::pickreplace(const int fieldno, const int valueno, const int subvalueno, CVR replacement) const {
	return var(*this).r(fieldno, valueno, subvalueno, replacement);
}

var var::pickreplace(const int fieldno, const int valueno, CVR replacement) const {
	return var(*this).r(fieldno, valueno, 0, replacement);
}

var var::pickreplace(const int fieldno, CVR replacement) const {
	return var(*this).r(fieldno, 0, 0, replacement);
}

VARREF var::r(const int fieldno, const int valueno, CVR replacement) {
	return r(fieldno, valueno, 0, replacement);
}

VARREF var::r(const int fieldno, CVR replacement) {
	return r(fieldno, 0, 0, replacement);
}

VARREF var::r(int fieldno, int valueno, int subvalueno, CVR replacement) {

	THISIS("VARREF var::r(int fieldno,int valueno,int subvalueno,CVR replacement)")
	assertStringMutator(function_sig);
	ISSTRING(replacement)

	// return whole thing if replace 0,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0) {
		// functionmode return var(replacement);
		// proceduremode
		var_str = replacement.var_str;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::string::size_type start_pos = 0;
	std::string::size_type field_end_pos;

	// negative means append
	if (fieldno < 0) {
		// append a FM_ only if there is existing data
		if (not var_str.empty())
			//var_str += FM_;
			var_str.push_back(FM_);
		start_pos = var_str.size();
		field_end_pos = start_pos;
	} else {

		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno) {
			start_pos = var_str.find(FM_, start_pos);
			// past of of string?
			if (start_pos == std::string::npos) {
				var_str.append(fieldno - fieldn2, FM_);
				// start_pos=var_str.size();
				// fieldn2=fieldno
				break;
			}
			start_pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		if (start_pos == std::string::npos) {
			start_pos = var_str.size();
			field_end_pos = start_pos;
		} else {
			field_end_pos = var_str.find(FM_, start_pos);
			if (field_end_pos == std::string::npos)
				field_end_pos = var_str.size();
		}
	}

	////////////// FIND VALUE ///////////////////
	std::string::size_type value_end_pos;

	// zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno < 0) {
		if (field_end_pos - start_pos > 0) {
			var_str.insert(field_end_pos, _VM);
			field_end_pos++;
		}
		start_pos = field_end_pos;
		value_end_pos = field_end_pos;
	} else if (valueno == 0 && subvalueno == 0) {
		var_str.replace(start_pos, field_end_pos - start_pos, replacement.var_str);
		return *this;
	} else {

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		while (valuen2 < valueno) {
			start_pos = var_str.find(VM_, start_pos);
			// past end of string or field?
			if (start_pos >= field_end_pos || start_pos == std::string::npos) {
				// start_pos = field_end_pos;
				// var_str.insert(field_end_pos,std::string(valueno-valuen2,VM_));
				var_str.insert(field_end_pos, valueno - valuen2, VM_);
				field_end_pos += valueno - valuen2;
				start_pos = field_end_pos;
				break;
			}
			start_pos++;
			valuen2++;
		}

		// find the end of the value (or string)
		value_end_pos = var_str.find(VM_, start_pos);
		if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
			value_end_pos = field_end_pos;
	}

	////////// FIND SUBVALUE  //////////////////////
	std::string::size_type subvalue_end_pos;

	// zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno < 0) {
		if (value_end_pos - start_pos > 0) {
			var_str.insert(value_end_pos, _SM);
			value_end_pos++;
		}
		start_pos = value_end_pos;
		subvalue_end_pos = value_end_pos;
	} else if (subvalueno == 0) {
		var_str.replace(start_pos, value_end_pos - start_pos, replacement.var_str);
		return *this;
	} else {
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		while (subvaluen2 < subvalueno) {
			start_pos = var_str.find(SM_, start_pos);
			// past end of string or value
			if (start_pos >= value_end_pos || start_pos == std::string::npos) {
				// start_pos = value_end_pos;
				// var_str.insert(value_end_pos,std::string(subvalueno-subvaluen2,SM_));
				var_str.insert(value_end_pos, subvalueno - subvaluen2, SM_);
				value_end_pos += subvalueno - subvaluen2;
				start_pos = value_end_pos;
				break;
			}
			start_pos++;
			subvaluen2++;
		}

		// find the end of the subvalue (or string)
		subvalue_end_pos = var_str.find(SM_, start_pos);
		if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
			subvalue_end_pos = value_end_pos;
	}
	// wcout<<start_pos<<" "<<subvalue_end_pos<<" "<<subvalue_end_pos-start_pos<<endl;

	var_str.replace(start_pos, subvalue_end_pos - start_pos, replacement.var_str);

	return *this;
}

///////////////////////////////////////////
// INSERT int int int var
///////////////////////////////////////////

var var::insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) const& {
	return var(*this).inserter(fieldno, valueno, subvalueno, insertion);
}

// given only field and value numbers
var var::insert(const int fieldno, const int valueno, CVR insertion) const& {
	return this->insert(fieldno, valueno, 0, insertion);
}

// given only field number
var var::insert(const int fieldno, CVR insertion) const& {
	return this->insert(fieldno, 0, 0, insertion);
}

// on temporary
VARREF var::insert(const int fieldno, const int valueno, const int subvalueno, CVR insertion) && {
	return this->inserter(fieldno, valueno, subvalueno, insertion);
}

// on temporary - given only field and value number
VARREF var::insert(const int fieldno, const int valueno, CVR insertion) && {
	return this->inserter(fieldno, valueno, 0, insertion);
}

// on temporary - given only field number
VARREF var::insert(const int fieldno, CVR insertion) && {
	return this->inserter(fieldno, 0, 0, insertion);
}

// in-place - given field and value no
VARREF var::inserter(const int fieldno, const int valueno, CVR insertion) {
	return this->inserter(fieldno, valueno, 0, insertion);
}

// in-place given only field no
VARREF var::inserter(const int fieldno, CVR insertion) {
	return this->inserter(fieldno, 0, 0, insertion);
}

//in-place - given everything
VARREF var::inserter(const int fieldno, const int valueno, const int subvalueno, CVR insertion) {

	THISIS(
		"VARREF var::inserter(const int fieldno,const int valueno,const int subvalueno,const "
		"VARREF insertion)")
	assertStringMutator(function_sig);
	ISSTRING(insertion)

	// 0,0,0 is like 1,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0) {
		//if (var_str.size())
		if (not var_str.empty())
			var_str.insert(0, insertion.var_str + FM_);
		else
			var_str = insertion.var_str;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::string::size_type start_pos = 0;
	std::string::size_type field_end_pos;

	int pad = false;

	// negative means append
	if (fieldno < 0) {
		// append a FM_ only if there is existing data
		if (not var_str.empty())
			//var_str += FM_;
			var_str.push_back(FM_);
		pad = true;
		start_pos = var_str.size();
		field_end_pos = start_pos;
	} else {
		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno) {
			start_pos = var_str.find(FM_, start_pos);
			// past of of string?
			if (start_pos == std::string::npos) {
				pad = true;
				var_str.append(fieldno - fieldn2, FM_);
				// start_pos=var_str.size();
				// fieldn2=fieldno
				break;
			}
			start_pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		if (start_pos == std::string::npos) {
			start_pos = var_str.size();
			field_end_pos = start_pos;
		} else {
			field_end_pos = var_str.find(FM_, start_pos);
			if (field_end_pos == std::string::npos)
				field_end_pos = var_str.size();
		}
	}

	////////////// FIND VALUE ///////////////////
	std::string::size_type value_end_pos;

	// zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno < 0) {
		if (field_end_pos - start_pos > 0) {
			var_str.insert(field_end_pos, _VM);
			field_end_pos++;
		}
		start_pos = field_end_pos;
		value_end_pos = field_end_pos;
		pad = true;
	} else if (valueno == 0 && subvalueno == 0) {
		if (!pad && !var_str.empty())
			var_str.insert(start_pos, _FM);
		var_str.insert(start_pos, insertion.var_str);
		return *this;
	} else {

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		while (valuen2 < valueno) {
			start_pos = var_str.find(VM_, start_pos);
			// past end of string or field?
			if (start_pos >= field_end_pos || start_pos == std::string::npos) {
				pad = true;
				// start_pos = field_end_pos;
				// var_str.insert(field_end_pos,std::string(valueno-valuen2,VM_));
				var_str.insert(field_end_pos, valueno - valuen2, VM_);
				field_end_pos += valueno - valuen2;
				start_pos = field_end_pos;
				break;
			}
			start_pos++;
			valuen2++;
		}

		// find the end of the value (or string)
		value_end_pos = var_str.find(VM_, start_pos);
		if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
			value_end_pos = field_end_pos;
	}

	////////// FIND SUBVALUE  //////////////////////
	//std::string::size_type subvalue_end_pos;

	// zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno < 0) {
		if (value_end_pos - start_pos > 0) {
			var_str.insert(value_end_pos, _SM);
			value_end_pos++;
		}
		start_pos = value_end_pos;
		//not required
		//subvalue_end_pos = value_end_pos;
		pad = true;
	} else if (subvalueno == 0) {
		if (!pad && (start_pos < field_end_pos || valueno > 1))
			var_str.insert(start_pos, _VM);
		var_str.insert(start_pos, insertion.var_str);
		return *this;
	} else {
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		while (subvaluen2 < subvalueno) {
			start_pos = var_str.find(SM_, start_pos);
			// past end of string or value
			if (start_pos >= value_end_pos || start_pos == std::string::npos) {
				pad = true;
				// start_pos = value_end_pos;
				// var_str.insert(value_end_pos,std::string(subvalueno-subvaluen2,SM_));
				var_str.insert(value_end_pos, subvalueno - subvaluen2, SM_);
				value_end_pos += subvalueno - subvaluen2;
				start_pos = value_end_pos;
				break;
			}
			start_pos++;
			subvaluen2++;
		}

		//not required
		// find the end of the subvalue (or string)
		//subvalue_end_pos = var_str.find(SM_, start_pos);
		//if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
		//	subvalue_end_pos = value_end_pos;
	}

	if (!pad && (start_pos < value_end_pos || subvalueno > 1))
		var_str.insert(start_pos, _SM);
	var_str.insert(start_pos, insertion.var_str);

	return *this;
}

///////////////
//STARTS / ENDS
///////////////

//starting is equivalent to x::index(y) == 1
//contains is equivalent to x::index(y) != 0

bool var::starts(SV str) const {

	THISIS("bool var::starts(SV str) const")
	//THISIS(__PRETTY_FUNCTION__)
	//      bool exodus::var::starts(const exodus::VARREF) const
	assertString(function_sig);

	// Differ from c++
	if (str.empty()) {
		VarError e(__PRETTY_FUNCTION__);
		e.description.errput();
		e.stack(1).f(1).errputl();
		return false;
	}

	return var_str.starts_with(str);
}

bool var::ends(SV str) const {

	THISIS("bool var::ends(SV str) const")
	assertString(function_sig);

	// Differ from c++
	if (str.empty()) {
		VarError e(__PRETTY_FUNCTION__);
		e.description.errput();
		e.stack(1).f(1).errputl();
		return false;
	}

	return var_str.ends_with(str);
}

bool var::contains(SV str) const {

	THISIS("bool var::contains(SV str) const")
	assertString(function_sig);

	// Differ from c++
	if (str.empty()) {
		VarError e(__PRETTY_FUNCTION__);
		e.description.errput();
		//e.stack().f(1).errputl(", ");
		e.stack(1).f(1).errputl(", ");
		return false;
	}

	return var_str.find(str) != std::string::npos;
	//C++23 return var_str.contains(vstr.var_str);
}

////////
// FIRST
////////

var var::first(const size_t  length) const& {

	THISIS("var var::first(const size_t length)")
	assertString(function_sig);

	// Assume high half of size_t is c++ unblockable conversion
	// of negative ints to size_t. Runtime error
	if (length > std::string::npos >> 1)
		throwNonPositive(__PRETTY_FUNCTION__);

	// Construct a new var with the required number of chars from this or all
	var rvo(this->var_str.data(), std::min(length, this->var_str.size()));

	return rvo;
}
//__cpp_lib_string_contains


//[1,y]
// var.s(1,length) substring
VARREF var::firster(const size_t length) {

	THISIS("VARREF var::firster(const size_t length)")
	assertStringMutator(function_sig);

	// Assume high half of size_t is c++ unblockable conversion
	// of negative ints to size_t. Runtime error
	if (length > std::string::npos >> 1)
		throwNonPositive(__PRETTY_FUNCTION__);

	// Reduce the size of this string if necessary
	if (length < this->var_str.size()) {
		this->var_str.resize(length);
	}

	return *this;
}

///////
// LAST
///////

var var::last(const size_t  length) const& {

	THISIS("var var::last(const size_t length)")
	assertString(function_sig);

	// Assume high half of size_t is c++ unblockable conversion
	// of negative ints to size_t. Runtime error
	if (length > std::string::npos >> 1)
		throwNonPositive(__PRETTY_FUNCTION__);

	// Example "abc".last(2)
	// min of 2, 3 -> 2 for copylen
	// copy start = data() +3 -2 = data+1, copylen 2

	size_t copylen = std::min(length, this->var_str.size());

	// Construct a new var with the required number of chars from this
	var rvo(this->var_str.data() + this->var_str.size() - copylen, copylen);

	return rvo;
}
//__cpp_lib_string_contains

//[-y]
// var.s(-length) substring
VARREF var::laster(const size_t length) {

	THISIS("VARREF var::laster(const size_t length)")
	assertStringMutator(function_sig);

	// Assume high half of size_t is c++ unblockable conversion
	// of negative ints to size_t. Runtime error
	if (length > std::string::npos >> 1)
		throwNonPositive(__PRETTY_FUNCTION__);

	// Example "abc".last(2)
	// 2 < 3
	// erase 0, 3 - 2 = 1

	// Erase the first part of this string if necessary
	if (length < this->var_str.size()) {
		this->var_str.erase(0, this->var_str.size() - length);
	}

	return *this;
}


//////
// CUT
//////

// var[1,length] = "" 
var var::cut(const int length) const& {

	THISIS("var var::cut(const int length)")
	assertString(function_sig);

	var rvo;
	rvo.var_typ = VARTYP_STR;

	// Assume var_str size is <= max int

	if (length >= static_cast<int>(var_str.size())) {
		// Example "ab".cut(2) return ""
		// Example "ab".cut(3) return ""
		// return empty string
	}

	else if (length >= 0) {
		// Positive - Copy from middle to end
		// Example "ab".cut(0) , append from pos 0, return "ab"
		// Example "ab".cut(1) , append from pos 1, return "b"
		rvo.var_str.append(var_str, length, std::string::npos);

	} else {
		// Negative = Copy first n chars
		// Example "ab".cut(-1) copyn = -1 + 2 = 1, return "a"
		// Example "ab".cut(-2) copyn = -2 + 2 = 0, return ""
		// Example "ab".cut(-3) copyn = -3 + 2 = -1, return ""
		auto copyn = length + static_cast<int>(var_str.size());
		if (copyn > 0)
			rvo.var_str.append(var_str, 0, copyn);
	}

	return rvo;
}

// x[1, length] = ""
// x[-length, length] = ""
VARREF var::cutter(const int length) {

	THISIS("VARREF var::cutter(const int length)")
	assertStringMutator(function_sig);

	if (length >= 0 ) {

		// Positive - cut first n chars. Erase from first char.
		// Example "ab".cutter(0) , erase 0, return "ab"
		// Example "ab".cutter(1) , erase 1, return "b"
		// Example "ab".cutter(2) , erase 2, return ""
		// Example "ab".cutter(3) , erase 3, return ""
		var_str.erase(0, length);

	} else {

		// Negative = cut last n chars. Erase from middle to end.
		// Example "ab".cutter(-1) start_pos = -1 + 2 = erase_pos 1, return "a"
		// Example "ab".cutter(-2) start_pos = -2 + 2 = erase_pos 0, return ""
		// Example "ab".cutter(-3) start_pos = -3 + 2 = erase_pos -1, return ""
		int erase_pos = length + var_str.size();
		if (erase_pos < 1)
			var_str.clear();
		else
			var_str.erase(erase_pos, std::string::npos);
	}

	return *this;
}


/////////
// SUBSTR
/////////

var var::substr(const int startindex1) const& {
	return var(*this).substrer(startindex1);
}

//[x,y]
// var.s(start,length) substring
var var::substr(const int startindex1, const int length) const& {
	return var(*this).substrer(startindex1, length);
}

// in-place
VARREF var::substrer(const int startindex1) {
	// TODO look at using erase to speed up
	this->toString();
	return this->substrer(startindex1, static_cast<int>(var_str.size()));
}

//[x,y]
// var.s(start,length) substring
VARREF var::substrer(const int startindex1, const int length) {

	THISIS("VARREF var::substrer(const int startindex1,const int length)")
	assertStringMutator(function_sig);

	// return "" for ""
	int max = static_cast<int>(var_str.size());
	if (max == 0) {
		var_str.clear();
		return *this;
	}

	int start = startindex1;

	// negative length means reverse the string
	if (length <= 0) {
		if (length == 0) {
			var_str.clear();
			return *this;
		}

		// sya
		if (start < 1) {
			if (start == 0) {
				var_str.clear();
				return *this;
			}
			start = max + start + 1;
			if (start < 1) {
				var_str.clear();
				return *this;
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
		return *this;
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
		return *this;
	}
	int stop = start + length;

	// wcout<<start-1<<stop-start<<endl;

	// TODO use erase for speed instead of copying whole string
	var_str = var_str.substr(start - 1, stop - start);

	return *this;
}

var var::operator[](int charno) const {
	//errputl(__PRETTY_FUNCTION__);
	return this->at(charno);
}

// var.c(charno) character
//var var::operator[](const int charno) const {
var var::at(const int charno) const {
	//std::cout << "rvalue" <<std::endl;

	THISIS("var var::at(const int charno) const")
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

/////////////////////////////////////
// MV (multivalued +++ --- *** /// :::
/////////////////////////////////////

// performs an operation + - * / : on two multivalued strings in parallel
// returning a multivalued string of the results
var var::mv(const char* opcode, CVR var2) const {

	THISIS("var var::multivalued(const char* opcode, CVR var2) const")
	assertString(function_sig);
	ISSTRING(var2)

	var outstr = "";
	var mv1;
	var mv2;
	char separator1 = VM_;
	char separator2 = VM_;

	// pointers into var_str
	// p1a and p1b are zero based indexes of first and last+1 characters of a value in var1
	// (this)
	std::string::size_type p1a = 0;
	std::string::size_type p1b;

	// pointers into var2.var_str
	// p2a and p2b are zero based indexes of first and last+1 characters of a value in var2
	std::string::size_type p2a = 0;
	std::string::size_type p2b;

	while (true) {

		char separator1_prior = separator1;

		// find the end of a value in var1 (this)
		if (separator1 <= separator2) {
getnextp1:
			p1b = var_str.find_first_of(_RM _FM _VM _SM _TM _ST, p1a);
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
			p2b = var2.var_str.find_first_of(_RM _FM _VM _SM _TM _ST, p2a);
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
				outstr ^= mv1 + mv2;
				break;

			case '-':
				outstr ^= mv1 - mv2;
				break;

			case '*':
				outstr ^= mv1 * mv2;
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
				outstr ^= mv1;
				break;

			case ':':
				outstr ^= mv1 ^ mv2;
				break;
		}

		if (separator1 == separator2) {

			// if both pointers past end of their strings then we are done
			if (separator1 > RM_)
				break;

			outstr ^= separator1;
			// outstr.convert(_VM _FM, "]^").outputl("= outstr=");
			p1a++;
			p2a++;
		} else if (separator1 < separator2) {
			outstr ^= separator1;
			// outstr.convert(_VM _FM, "]^").outputl("< outstr=");
			mv2 = "";
			p1a++;
			separator1_prior = separator1;
			goto getnextp1;
		} else {
			outstr ^= separator2;
			// outstr.convert(_VM _FM, "]^").outputl("> outstr=");
			mv1 = "";
			p2a++;
			goto getnextp2;
		}
	}

	return outstr;
}

////////
// SUBSTR upto any specified characters - similar to var::substr3
////////

// returns the characters up to the next delimiter
// also returns the index of the next delimiter discovered or 1 after the string if none (like
// COL2() in pickos) NOTE startindex1 is 1 based not 0. anything less than 1 is treated as 1
var var::substr(const int startindex1, CVR delimiterchars, int& endindex) const {

	THISIS("var var::substr(const int startindex1, VARREF delimiterchars, int& endindex) const")
	assertString(function_sig);
	ISSTRING(delimiterchars)

	std::string::size_type start_pos;

	// domain check min startindex1
	// handle before start of string
	// startindex1 arg is 1 based per mv/pick standard
	// remove treats anything below 1 as 1
	// start_pos variable is zero based standard c++ logic
	if (startindex1 > 0)
		start_pos = startindex1 - 1;
	else
		start_pos = 0;

	// domain check max startindex1
	// handle after end of string
	if (start_pos >= var_str.size()) {
		endindex = static_cast<int>(var_str.size() + 1);
		return "";
	}

	// find the end of the field (or string)
	std::string::size_type end_pos;
	end_pos = var_str.find_first_of(delimiterchars.var_str, start_pos);

	// past of of string?
	if (end_pos == std::string::npos) {
		endindex = static_cast<int>(var_str.size() + 1);
		//return var_str.substr(start_pos, var_str.size() - start_pos);
		return var_str.substr(start_pos);
	}

	// return the index of the dicovered delimiter
	// unlike remove which returns the index of one AFTER the discovered delimiter
	endindex = static_cast<int>(end_pos + 1);

	// extract and return the substr as well
	return var_str.substr(start_pos, end_pos - start_pos);
}

////////
// SUBSTR2 - similar to substr(startindex,delimiterchars) was called remove() in pickos
////////

// returns the characters up to the next delimiter
// delimiter returned as numbers RM=1F=1 FM=1E=2, VM=1D=3 SM=1C=4 TM=1B=5 to ST=1A=6 or 0 if not found
// NOTE startindex1 is 1 based not 0. anything less than 1 is treated as 1
var var::substr2(VARREF startindex1, VARREF delimiterno) const {

	THISIS("var var::substr2(VARREF startindex1, VARREF delimiterno) const")
	assertString(function_sig);
	ISNUMERIC(startindex1)
	ISDEFINED(delimiterno)

	int startindex0 = startindex1.toInt() - 1;
	std::string::size_type start_pos = (startindex0 >= 0) ? startindex0 : 0;

	//var returnable = "";

	// domain check
	// handle before start of string
	// startindex1 arg is 1 based per mv/pick standard
	// treats anything below 1 as 1
	// start_pos variable is zero based standard c++ logic
	// start_pos cannot be < 0
//	if (static_cast<long>(start_pos) < 0)
//		start_pos = 0;

	// domain check
	// handle after end of string
	if (start_pos >= var_str.size()) {
		delimiterno = 0;
		return "";
		//return returnable;
	}

	// find the end of the field (or string)
	std::string::size_type end_pos;
	end_pos = var_str.find_first_of(_RM _FM _VM _SM _TM _ST, start_pos);

	// past of of string?
	if (end_pos == std::string::npos) {
		// wont work if string is the maximum string length but that cant occur
		startindex1 = static_cast<int>(var_str.size() + 2);
		delimiterno = 0;
		return var_str.substr(start_pos, var_str.size() - start_pos);
		//returnable = (var_str.substr(start_pos, var_str.size() - start_pos));
		//return returnable;
	}

	// delimiters returned as numbers RM=1F=1 FM=1E=2, VM=1D=3 SM=1C=4 TM=1B=5 to ST=1A=6
	// delimiterno=int(LASTDELIMITERCHARNOPLUS1-var_str[end_pos]);
	delimiterno = static_cast<int>(*_RM) - static_cast<int>(var_str[end_pos]) + 1;

	// point AFTER the found separator or TWO after the length of the string (TODO shouldnt this
	// be one??/bug in pickos) wont work if string is the maximum string length but that cant
	// occur
	startindex1 = static_cast<int>(end_pos + 2);

	// extract and return the substr as well
	return var_str.substr(start_pos, end_pos - start_pos);
	//returnable = (var_str.substr(start_pos, end_pos - start_pos));
	//return returnable;
}

#if 0

//class var_brackets_proxy
class PUBLIC var_brackets_proxy {
   public:
	var& var_;
	int index_;

	// Constructor from a var and an index
	var_brackets_proxy(VARREF var1, int index) : var_(var1), index_(index) {
	}

	// Implicit conversion to var
	operator var() const {
		return var_.at(index_);
	}

//	// Implicit conversion to char
//	operator char() const {
//		return var_.at(index_).toChar();
//	}

	// Operator assign
	void operator=(const char char1) {
		var_.paster(index_, 1, char1);
	}

	// Operator ==
	template<class T>
	bool operator==(T compare) {
		return var_.at(index_) == compare;
	}

	// Operator !=
	template<class T>
	bool operator!=(T compare) {
		return var_.at(index_) != compare;
	}

	// Operator ^
	template<class T>
	var operator^(T appendage) {
		return var_.at(index_) ^ appendage;
	}

}

// could be used if var_brackets_proxy holds a string perhaps for performance

//implicit conversion to var
var_brackets_proxy::operator var() const {
	//before first character?
	if (index_ < 1) {
		index_ += str_.size();
		//overly negative - return ""
		if (index_ < 1)
			index_ = 1;
	}
	//off end - return ""
	else if (static_cast<unsigned int>(index_) > str_.size())
		return "";

	//within range
	return str_[index_ - 1];
}

//operator assign a char
void var_brackets_proxy::operator=(char char1) {
	//before first character?
	if (index_ < 1) {
		index_ += str_.size();
		//overly negative - return ""
		if (index_ < 1)
			index_ = 1;
	}
	//off end - return ""
	else if (static_cast<unsigned int>(index_) > str_.size()) {
		str_.push_back(char1);
	} else {
		str_[index_ - 1] = char1;
	}

	return;
}
#endif

//////
// SUM
/////
var var::sumall() const {

	THISIS("var var::sumall()")
	assertString(function_sig);

	// Add up all numbers regardless of separators or levels (multilevel)
	// Limit the number of decimal places in returned value to the max found in the input
	// assert(sum("2245000900.76" _FM "102768099.9" _VM "-2347769000.66") == 0);

	var result = 0;
	var start = 0;
	var subfield, term;
	size_t maxdecimals = 0;
	while (true) {
		//this extracts a substring up to any separator charactor ST-RM
		subfield = (*this).substr2(start, term);

		size_t subfieldsize = subfield.var_str.size();
		if (subfieldsize) {
			//for clarity of error message,
			//throw any error here instead of leaving it up to the +=
			if (!subfield.isnum())
				throw VarNonNumeric("sumall() " ^ subfield.first(128).quote());

			result += subfield;
			size_t n = subfield.var_str.find('.');
			if (n) {
				n = subfieldsize - n;
				if (n > maxdecimals)
					maxdecimals = n;
			}
		}
		if (not term)
			break;
	}
	return result.round(maxdecimals);
}

var var::sum() const {

	THISIS("var var::sum()")
	assertString(function_sig);

	// Limit the number of decimal places in returned value to the max found in the input
	// assert(sum("2245000900.76" _VM "102768099.9" _VM "-2347769000.66") == 0);

	var part;	  //num
	var nextsep;  //num
	var accum;	  //num

	var min_sep = ST.seq();  //26
	var max_sep = RM.seq();	  //31

	var min_sep_present;
	for (min_sep_present = min_sep; min_sep_present <= max_sep; ++min_sep_present) {
		if (this->index(this->chr(min_sep_present)))
			break;
	}
	if (min_sep_present > max_sep) {

		//for clarity of error message,
		//throw any error here instead of leaving it up to the +=
		if (!this->isnum())
			throw VarNonNumeric("sum() " ^ this->first(128).quote());

		return (*this) + 0;
	}
	min_sep_present = 1 + max_sep - min_sep_present;
	var inpos = 1;
	var flag = 0;
	var outstr = "";  //NRVO hopefully

	//std::clog << (*this) << std::endl;

	size_t maxndecimals = 0;  //initialise only to avoid warning
	do {

		//extract the next field and get the nextsep field number 1-6 or 0 if none
		part = this->substr2(inpos, nextsep);

		//for clarity of error message,
		//throw any error here instead of leaving it up to the +=
		if (!part.isnum())
			throw VarNonNumeric("sum() " ^ part.first(128).quote());

		if (flag) {

			//accum = (accum + part).round(maxdecimals);
			accum += part;

			//record maximum decimal places on input
			size_t pos = part.var_str.find('.');
			if (pos != std::string::npos) {
				pos = part.var_str.size() - pos - 1;
				if (pos > maxndecimals)
					maxndecimals = pos;
			}

		} else {

			accum = part;

			//record maximum decimal places on input
			size_t pos = part.var_str.find('.');
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
				accum = accum.round(maxndecimals);

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

				outstr.var_str.append(accum.var_str);

			}

			if (nextsep) {
				outstr ^= this->chr(1 + max_sep - nextsep);
			}

			flag = 0;
		}

	} while (nextsep);

	return outstr;	//NRVO hopefully since single named return
}

var var::sum(SV separator) const {

	THISIS("var var::sum(SV separator) const")
	assertString(function_sig);

	var result = 0;
	int nn = this->fcount(separator);

	// static var allseparators=_ST _TM _SM _VM _FM _RM;
	// var separatorn=allseparators.contains(separator);
	// if (!separatorn) return var1*var2;

	// TODO make this faster using remove or index?
	var temp;
	for (int ii = 1; ii <= nn; ++ii) {
		temp = (*this).field(separator, ii);
		if (temp.isnum())
			result += temp;
		else
			//result += (*this).sum(temp);
			result += temp.sum();
	}
	return result;	//NRVO hopefully since single named return
}

}  // namespace exodus
