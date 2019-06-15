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

#define MV_NO_NARROW

// C4530: C++ exception handler used, but unwind semantics are not enabled.
#pragma warning(disable : 4530)

#include <cstring> //for strlen strstr

#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus
{

// includes dim::split

// and var::field,field2,locate,extract,remove,pickreplace,insert,substr,splice,remove

///////
// SPLIT
///////

// dim=var.split()
dim var::split() const
{
	THISIS("var var::split() const")
	THISISSTRING()

	// should use dim's move constructor to place the array directly in place avoiding a slow
	// deep copy and perhaps even copy/move elision to not even copy the base dim object (which
	// contains a pointer to an array of vars)
	dim tempdim2;
	tempdim2.split(*this);
	return tempdim2;
}

// number=dim.split(varstr)
// returns number of elements
var dim::split(const var& str1)
{
	THISIS("var dim::split(const var& var1)")
	ISSTRING(str1)

	// automatically dimension to the size of the string
	this->redim(str1.count(FM_) + 1);

	// empty string just fills array with empty string
	if (str1.length() == 0)
	{
		(*this) = "";
		return this->nrows_;
	}

	// start at the beginning and look for FM delimiters
	std::string::size_type start_pos = 0;
	std::string::size_type next_pos = 0;
	int fieldno;
	for (fieldno = 1; fieldno <= this->nrows_;)
	{

		// find the next FM delimiter
		next_pos = str1.var_str.find(FM_, start_pos);

		// not found - past end of string?
		if (next_pos == std::string::npos)
		{
			this->data_[fieldno] = str1.var_str.substr(start_pos);
			break;
		}

		// fill an element with a field
		this->data_[fieldno] = str1.var_str.substr(start_pos, next_pos - start_pos);

		start_pos = next_pos + 1;
		fieldno++;
	}

	int nfields = fieldno;

	// stuff any excess fields into the last element
	if (next_pos != std::string::npos)
	{
		this->data_[this->nrows_] ^= FM ^ str1.var_str.substr(start_pos);
	}
	else
	{

		++fieldno;
		// fill any remaining array elements with empty string
		for (; fieldno <= (this->nrows_); ++fieldno)
			this->data_[fieldno] = "";
	}

	return nfields;
}

///////////////////////////////////////////
// FIELD
///////////////////////////////////////////

var var::field2(const var& sep, const int fieldno, const int nfields) const
{
	if (fieldno >= 0)
		return field(sep, fieldno, nfields);

	return field(sep, count(sep) + 1 + fieldno + 1, nfields);
}

// FIELD(x,y,z)
// var.field(substr,fieldno,nfields)
var var::field(const var& substrx, const int fieldnx, const int nfieldsx) const
{
	THISIS("var var::field(const var& substrx,const int fieldnx,const int nfieldsx) const")
	THISISSTRING()
	ISSTRING(substrx)

	if (substrx.var_str == "")
		return var("");

	int fieldno = fieldnx > 0 ? fieldnx : 1;
	int nfields = nfieldsx > 0 ? nfieldsx : 1;

	// substr might be multi-byte ... esp. for non-ASCII
	std::string::size_type len_substr = substrx.var_str.length();

	// FIND FIELD

	// find the starting position of the field or return ""
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno)
	{
		start_pos = var_str.find(substrx.var_str, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return var("");
		// start_pos++;
		start_pos += len_substr;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type end_pos = start_pos;
	int pastfieldn = fieldno + nfields;
	while (fieldn2 < pastfieldn)
	{
		end_pos = var_str.find(substrx.var_str, end_pos);
		// past of of string?
		if (end_pos == std::string::npos)
		{
			return var(var_str.substr(start_pos, var_str.length() - start_pos));
		}
		// end_pos++;
		end_pos += len_substr;
		fieldn2++;
	}
	// backup to first character if closing separator in case multi-byte separator
	end_pos -= (len_substr - 1);

	return var(var_str.substr(start_pos, end_pos - start_pos - 1));
}

///////////////////////////////////////////
// FIELD
///////////////////////////////////////////

// FIELDSTORE(x,y,z)
// var.fieldstore(substr,fieldno,nfields,replacement)
var var::fieldstore(const var& sepchar, const int fieldnx, const int nfieldsx,
		    const var& replacementx) const&
{
	THISIS("var var::fieldstore(const var& sepchar,const int fieldnx,const int nfieldsx, const "
	       "var& replacementx) const")
	THISISDEFINED()

	var newmv = *this;
	return newmv.fieldstorer(sepchar, fieldnx, nfieldsx, replacementx);
}

// on temporary
var& var::fieldstore(const var& sepchar, const int fieldnx, const int nfieldsx,
		    const var& replacementx) &&
{
	return this->fieldstorer(sepchar, fieldnx, nfieldsx, replacementx);
}

// in-place
var& var::fieldstorer(const var& sepchar0, const int fieldnx, const int nfieldsx,
		      const var& replacementx)
{
	THISIS("var& var::fieldstorer(const var& sepchar0,const int fieldnx,const int nfieldsx, "
	       "const var& replacementx)")
	THISISSTRING()
	ISSTRING(sepchar0)

	std::string sepchar = sepchar0.var_str;
	if (sepchar == "")
	{
		*this = "";
		return *this;
	}

	// handle multibyte/non-ASCII sepchars
	std::string::size_type sepchar_len = sepchar0.var_str.length();

	int fieldno;
	if (fieldnx > 0)
		fieldno = fieldnx;
	else if (fieldnx < 0)
		fieldno = this->count(sepchar0) + 1 + fieldnx + 1;
	else
		fieldno = 1;

	int nfields = nfieldsx > 0 ? nfieldsx : -nfieldsx;

	// pad replacement if required
	var replacement;
	if (nfieldsx >= 0)
	{
		int nreplacementfields = replacementx.count(sepchar0) + 1;
		// pad to correct number of fields
		if (nreplacementfields < nfields)
		{
			replacement = replacementx;
			for (; nreplacementfields < nfields; nreplacementfields++)
				replacement ^= sepchar;
		}
		else if (nfieldsx && nreplacementfields > nfields)
		{
			replacement = replacementx.field(sepchar, 1, nfields);
		}
		else
			replacement = replacementx;
	}
	else
	{
		replacement = replacementx;
	}

	// FIND FIELD

	// find the starting position of the field or return ""
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno)
	{
		start_pos = var_str.find(sepchar, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
		{
			do
			{
				var_str += sepchar;
				fieldn2++;
			} while (fieldn2 < fieldno);
			var_str += replacement.var_str;
			return *this;
		}
		// start_pos++;
		start_pos += sepchar_len;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type end_pos = start_pos;
	int pastfieldn = fieldno + nfields;
	while (fieldn2 < pastfieldn)
	{
		end_pos = var_str.find(sepchar, end_pos);
		// past of of string?
		if (end_pos == std::string::npos)
		{
			var_str.replace(start_pos, std::string::npos, replacement.var_str);
			return *this;
		}
		// end_pos++;
		end_pos += sepchar_len;
		fieldn2++;
	}

	// backup to first byte of end of field sep
	end_pos -= (sepchar_len - 1);

	// insert or replace
	if (end_pos == start_pos)
	{
		if (nfields == 0)
			replacement.var_str += sepchar;
		var_str.insert(start_pos, replacement.var_str);
	}
	else
	{
		var_str.replace(start_pos, end_pos - start_pos - 1, replacement.var_str);
	}

	return *this;
}

///////////////////////////////////////////
// LOCATE
///////////////////////////////////////////

// hardcore string locate function given a section of a string and all parameters
inline bool locateat(const std::string& var_str, const std::string& target, size_t start_pos,
		     size_t end_pos, const char order, const std::string& usingchar, var& setting)
{
	// private - assume everything is defined/assigned correctly

	//
	// if (target.length()==0&&start_pos==end_pos)
	// if (target.length()==0)
	//{
	//	setting=1;
	//	return true;
	//}

	// arev strangeness? locate even if out of order
	// if this is the PICK standard and needs to be implemented properly it should be
	// implemented as continue to search to end instead of search twice like this this will
	// probably be switched off as unnecessary and slow behaviour for neosys applications
	if (order)
	{
		// THISIS(...)
		// ISSTRING(usingchar)
		bool result = locateat(var_str, target, start_pos, end_pos, 0, usingchar, setting);
		if (result)
			return result;
	}

	// find null in a null field
	if (start_pos >= end_pos)
	{
		setting = 1;
		return !target.length();
	}

	size_t usingchar_len = usingchar.length();

	// for use in AR/DR;
	var value;
	var target2;
	target2 = target;

	// find the starting position of the value or return ""
	// using start_pos and end_pos of
	int targetlen = (int)target.length();
	int valuen2 = 1;
	do
	{
		size_t nextstart_pos = var_str.find(usingchar, start_pos);
		// past end of string?
		// if (nextstart_pos==string::npos)
		//{
		//	setting=valuen2+1;
		//	return false;
		//}
		// nextstart_pos++;
		// past end of field?
		if (nextstart_pos >= end_pos)
		{
			nextstart_pos = end_pos;
			switch (order)
			{
			// No order
			case '\x00':
				if (var_str.substr(start_pos, end_pos - start_pos) == target)
				{
					setting = valuen2;
					return true;
				}
				setting = valuen2 + 1;
				return false;
				break;

			// AL Ascending Left Justified
			case '\x01':
				if (var_str.substr(start_pos, nextstart_pos - start_pos) >= target)
				{
					setting = valuen2;
					if (var_str.substr(start_pos, nextstart_pos - start_pos) ==
					    target)
						return true;
					else
						// arev error strangeness? empty is not greater than
						// any target except empty
						//						if
						//(start_pos==end_pos) setting+=1;
						return false;
				}
				setting = valuen2 + 1;
				return false;
				break;

			// AR Ascending Right Justified
			case '\x02':
				value = var_str.substr(start_pos, nextstart_pos - start_pos);
				if (value >= target2)
				{
					setting = valuen2;
					if (value == target2)
						return true;
					else
						return false;
				}
				setting = valuen2 + 1;
				return false;
				break;

			// DL Descending Left Justified
			case '\x03':
				if (var_str.substr(start_pos, nextstart_pos - start_pos) <= target)
				{
					setting = valuen2;
					if (var_str.substr(start_pos, nextstart_pos - start_pos) ==
					    target)
						return true;
					else
						return false;
				}
				setting = valuen2 + 1;
				return false;
				break;

			// DR Descending Right Justified
			case '\x04':
				value = var_str.substr(start_pos, nextstart_pos - start_pos);
				if (value <= target2)
				{
					setting = valuen2;
					if (value == target2)
						return true;
					else
						return false;
				}
				setting = valuen2 + 1;
				return false;
				break;

			default:
				throw MVException("locateat() invalid mode " ^ var(order));
			}
		}

		// if (var_str.substr(start_pos,nextstart_pos-start_pos)==target)
		// should only test for target up to next sep
		// but arev (by accidental error?) at least checks for the whole target
		// even if the target contains the sep character
		// if (var_str.substr(start_pos,nextstart_pos-start_pos)==target)
		//(int) is to avoid warning of unsigned integer
		switch (order)
		{
		// No order
		case '\x00':
			if (var_str.substr(start_pos, targetlen) == target)
			{
				bool x = (int)(nextstart_pos - start_pos) <= targetlen;
				if (x)
				{
					setting = valuen2;
					return true;
				}
			}
			break;

		// AL Ascending Left Justified
		case '\x01':
			//				//arev strangeness? to locate a field whereever
			//it is regardless of order even ""? 				if
			// (!targetlen&&nextstart_pos==start_pos) break;

			if (var_str.substr(start_pos, nextstart_pos - start_pos) >= target)
			{
				setting = valuen2;
				if (var_str.substr(start_pos, nextstart_pos - start_pos) == target)
					return true;
				else
					return false;
			}
			break;

		// DL Descending Left Justified
		case '\x02':
			//				//arev strangeness? to locate a field whereever
			//it is regardless of order even ""? 				if
			// (!targetlen&&nextstart_pos==start_pos) break;

			value = var_str.substr(start_pos, nextstart_pos - start_pos);
			if (value >= target2)
			{
				setting = valuen2;
				if (value == target2)
					return true;
				else
					return false;
			}
			break;

		// AR Ascending Right Justified
		case '\x03':
			//				//arev strangeness? to locate a field whereever
			//it is regardless of order even ""? 				if
			// (!targetlen&&nextstart_pos==start_pos) break;

			if (var_str.substr(start_pos, nextstart_pos - start_pos) <= target)
			{
				setting = valuen2;
				if (var_str.substr(start_pos, nextstart_pos - start_pos) == target)
					return true;
				else
					return false;
			}
			break;

		// DR Descending Right Justified
		case '\x04':
			//				//arev strangeness? to locate a field whereever
			//it is regardless of order even ""? 				if
			// (!targetlen&&nextstart_pos==start_pos) break;

			value = var_str.substr(start_pos, nextstart_pos - start_pos);
			if (value <= target2)
			{
				setting = valuen2;
				if (value == target2)
					return true;
				else
					return false;
			}
			break;

		default:
			throw MVException("locateat() invalid order" ^ var(order));
		}
		// skip over any sep character
		start_pos = nextstart_pos + usingchar_len;
		valuen2++;
	} while (true);
}

// locate within extraction
inline bool locatex(const std::string& var_str, const std::string& target, const char* ordercode,
		    const std::string& usingchar, var& setting, int fieldno, int valueno,
		    const int subvalueno)
{
	// private - assume everything is defined/assigned correctly

	// any negatives at all returns ""
	// done inline since unusual
	// if (fieldno<0||valueno<0||subvalueno<0) return var("")

	char ordermode;
	if (strlen(ordercode) == 0)
		ordermode = 0;
	else
	{
		// locate the order code in a list of the four possible order codes
		// and throw if not found
		const char* ordercodes = "ALARDLDR";
		const char* orderindex = strstr(ordercodes, ordercode);
		if (orderindex == NULL)
			throw MVException("locateby('" ^ var(ordercode) ^ "') is invalid");

		// convert the memory address to the char position within the codes
		ordermode = int(orderindex - ordercodes);
		// add two and divide by two to get the order no AL=1 AR=2 DL=3 DR=4
		ordermode = (ordermode + 2) >> 1;
	}

	// zero means all, negative return ""
	// if (fieldno<=0)     (but locate x<0> using VM should work too
	if (fieldno <= 0)
	{
		// locate negative field number always returns false and setting 1
		if (fieldno < 0)
		{
			setting = 1;
			return !target.length();
		}

		if (valueno || subvalueno)
			fieldno = 1;
		else
		{
			return locateat(var_str, target, (size_t)0, var_str.length(), ordermode,
					usingchar, setting);
		}
	}

	// find the starting position of the field or return ""
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno)
	{
		start_pos = var_str.find(FM_, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
		{
			// if (valueno||subvalueno) setting=1;
			// else setting=fieldn2+1;
			setting = 1;
			return !target.length();
		}
		start_pos++;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type field_end_pos;
	field_end_pos = var_str.find(FM_, start_pos);
	if (field_end_pos == std::string::npos)
		field_end_pos = var_str.length();

	// FIND VALUE

	if (start_pos >= field_end_pos)
	{
		setting = 1;
		return !target.length();
	}

	// zero means all, negative return ""
	if (valueno <= 0)
	{
		if (valueno < 0)
		{
			setting = 1;
			return !target.length();
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
	while (valuen2 < valueno)
	{
		start_pos = var_str.find(VM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos)
		{
			// if (subvalueno) setting=1;
			// else setting=valuen2+1;
			setting = 1;
			return !target.length();
		}
		start_pos++;
		// past end of field?
		if (start_pos > field_end_pos)
		{
			// setting=valuen2+1;
			setting = 1;
			return !target.length();
		}
		valuen2++;
	}

	// find the end of the value (or string)
	std::string::size_type value_end_pos;
	value_end_pos = var_str.find(VM_, start_pos);
	if (value_end_pos == std::string::npos || value_end_pos > field_end_pos)
		value_end_pos = field_end_pos;

	// FIND SUBVALUE

	if (start_pos >= value_end_pos)
	{
		setting = 1;
		return !target.length();
	}

	// zero means all, negative means ""
	if (subvalueno == 0)
		return locateat(var_str, target, start_pos, value_end_pos, ordermode, usingchar,
				setting);
	if (subvalueno < 0)
	{
		setting = 1;
		return !target.length();
	}

	// find the starting position of the subvalue or return ""
	// using start_pos and end_pos of
	int subvaluen2 = 1;
	while (subvaluen2 < subvalueno)
	{
		start_pos = var_str.find(SM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos)
		{
			// setting=subvaluen2+1;
			setting = 1;
			return !target.length();
		}
		start_pos++;
		// past end of value?
		if (start_pos > value_end_pos)
		{
			// setting=subvaluen2+1;
			setting = 1;
			return !target.length();
		}
		subvaluen2++;
	}

	// find the end of the subvalue (or string)
	std::string::size_type subvalue_end_pos;
	subvalue_end_pos = var_str.find(SM_, start_pos);
	if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
	{
		return locateat(var_str, target, start_pos, value_end_pos, ordermode, usingchar,
				setting);
	}

	return locateat(var_str, target, start_pos, subvalue_end_pos, ordermode, usingchar,
			setting);
}

// default locate using VM
bool var::locate(const var& target, var& setting) const
{
	THISIS("bool var::locate(const var& target, var& setting, const int fieldno/*=0*/,const "
	       "int valueno/*=0*/) const")
	THISISSTRING()
	ISSTRING(target)
	ISDEFINED(setting)

	return locatex(var_str, target.var_str, "", _VM_, setting, 0, 0, 0);
}

bool var::locate(const var& target, var& setting, const int fieldno, const int valueno /*=0*/) const
{
	THISIS("bool var::locate(const var& target, var& setting, const int fieldno/*=0*/,const "
	       "int valueno/*=0*/) const")
	THISISSTRING()
	ISSTRING(target)
	ISDEFINED(setting)

	std::string usingchar;
	if (valueno != 0)
		usingchar = _SM_;
	else if (fieldno != 0)
		usingchar = _VM_;
	else
		usingchar = _FM_;
	// if (fieldno<=0) *usingchar=FM_;
	// else if (valueno<=0) *usingchar=VM_;
	// else *usingchar=SM_;

	return locatex(var_str, target.var_str, "", usingchar, setting, fieldno, valueno, 0);
}

// without setting
bool var::locate(const var& target) const
{
	THISIS("bool var::locate(const var& target")
	THISISSTRING()
	ISSTRING(target)

	var setting;
	return locatex(var_str, target.var_str, "", _VM_, setting, 0, 0, 0);
}

///////////////////////////////////////////
// LOCATE BY
///////////////////////////////////////////

// this version caters for the rare syntax where the order is given as a variable
bool var::locateby(const var& ordercode, const var& target, var& setting, const int fieldno,
		   const int valueno /*=0*/) const
{
	THISIS("bool var::locateby(const var& ordercode, const var& target, var& setting, const "
	       "int fieldno, const int valueno/*=0*/)const")
	ISSTRING(ordercode)

	return locateby(ordercode.toString().c_str(), target, setting, fieldno, valueno);
}

// no fieldno or valueno means locate using character VM
// this version caters for the rare syntax where the order is given as a variable
bool var::locateby(const var& ordercode, const var& target, var& setting) const
{
	THISIS("bool var::locateby(const var& ordercode, const var& target, var& setting) const")
	ISSTRING(ordercode)

	return locateby(ordercode.toString().c_str(), target, setting);
}

// no fieldno or valueno means locate using character VM
// specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locateby(const char* ordercode, const var& target, var& setting) const
{
	THISIS("bool var::locateby(const char* ordercode, const var& target, var& setting) const")
	THISISSTRING()
	ISSTRING(target)
	ISDEFINED(setting)

	// TODO either make a "locatefrom" version of the above where the locate STARTS its search
	// from the last numbered subvalue (add a new parameter), value or field. OR possibly modify
	// this function to understand a negative number as "start from" instead of "within this"

	// determine locate by field, value or subvalue depending on the parameters as follows:
	// if value number is given then locate in subvalues of that value
	// if field number is given then locate in values of that field
	// otherwise locate in fields of the string

	return locatex(var_str, target.var_str, ordercode, _VM_, setting, 0, 0, 0);
}

// specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locateby(const char* ordercode, const var& target, var& setting, const int fieldno,
		   const int valueno /*=0*/) const
{
	THISIS("bool var::locateby(const char* ordercode, const var& target, var& setting, const "
	       "int fieldno, const int valueno/*=0*/) const")
	THISISSTRING()
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

///////////////////////////////////////////
// LOCATE BY USING
///////////////////////////////////////////

// this version caters for the rare syntax where the order is given as a variable
bool var::locatebyusing(const var& ordercode, const var& usingchar, const var& target, var& setting,
			const int fieldno /*=0*/, const int valueno /*=0*/) const
{
	THISIS("bool var::locatebyusing(const var& ordercode, const var& usingchar, const var& "
	       "target, var& setting, const int fieldno//*=0*//, const int valueno//*=0*//)const")
	ISSTRING(ordercode)
	ISSTRING(usingchar)
	ISSTRING(target)

	// return locatebyusing(ordercode.toString().c_str(), usingchar.toString().c_str(), target,
	// setting, fieldno, valueno);
	return locatex(var_str, target.var_str, ordercode.toString().c_str(), usingchar.var_str,
		       setting, fieldno, valueno, 0);
}

// specialised const char version of ordercode for speed of usual syntax where ordermode is given as
// string it avoids the conversion from string to var and back again
bool var::locatebyusing(const char* ordercode, const char* usingchar, const var& target,
			var& setting, const int fieldno /*=0*/, const int valueno /*=0*/) const
{
	THISIS("bool var::locatebyusing(const char* ordercode, const char* usingchar, const var& "
	       "target, var& setting, const int fieldno//*=0*//, const int valueno//*=0*//) const")
	THISISSTRING()
	ISSTRING(target)
	ISDEFINED(setting)

	// TODO either make a "locatefrom" version of the above where the locate STARTS its search
	// from the last numbered subvalue (add a new parameter), value or field. OR possibly modify
	// this function to understand a negative number as "start from" instead of "within this"

	// determine locate by field, value or subvalue depending on the parameters as follows:
	// if value number is given then locate in subvalues of that value
	// if field number is given then locate in values of that field
	// otherwise locate in fields of the string

	return locatex(var_str, target.var_str, ordercode, usingchar, setting, fieldno, valueno, 0);
}

///////////////////////////////////////////
// LOCATE USING
///////////////////////////////////////////

bool var::locateusing(const var& usingchar, const var& target) const
{
	THISIS("bool var::locateusing(const var& usingchar, const var& target) const")
	THISISSTRING()
	ISSTRING(target)
	ISSTRING(usingchar)
	// ISDEFINED(setting)

	var setting = "";
	return locatex(var_str, target.var_str, "", usingchar.var_str, setting, 0, 0, 0);
}

bool var::locateusing(const var& usingchar, const var& target, var& setting,
		      const int fieldno /*=0*/, const int valueno /*=0*/,
		      const int subvalueno /*=0*/) const
{
	THISIS("bool var::locateusing(const var& usingchar, const var& target, var& setting, const "
	       "int fieldno/*=0*/, const int valueno/*=0*/, const int subvalueno/*=0*/) const")
	THISISSTRING()
	ISSTRING(target)
	ISSTRING(usingchar)
	ISDEFINED(setting)

	return locatex(var_str, target.var_str, "", usingchar.var_str, setting, fieldno, valueno,
		       subvalueno);
}

///////////////////////////////////////////
// EXTRACT
///////////////////////////////////////////

var var::operator()(int fieldno, int valueno, int subvalueno) const
{
	//std::cout << "var" << std::endl;
	return a(fieldno, valueno, subvalueno);
}

/*
var& var::operator()(int fieldno, int valueno, int subvalueno)
{
	std::cout << "var&" << std::endl;
	//return extract(fieldno, valueno, subvalueno);
	return (*this);
}
*/

var var::extract(const int argfieldn, const int argvaluen, const int argsubvaluen) const
{
	return a(argfieldn, argvaluen, argsubvaluen);
}

// extract int int int
// NB declared with value=0 and subvalue=0
var var::a(const int argfieldn, const int argvaluen, const int argsubvaluen) const
{
	THISIS("var var::extract(const int argfieldn, const int argvaluen, const int argsubvaluen) "
	       "const")
	THISISSTRING()

	// any negatives at all returns ""
	// done inline since unusual
	// if (fieldno<0||valueno<0||subvalueno<0) return var("")

	// FIND FIELD

	int fieldno = argfieldn;
	int valueno = argvaluen;
	int subvalueno = argsubvaluen;
	// zero means all, negative return ""
	if (fieldno <= 0)
	{
		if (fieldno < 0)
			return var("");
		if (valueno || subvalueno)
			fieldno = 1;
		else
			return var(var_str);
	}

	// find the starting position of the field or return ""
	std::string::size_type start_pos = 0;
	int fieldn2 = 1;
	while (fieldn2 < fieldno)
	{
		start_pos = var_str.find(FM_, start_pos);
		// past of of string?
		if (start_pos == std::string::npos)
			return var("");
		start_pos++;
		fieldn2++;
	}

	// find the end of the field (or string)
	std::string::size_type field_end_pos;
	field_end_pos = var_str.find(FM_, start_pos);
	if (field_end_pos == std::string::npos)
		field_end_pos = var_str.length();

	// FIND VALUE

	// zero means all, negative return ""
	if (valueno <= 0)
	{
		if (valueno < 0)
			return var("");
		if (subvalueno)
			valueno = 1;
		else
			return var(var_str.substr(start_pos, field_end_pos - start_pos));
	}

	// find the starting position of the value or return ""
	// using start_pos and end_pos of
	int valuen2 = 1;
	while (valuen2 < valueno)
	{
		start_pos = var_str.find(VM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos)
			return var("");
		start_pos++;
		// past end of field?
		if (start_pos > field_end_pos)
			return var("");
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
		return var(var_str.substr(start_pos, value_end_pos - start_pos));
	if (subvalueno < 0)
		return var("");

	// find the starting position of the subvalue or return ""
	// using start_pos and end_pos of
	int subvaluen2 = 1;
	while (subvaluen2 < subvalueno)
	{
		start_pos = var_str.find(SM_, start_pos);
		// past end of string?
		if (start_pos == std::string::npos)
			return var("");
		start_pos++;
		// past end of value?
		if (start_pos > value_end_pos)
			return var("");
		subvaluen2++;
	}

	// find the end of the subvalue (or string)
	std::string::size_type subvalue_end_pos;
	subvalue_end_pos = var_str.find(SM_, start_pos);
	if (subvalue_end_pos == std::string::npos || subvalue_end_pos > value_end_pos)
		return var(var_str.substr(start_pos, value_end_pos - start_pos));

	return var(var_str.substr(start_pos, subvalue_end_pos - start_pos));
}

/////////////////////////////////////////////////
// REMOVE int fieldno, int valueno=0, int subvalueno=0
/////////////////////////////////////////////////

// var var::erase(const int fieldno,const int valueno,const int subvalueno) const
var var::remove(const int fieldno, const int valueno, const int subvalueno) const
{
	THISIS("var var::remove(const int fieldno,const int valueno,const int subvalueno) const")
	THISISSTRING()

	var newmv = *this;
	// return newmv.eraser(fieldno,valueno,subvalueno);
	return newmv.remover(fieldno, valueno, subvalueno);
}

// var& var::eraser(int fieldno,int valueno,int subvalueno)
var& var::remover(int fieldno, int valueno, int subvalueno)
{
	THISIS("var& var::remover(int fieldno,int valueno,int subvalueno)")
	THISISSTRING()

	// return "" if replacing 0,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0)
	{
		// functionmode return var("");//var(var1);
		// proceduremode
		var_str = "";
		var_typ = VARTYP_STR;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::string::size_type start_pos = 0;
	std::string::size_type field_end_pos;

	// negative means return all
	if (fieldno < 0)
	{
		return *this;
	}
	else
	{

		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno)
		{
			start_pos = var_str.find(FM_, start_pos);
			// past of of string?
			if (start_pos == std::string::npos)
			{
				return *this;
			}
			start_pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		if (start_pos == std::string::npos)
		{
			start_pos = var_str.length();
			field_end_pos = start_pos;
		}
		else
		{
			field_end_pos = var_str.find(FM_, start_pos);
			if (field_end_pos == std::string::npos)
				field_end_pos = var_str.length();
		}
	}

	////////////// FIND VALUE ///////////////////
	std::string::size_type value_end_pos;

	// zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno < 0)
	{
		return *this;
	}
	else if (valueno == 0 && subvalueno == 0)
	{
		if (fieldno > 1)
			start_pos--;
		else if (field_end_pos < var_str.length())
			field_end_pos++;
		var_str.erase(start_pos, field_end_pos - start_pos);
		return *this;
	}
	else
	{

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		while (valuen2 < valueno)
		{
			start_pos = var_str.find(VM_, start_pos);
			// past end of string or field?
			if (start_pos >= field_end_pos || start_pos == std::string::npos)
			{
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

	// zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno < 0)
	{
		return *this;
	}
	else if (subvalueno == 0)
	{
		if (valueno > 1)
			start_pos--;
		else if (value_end_pos < field_end_pos)
			value_end_pos++;
		var_str.erase(start_pos, value_end_pos - start_pos);
		return *this;
	}
	else
	{
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		while (subvaluen2 < subvalueno)
		{
			start_pos = var_str.find(SM_, start_pos);
			// past end of string or value
			if (start_pos >= value_end_pos || start_pos == std::string::npos)
			{
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

var var::pickreplace(const int fieldno, const int valueno, const int subvalueno,
		     const var& replacement) const
{
	THISIS("var var::pickreplace(const int fieldno,const int valueno,const int "
	       "subvalueno,const var& replacement) const")
	THISISSTRING()

	return var(*this).r(fieldno, valueno, subvalueno, replacement);
}

var var::pickreplace(const int fieldno, const int valueno, const var& replacement) const
{
	THISIS("var var::pickreplace(const int fieldno,const int valueno,const var& replacement) "
	       "const")
	THISISSTRING()

	return var(*this).r(fieldno, valueno, 0, replacement);
}

var var::pickreplace(const int fieldno, const var& replacement) const
{
	THISIS("var var::pickreplace(const int fieldno,const var& replacement) const")
	THISISSTRING()

	return var(*this).r(fieldno, 0, 0, replacement);
}

var& var::r(const int fieldno, const int valueno, const var& replacement)
{
	THISIS("var var::r(const int fieldno,const int valueno,const var& replacement")
	THISISSTRING()

	return r(fieldno, valueno, 0, replacement);
}

var& var::r(const int fieldno, const var& replacement)
{
	THISIS("var var::r(const int fieldno,const var& replacement)")
	THISISSTRING()

	return r(fieldno, 0, 0, replacement);
}

var& var::r(int fieldno, int valueno, int subvalueno, const var& replacement)
{
	THISIS("var& var::r(int fieldno,int valueno,int subvalueno,const var& replacement)")
	THISISSTRING()
	ISSTRING(replacement)

	// return whole thing if replace 0,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0)
	{
		// functionmode return var(replacement);
		// proceduremode
		var_str = replacement.var_str;
		var_typ = VARTYP_STR;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::string::size_type start_pos = 0;
	std::string::size_type field_end_pos;

	// negative means append
	if (fieldno < 0)
	{
		// append a FM_ only if there is existing data
		if (var_str.length() != 0)
			var_str += FM_;
		start_pos = var_str.length();
		field_end_pos = start_pos;
	}
	else
	{

		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno)
		{
			start_pos = var_str.find(FM_, start_pos);
			// past of of string?
			if (start_pos == std::string::npos)
			{
				var_str.append(fieldno - fieldn2, FM_);
				// start_pos=var_str.length();
				// fieldn2=fieldno
				break;
			}
			start_pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		if (start_pos == std::string::npos)
		{
			start_pos = var_str.length();
			field_end_pos = start_pos;
		}
		else
		{
			field_end_pos = var_str.find(FM_, start_pos);
			if (field_end_pos == std::string::npos)
				field_end_pos = var_str.length();
		}
	}

	////////////// FIND VALUE ///////////////////
	std::string::size_type value_end_pos;

	// zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno < 0)
	{
		if (field_end_pos - start_pos > 0)
		{
			var_str.insert(field_end_pos, _VM_);
			field_end_pos++;
		}
		start_pos = field_end_pos;
		value_end_pos = field_end_pos;
	}
	else if (valueno == 0 && subvalueno == 0)
	{
		var_str.replace(start_pos, field_end_pos - start_pos, replacement.var_str);
		return *this;
	}
	else
	{

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		while (valuen2 < valueno)
		{
			start_pos = var_str.find(VM_, start_pos);
			// past end of string or field?
			if (start_pos >= field_end_pos || start_pos == std::string::npos)
			{
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
	if (subvalueno < 0)
	{
		if (value_end_pos - start_pos > 0)
		{
			var_str.insert(value_end_pos, _SM_);
			value_end_pos++;
		}
		start_pos = value_end_pos;
		subvalue_end_pos = value_end_pos;
	}
	else if (subvalueno == 0)
	{
		var_str.replace(start_pos, value_end_pos - start_pos, replacement.var_str);
		return *this;
	}
	else
	{
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		while (subvaluen2 < subvalueno)
		{
			start_pos = var_str.find(SM_, start_pos);
			// past end of string or value
			if (start_pos >= value_end_pos || start_pos == std::string::npos)
			{
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

var var::insert(const int fieldno, const int valueno, const int subvalueno,
		const var& insertion) const&
{
	THISIS("var var::insert(const int fieldno,const int valueno,const int subvalueno,const "
	       "var& insertion) const")
	THISISSTRING()

	var newmv = var(*this).inserter(fieldno, valueno, subvalueno, insertion);
	return newmv;
}

// given only field and value numbers
var var::insert(const int fieldno, const int valueno, const var& insertion) const&
{
	return this->insert(fieldno, valueno, 0, insertion);
}

// given only field number
var var::insert(const int fieldno, const var& insertion) const&
{
	return this->insert(fieldno, 0, 0, insertion);
}

// on temporary
var& var::insert(const int fieldno, const int valueno, const int subvalueno,
		const var& insertion) &&
{
	return this->inserter(fieldno, valueno, subvalueno, insertion);
}

// on temporary - given only field and value number
var& var::insert(const int fieldno, const int valueno, const var& insertion) &&
{
	return this->inserter(fieldno, valueno, 0, insertion);
}

// on temporary - given only field number
var& var::insert(const int fieldno, const var& insertion) &&
{
	return this->inserter(fieldno, 0, 0, insertion);
}

// in-place - given field and value no
var& var::inserter(const int fieldno, const int valueno, const var& insertion)
{
	return this->inserter(fieldno, valueno, 0, insertion);
}

// in-place given only field no
var& var::inserter(const int fieldno, const var& insertion)
{
	return this->inserter(fieldno, 0, 0, insertion);
}

//in-place - given everything
var& var::inserter(const int fieldno, const int valueno, const int subvalueno, const var& insertion)
{
	THISIS("var& var::inserter(const int fieldno,const int valueno,const int subvalueno,const "
	       "var& insertion)")
	THISISSTRING()
	ISSTRING(insertion)

	// 0,0,0 is like 1,0,0
	if (fieldno == 0 && valueno == 0 && subvalueno == 0)
	{
		if (var_str.length())
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
	if (fieldno < 0)
	{
		// append a FM_ only if there is existing data
		if (var_str.length() != 0)
			var_str += FM_;
		pad = true;
		start_pos = var_str.length();
		field_end_pos = start_pos;
	}
	else
	{
		// find the starting position of the field or append enough FM_ to satisfy
		int fieldn2 = 1;
		while (fieldn2 < fieldno)
		{
			start_pos = var_str.find(FM_, start_pos);
			// past of of string?
			if (start_pos == std::string::npos)
			{
				pad = true;
				var_str.append(fieldno - fieldn2, FM_);
				// start_pos=var_str.length();
				// fieldn2=fieldno
				break;
			}
			start_pos++;
			fieldn2++;
		}

		// find the end of the field (or string)
		if (start_pos == std::string::npos)
		{
			start_pos = var_str.length();
			field_end_pos = start_pos;
		}
		else
		{
			field_end_pos = var_str.find(FM_, start_pos);
			if (field_end_pos == std::string::npos)
				field_end_pos = var_str.length();
		}
	}

	////////////// FIND VALUE ///////////////////
	std::string::size_type value_end_pos;

	// zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno < 0)
	{
		if (field_end_pos - start_pos > 0)
		{
			var_str.insert(field_end_pos, _VM_);
			field_end_pos++;
		}
		start_pos = field_end_pos;
		value_end_pos = field_end_pos;
		pad = true;
	}
	else if (valueno == 0 && subvalueno == 0)
	{
		if (!pad && var_str.length())
			var_str.insert(start_pos, _FM_);
		var_str.insert(start_pos, insertion.var_str);
		return *this;
	}
	else
	{

		// find the starting position of the value or insert enough VM_ to satisfy
		int valuen2 = 1;
		while (valuen2 < valueno)
		{
			start_pos = var_str.find(VM_, start_pos);
			// past end of string or field?
			if (start_pos >= field_end_pos || start_pos == std::string::npos)
			{
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
	std::string::size_type subvalue_end_pos;

	// zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno < 0)
	{
		if (value_end_pos - start_pos > 0)
		{
			var_str.insert(value_end_pos, _SM_);
			value_end_pos++;
		}
		start_pos = value_end_pos;
		//not required
		//subvalue_end_pos = value_end_pos;
		pad = true;
	}
	else if (subvalueno == 0)
	{
		if (!pad && (start_pos < field_end_pos || valueno > 1))
			var_str.insert(start_pos, _VM_);
		var_str.insert(start_pos, insertion.var_str);
		return *this;
	}
	else
	{
		// find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2 = 1;
		while (subvaluen2 < subvalueno)
		{
			start_pos = var_str.find(SM_, start_pos);
			// past end of string or value
			if (start_pos >= value_end_pos || start_pos == std::string::npos)
			{
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
		var_str.insert(start_pos, _SM_);
	var_str.insert(start_pos, insertion.var_str);

	return *this;
}

////////
// SUBSTR
////////

var var::substr(const int startindex1) const&
{
	THISIS("var var::substr(const int startindex1) const")
	THISISSTRING()

	return var(*this).substrer(startindex1);
}

// on temporary
var& var::substr(const int startindex1) &&
{
	return this->substrer(startindex1);
}

//[x,y]
// var.s(start,length) substring
var var::substr(const int startindex1, const int length) const&
{
	THISIS("var var::substr(const int startindex1,const int length) const")
	THISISSTRING()

	return var(*this).substrer(startindex1, length);
}

// on temporary
var& var::substr(const int startindex1, const int length) &&
{
	return this->substrer(startindex1, length);
}

// in-place
var& var::substrer(const int startindex1)
{
	THISIS("var& var::substrer(const int startindex1) const")
	THISISSTRING()

	// TODO look at using erase to speed up
	return substrer(startindex1, (int)var_str.length());
}

//[x,y]
// var.s(start,length) substring
var& var::substrer(const int startindex1, const int length)
{
	THISIS("var& var::substrer(const int startindex1,const int length) const")
	THISISSTRING()

	// return "" for ""
	int max = (int)var_str.length();
	if (max == 0)
	{
		var_str = "";
		var_typ = VARTYP_INTSTR;
		var_int = 0;
		return *this;
	}

	int start = startindex1;

	// negative length means reverse the string
	if (length <= 0)
	{
		if (length == 0)
		{
			var_str = "";
			var_typ = VARTYP_INTSTR;
			var_int = 0;
			return *this;
		}

		// sya
		if (start < 1)
		{
			if (start == 0)
			{
				var_str = "";
				var_typ = VARTYP_INTSTR;
				var_int = 0;
				return *this;
			}
			start = max + start + 1;
			if (start < 1)
			{
				var_str = "";
				var_typ = VARTYP_INTSTR;
				var_int = 0;
				return *this;
			}
		}
		else if (start > max)
		{
			start = max;
		}

		int stop = start + length + 1;
		if (stop < 1)
			stop = 1;

		std::string result = "";
		for (int ii = start; ii >= stop; ii--)
			result += var_str[ii - 1];

		var_str = result;
		var_typ = VARTYP_STR;
		return *this;
	}

	if (start < 1)
	{
		if (start == 0)
			start = 1;
		else
		{
			start = max + start + 1;
			if (start < 1)
				start = 1;
		}
	}
	else if (start > max)
	{
		var_str = "";
		var_typ = VARTYP_INTSTR;
		var_int = 0;
		return *this;
	}
	int stop = start + length;

	// wcout<<start-1<<stop-start<<endl;

	// TODO use erase for speed instead of copying whole string
	var_str = var_str.substr(start - 1, stop - start);
	var_typ = VARTYP_STR;

	return *this;
}

/*
var_brackets_proxy var::operator[](int charno) const&
{
        std::cout << "lvalue" <<std::endl;
        return var_brackets_proxy{var_str,charno};
}

//[x]
var var::operator[](int charno) &&
{
*/

// var.c(charno) character
var var::operator[](const int charno) const
{
	//std::cout << "rvalue" <<std::endl;
	THISIS("var var::operator[](const int charno) const")
	THISISSTRING()

	int nchars = int(var_str.length());

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
	if (charno == 0)
	{
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
		return var_str[charno2]; // no need for -1 here

	// otherwise so negative as to point before beginning of string
	// and rule is to return the first character in that case
	return var_str[0];
}

/////////////////////////////////////
// MV (multivalued +++ --- *** /// :::
/////////////////////////////////////

// performs an operation + - * / : on two multivalued strings in parallel
// returning a multivalued string of the results
var var::mv(const char* opcode, const var& var2) const
{
	THISIS("var var::multivalued(const char* opcode, const var& var2) const")
	THISISSTRING()
	ISSTRING(var2)

	var outstr = "";
	var mv1;
	var mv2;
	char sepchar1 = VM_;
	char sepchar2 = VM_;

	// pointers into this->var_str
	// p1a and p1b are zero based indexes of first and last+1 characters of a value in var1
	// (this)
	std::string::size_type p1a = 0;
	std::string::size_type p1b;

	// pointers into var2.var_str
	// p2a and p2b are zero based indexes of first and last+1 characters of a value in var2
	std::string::size_type p2a = 0;
	std::string::size_type p2b;

	while (true)
	{

		char sepchar1_prior = sepchar1;

		// find the end of a value in var1 (this)
		if (sepchar1 <= sepchar2)
		{
		getnextp1:
			p1b = this->var_str.find_first_of(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_, p1a);
			if (p1b == std::string::npos)
			{
				sepchar1 = RM_ + 1;
			}
			else
			{
				sepchar1 = this->var_str[p1b];
			}
			mv1 = var(this->var_str.substr(p1a, p1b - p1a)); //.outputl("mv1=");
			p1a = p1b;
		}

		// find the end of a value in var1 (this)
		if (sepchar2 <= sepchar1_prior)
		{
		getnextp2:
			p2b = var2.var_str.find_first_of(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_, p2a);
			if (p2b == std::string::npos)
			{
				sepchar2 = RM_ + 1;
			}
			else
			{
				sepchar2 = var2.var_str[p2b];
			}
			mv2 = var(var2.var_str.substr(p2a, p2b - p2a)); //.outputl("mv2=");
			p2a = p2b;
		}

		switch (opcode[0])
		{

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

		if (sepchar1 == sepchar2)
		{

			// if both pointers past end of their strings then we are done
			if (sepchar1 > RM_)
				break;

			outstr ^= sepchar1;
			// outstr.convert(_VM_ _FM_, "]^").outputl("= outstr=");
			p1a++;
			p2a++;
		}
		else if (sepchar1 < sepchar2)
		{
			outstr ^= sepchar1;
			// outstr.convert(_VM_ _FM_, "]^").outputl("< outstr=");
			mv2 = "";
			p1a++;
			sepchar1_prior = sepchar1;
			goto getnextp1;
		}
		else
		{
			outstr ^= sepchar2;
			// outstr.convert(_VM_ _FM_, "]^").outputl("> outstr=");
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
// COL2() in arev) NOTE startindex1 is 1 based not 0. anything less than 1 is treated as 1
var var::substr(const int startindex1, const var& delimiterchars, int& endindex) const
{
	THISIS("var var::substr(const int startindex1, var& delimiterchars, int& endindex) const")
	THISISSTRING()
	ISSTRING(delimiterchars)

	std::string::size_type start_pos = startindex1 - 1;

	// domain check min startindex1
	// handle before start of string
	// startindex1 arg is 1 based per mv/pick standard
	// remove treats anything below 1 as 1
	// start_pos variable is zero based standard c++ logic
	if (long(start_pos) < 0)
		start_pos = 0;

	// domain check max startindex1
	// handle after end of string
	if (start_pos >= var_str.length())
	{
		endindex = int(var_str.length() + 1);
		return "";
	}

	// find the end of the field (or string)
	std::string::size_type end_pos;
	end_pos = var_str.find_first_of(delimiterchars.var_str, start_pos);

	// past of of string?
	if (end_pos == std::string::npos)
	{
		endindex = int(var_str.length() + 1);
		return var(var_str.substr(start_pos, var_str.length() - start_pos));
	}

	// return the index of the dicovered delimiter
	// unlike remove which returns the index of one AFTER the discovered delimiter
	endindex = int(end_pos + 1);

	// extract and return the substr as well
	return var(var_str.substr(start_pos, end_pos - start_pos));
}

////////
// SUBSTR2 - similar to substr(startindex,delimiterchars) was called remove() in pick/arev
////////

// returns the characters up to the next delimiter
// NOTE startindex1 is 1 based not 0. anything less than 1 is treated as 1
var var::substr2(var& startindex1, var& delimiterno) const
{
	THISIS("var var::substr2(var& startindex1, var& delimiterno) const")
	THISISSTRING()
	ISNUMERIC(startindex1)
	ISDEFINED(delimiterno)

	std::string::size_type start_pos = startindex1.toInt() - 1;

	// domain check
	// handle before start of string
	// startindex1 arg is 1 based per mv/pick standard
	// treats anything below 1 as 1
	// start_pos variable is zero based standard c++ logic
	if (long(start_pos) < 0)
		start_pos = 0;

	// domain check
	// handle after end of string
	if (start_pos >= var_str.length())
	{
		delimiterno = 0;
		return "";
	}

	// find the end of the field (or string)
	std::string::size_type end_pos;
	end_pos = var_str.find_first_of(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_, start_pos);

	// past of of string?
	if (end_pos == std::string::npos)
	{
		// wont work if string is the maximum string length but that cant occur
		startindex1 = (int)(var_str.length() + 2);
		delimiterno = 0;
		return var(var_str.substr(start_pos, var_str.length() - start_pos));
	}

	// delimiters returned as numbers FF=1 FE=2, FD=3 etc to F9=7
	// delimiterno=int(LASTDELIMITERCHARNOPLUS1-var_str[end_pos]);
	delimiterno = int(*_RM_) - int(var_str[end_pos]) + 1;

	// point AFTER the found separator or TWO after the length of the string (TODO shouldnt this
	// be one??/bug in AREV) wont work if string is the maximum string length but that cant
	// occur
	startindex1 = int(end_pos + 2);

	// extract and return the substr as well
	return var(var_str.substr(start_pos, end_pos - start_pos));
}

//////
// SUM
/////

var var::sum() const
{
	THISIS("var var::sum()")
	THISISSTRING()

	// add up all number regardless of separator level (multilevel)
	var result = 0;
	var start = 0;
	var bit, term, xx;
	while (true)
	{
		// bit=(*this).remove(start, term);
		bit = (*this).substr2(start, term);
		if (bit.length())
			result += bit;
		if (not term)
			break;
	}
	return result;
}

var var::sum(const var& sepchar) const
{
	THISIS("var var::sum(const var& sepchar) const")
	THISISSTRING()

	var result = 0;
	int nn = this->dcount(sepchar);

	// static var allsepchars=_STM_ _TM_ _SM_ _VM_ _FM_ _RM_;
	// var sepcharn=allsepchars.index(sepchar);
	// if (!sepcharn) return var1*var2;

	// TODO make this faster using remove or index?
	var temp;
	for (int ii = 1; ii <= nn; ++ii)
	{
		temp = (*this).field(sepchar, ii);
		if (temp.isnum())
			result += temp;
		else
			result += (*this).sum(temp);
	}
	return result;
}

} // namespace exodus
