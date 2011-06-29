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

//C4530: C++ exception handler used, but unwind semantics are not enabled. 
#pragma warning (disable: 4530)

#include <cstring> //for strlen strstr

#include <exodus/mvimpl.h>
#include <exodus/mv.h>
#include <exodus/mvexceptions.h>

namespace exodus
{

//includes dim::matparse

//and var::field,field2,locate,extract,erase,replace,insert,substr,splice,remove

///////
//PARSE
///////

//var.matparse(dimarray2)
var dim::parse(const var& var1)
{
	//THISIS(L"var dim::parse(const var& var1)")
	//ISSTRING(var1)

	(*this).redim(var1.count(FM_)+1);

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldno=1;
	for (;;)
	{
		std::wstring::size_type next_pos;
		next_pos=var1.var_mvstr.find(FM_,start_pos);

		data_[fieldno]=var1.var_mvstr.substr(start_pos,next_pos-start_pos);

		//past of of string?
		if (next_pos==std::wstring::npos)
			break;

		start_pos=next_pos+1;
		fieldno++;
	}

	return fieldno;
}

///////////////////////////////////////////
//FIELD
///////////////////////////////////////////

var var::field2(const var& sep, const int fieldno, const int nfields) const
{
	if (fieldno >= 0)
		return field(sep, fieldno, nfields);

	return field(sep, count(sep) + 1 + fieldno + 1, nfields);
}

//FIELD(x,y,z)
//var.field(substr,fieldno,nfields)
var var::field(const var& substrx,const int fieldnx,const int nfieldsx) const
{
	THISIS(L"var var::field(const var& substrx,const int fieldnx,const int nfieldsx) const")
	THISISSTRING()
	ISSTRING(substrx)

	std::wstring substr=substrx.var_mvstr;
	if (substr==L"")
		return var(L"");

	int fieldno=fieldnx>0?fieldnx:1;
	int nfields=nfieldsx>0?nfieldsx:1;

	//FIND FIELD

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldn2=1;
	while (fieldn2<fieldno)
	{
		start_pos=var_mvstr.find(substr,start_pos);
		//past of of string?
		if (start_pos==std::wstring::npos)
			return var(L"");
		start_pos++;
		fieldn2++;
	}

	//find the end of the field (or string)
	std::wstring::size_type end_pos=start_pos;
	int pastfieldn=fieldno+nfields;
	while (fieldn2<pastfieldn)
	{
		end_pos=var_mvstr.find(substr,end_pos);
		//past of of string?
		if (end_pos==std::wstring::npos)
		{
			return var(var_mvstr.substr(start_pos,var_mvstr.length()-start_pos));
		}
		end_pos++;
		fieldn2++;
	}
	return var(var_mvstr.substr(start_pos,end_pos-start_pos-1));
}

///////////////////////////////////////////
//FIELD
///////////////////////////////////////////

//FIELDSTORE(x,y,z)
//var.fieldstore(substr,fieldno,nfields,replacement)
var var::fieldstore(const var& sepchar,const int fieldnx,const int nfieldsx, const var& replacementx) const
{
	THISIS(L"var var::fieldstore(const var& sepchar,const int fieldnx,const int nfieldsx, const var& replacementx) const")
	THISISDEFINED()

		var newmv=*this;
	return newmv.fieldstorer(sepchar,fieldnx,nfieldsx,replacementx);
}

var& var::fieldstorer(const var& sepchar0,const int fieldnx,const int nfieldsx, const var& replacementx)
{
	THISIS(L"var& var::fieldstorer(const var& sepchar0,const int fieldnx,const int nfieldsx, const var& replacementx)")
	THISISSTRING()
	ISSTRING(sepchar0)

	std::wstring sepchar=sepchar0.var_mvstr;
	if (sepchar==L"")
	{
		*this=L"";
		return *this;
	}

	int fieldno=fieldnx>0?fieldnx:1;

	int nfields=nfieldsx>0?nfieldsx:-nfieldsx;

	//pad replacement if required
	var replacement;
	if (nfieldsx>=0)
	{
		int nreplacementfields=replacementx.count(sepchar0)+1;
		//pad to correct number of fields
		if (nreplacementfields<nfields)
		{
			replacement=replacementx;
			for (;nreplacementfields<nfields;nreplacementfields++)
				replacement^=sepchar;
		}
		else if (nfieldsx&&nreplacementfields>nfields)
		{
			replacement=replacementx.field(sepchar,1,nfields);
		}
		else replacement=replacementx;
	}
	else
	{
		replacement=replacementx;
	}

	//FIND FIELD

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldn2=1;
	while (fieldn2<fieldno)
	{
		start_pos=var_mvstr.find(sepchar,start_pos);
		//past of of string?
		if (start_pos==std::wstring::npos)
		{
			do
			{
				var_mvstr+=sepchar;
				fieldn2++;
			} while (fieldn2<fieldno);
			var_mvstr+=replacement.var_mvstr;
			return *this;
		}
		start_pos++;
		fieldn2++;
	}

	//find the end of the field (or string)
	std::wstring::size_type end_pos=start_pos;
	int pastfieldn=fieldno+nfields;
	while (fieldn2<pastfieldn)
	{
		end_pos=var_mvstr.find(sepchar,end_pos);
		//past of of string?
		if (end_pos==std::wstring::npos)
		{
			var_mvstr.replace(start_pos, std::wstring::npos, replacement.var_mvstr);
			return *this;
		}
		end_pos++;
		fieldn2++;
	}
	if (end_pos==start_pos)
	{
		if (nfields==0) replacement.var_mvstr+=sepchar;
		var_mvstr.insert(start_pos,replacement.var_mvstr);
	}
	else
		var_mvstr.replace(start_pos,end_pos-start_pos-1,replacement.var_mvstr);
	return *this;
}

///////////////////////////////////////////
//LOCATE
///////////////////////////////////////////

bool var::locate(const var& target, var& setting, const int fieldno/*=0*/,const int valueno/*=0*/) const
{
	THISIS(L"bool var::locate(const var& target, var& setting, const int fieldno/*=0*/,const int valueno/*=0*/) const")
	THISISSTRING()
	ISSTRING(target)
	ISDEFINED(setting)

	wchar_t usingchar;
	if (valueno!=0) usingchar=SM_;
	else if (fieldno!=0) usingchar=VM_;
	else usingchar=FM_;
	//if (fieldno<=0) *usingchar=FM_;
	//else if (valueno<=0) *usingchar=VM_;
	//else *usingchar=SM_;

	return locatex(target.var_mvstr,0,usingchar,setting,fieldno,valueno,0);
}

///////////////////////////////////////////
//LOCATE BY
///////////////////////////////////////////

//this version caters for the rare syntax where the order is given as a variable
bool var::locateby(const var& target,const var& ordercode, var& setting, const int fieldno/*=0*/, const int valueno/*=0*/)const
{
	THISIS(L"bool var::locateby(const var& target,const var& ordercode, var& setting, const int fieldno/*=0*/, const int valueno/*=0*/)const")
	ISSTRING(ordercode)

	return locateby(target,ordercode.toString().c_str(), setting, fieldno, valueno);
}

//specialised const wchar_t version of ordercode for speed of usual syntax where ordermode is given as string
//it avoids the conversion from string to var and back again
bool var::locateby(const var& target,const char* ordercode, var& setting, const int fieldno/*=0*/, const int valueno/*=0*/) const
{
	THISIS(L"bool var::locateby(const var& target,const char* ordercode, var& setting, const int fieldno/*=0*/, const int valueno/*=0*/) const")
	THISISSTRING()
	ISSTRING(target)
	ISDEFINED(setting)

	//TODO either make a "locatefrom" version of the above where the locate STARTS its search from the
	//last numbered subvalue (add a new parameter), value or field.
	//OR possibly modify this function to understand a negative number as "start from" instead of "within this"

	//determine locate by field, value or subvalue depending on the parameters as follows:
	//if value number is given then locate in subvalues of that value
	//if field number is given then locate in values of that field
	//otherwise locate in fields of the string
	wchar_t usingchar;
	if (valueno!=0) usingchar=SM_;
	else if (fieldno!=0) usingchar=VM_;
	else usingchar=FM_;
	//if (fieldno<=0) usingchar=FM_;
	//else if (valueno<=0) usingchar=VM_;
	//else usingchar=SM_;

	char ordermode;
	if (strlen(ordercode)==0)
		ordermode=0;
	else
	{
		//locate the order code in a list of the four possible order codes
		//and throw if not found
		const char* ordercodes="ALARDLDR";
		const char* orderindex=strstr(ordercodes,ordercode);
		if (orderindex==NULL)
			throw MVException(L"locateby('" ^ var(ordercode) ^ L"') is invalid");

		//convert the memory address to the wchar_t position within the codes
		ordermode=int(orderindex-ordercodes);
		//add two and divide by two to get the order no AL=1 AR=2 DL=3 DR=4
		ordermode=(ordermode+2)>>1;
	}
	return locatex(target.var_mvstr,ordermode,usingchar,setting,fieldno,valueno,0);
}

///////////////////////////////////////////
//LOCATE USING
///////////////////////////////////////////

bool var::locateusing(const var& target,const var& usingchar) const
{
	THISIS(L"bool var::locateusing(const var& target,const var& usingchar) const")
	THISISSTRING()
	ISSTRING(target)
	ISSTRING(usingchar)
	//ISDEFINED(setting)

	var setting=L"";
	return locatex(target.var_mvstr,0,usingchar.var_mvstr.c_str()[0],setting,0,0,0);
}

bool var::locateusing(const var& target,const var& usingchar, var& setting, const int fieldno/*=0*/,const int valueno/*=0*/, const int subvalueno/*=0*/) const
{
	THISIS(L"bool var::locateusing(const var& target,const var& usingchar, var& setting, const int fieldno/*=0*/,const int valueno/*=0*/, const int subvalueno/*=0*/) const")
	THISISSTRING()
	ISSTRING(target)
	ISSTRING(usingchar)
	ISDEFINED(setting)

	return locatex(target.var_mvstr,0,usingchar.var_mvstr.c_str()[0],setting,fieldno,valueno,subvalueno);

}

//locate within extraction
bool var::locatex(const std::wstring& target,const char ordercode,const wchar_t usingchar,var& setting, int fieldno,int valueno,const int subvalueno) const
{
	//private - assume everything is defined/assigned correctly

	//any negatives at all returns ""
	//done inline since unusual
	//if (fieldno<0||valueno<0||subvalueno<0) return var(L"")

	//zero means all, negative return L""
	//if (fieldno<=0)     (but locate x<0> using VM should work too
	if (fieldno<=0)
	{
		//locate negative field number always returns false and setting 1
		if (fieldno<0)
		{
			setting=1;
			return !target.length();
		}

		if (valueno||subvalueno) fieldno=1; else
		{
			return locateat(target,(size_t) 0,var_mvstr.length(),ordercode,usingchar,setting);
		}
	}

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldn2=1;
	while (fieldn2<fieldno)
	{
		start_pos=var_mvstr.find(FM_,start_pos);
		//past of of string?
		if (start_pos==std::wstring::npos)
		{
			//if (valueno||subvalueno) setting=1;
			//else setting=fieldn2+1;
			setting=1;
			return !target.length();
		}
		start_pos++;
		fieldn2++;
	}

	//find the end of the field (or string)
	std::wstring::size_type field_end_pos;
	field_end_pos=var_mvstr.find(FM_ ,start_pos);
	if (field_end_pos==std::wstring::npos) field_end_pos=var_mvstr.length();

	//FIND VALUE

	if (start_pos>=field_end_pos)
	{
		setting=1;
		return !target.length();
	}

	//zero means all, negative return L""
	if (valueno<=0)
	{
		if (valueno<0)
		{
			setting=1;
			return !target.length();
		}
		if (subvalueno) valueno=1; else
			return locateat(target,start_pos,field_end_pos,ordercode,usingchar,setting);
	}

	//find the starting position of the value or return L""
	//using start_pos and end_pos of
	int valuen2=1;
	while (valuen2<valueno)
	{
		start_pos=var_mvstr.find(VM_ ,start_pos);
		//past end of string?
		if (start_pos==std::wstring::npos)
		{
			//if (subvalueno) setting=1;
			//else setting=valuen2+1;
			setting=1;
			return !target.length();
		}
		start_pos++;
		//past end of field?
		if (start_pos>field_end_pos)
		{
			//setting=valuen2+1;
			setting=1;
			return !target.length();
		}
		valuen2++;
	}

	//find the end of the value (or string)
	std::wstring::size_type value_end_pos;
	value_end_pos=var_mvstr.find(VM_ ,start_pos);
	if (value_end_pos==std::wstring::npos||value_end_pos>field_end_pos)
	       	value_end_pos=field_end_pos;

	//FIND SUBVALUE

	if (start_pos>=value_end_pos)
	{
		setting=1;
		return !target.length();
	}

	//zero means all, negative means ""
	if (subvalueno==0)
		return locateat(target,start_pos,value_end_pos,ordercode,usingchar,setting);
	if (subvalueno<0)
	{
		setting=1;
		return !target.length();
	}

	//find the starting position of the subvalue or return L""
	//using start_pos and end_pos of
	int subvaluen2=1;
	while (subvaluen2<subvalueno)
	{
		start_pos=var_mvstr.find(SM_ ,start_pos);
		//past end of string?
		if (start_pos==std::wstring::npos)
		{
			//setting=subvaluen2+1;
			setting=1;
			return !target.length();
		}
		start_pos++;
		//past end of value?
		if (start_pos>value_end_pos)
		{
			//setting=subvaluen2+1;
			setting=1;
			return !target.length();
		}
		subvaluen2++;
	}

	//find the end of the subvalue (or string)
	std::wstring::size_type subvalue_end_pos;
	subvalue_end_pos=var_mvstr.find(SM_ ,start_pos);
	if (subvalue_end_pos==std::wstring::npos||subvalue_end_pos>value_end_pos)
	{
	       	return locateat(target,start_pos,value_end_pos,ordercode,usingchar,setting);
	}

	return locateat(target,start_pos,subvalue_end_pos,ordercode,usingchar,setting);

}

//hardcore string locate function given a section of a string and all parameters
bool var::locateat(const std::wstring& target,size_t start_pos,size_t end_pos,const wchar_t order,const var& usingchar,var& setting) const
{
	//private - assume everything is defined/assigned correctly

	//
	//if (target.length()==0&&start_pos==end_pos)
	//if (target.length()==0)
	//{
	//	setting=1;
	//	return true;
	//}

	//arev strangeness? locate even if out of order
	//if this is the PICK standard and needs to be implemented properly it should be implemented
	//as continue to search to end instead of search twice like this
	//this will probably be switched off as unnecessary and slow behaviour for neosys applications
	if (order)
	{
		//THISIS(...)
		//ISSTRING(usingchar)
		bool result=locateat(target,start_pos,end_pos,0,usingchar,setting);
		if (result) return result;
	}

	//find null in a null field
	if (start_pos>=end_pos)
	{
		setting=1;
		return !target.length();
	}

	//for use in AR/DR;
	var value;
	var target2;
	target2=target;

	//find the starting position of the value or return L""
	//using start_pos and end_pos of
	int targetlen=(int)target.length();
	int valuen2=1;
	std::wstring usingstring=usingchar.var_mvstr;
	do
	{
		size_t nextstart_pos=var_mvstr.find(usingstring,start_pos);
		//past end of string?
		//if (nextstart_pos==string::npos)
		//{
		//	setting=valuen2+1;
		//	return false;
		//}
		//nextstart_pos++;
		//past end of field?
		if (nextstart_pos>=end_pos)
		{
			nextstart_pos=end_pos;
			switch (order)
			{
				//No order
				case L'\x00':
					if (var_mvstr.substr(start_pos,end_pos-start_pos)==target)
					{
						setting=valuen2;
						return true;
					}
					setting=valuen2+1;
					return false;
					break;

				//AL Ascending Left Justified
				case L'\x01':
					if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)>=target)
					{
						setting=valuen2;
						if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)==target)
							return true;
						else
						//arev error strangeness? empty is not greater than any target except empty
//						if (start_pos==end_pos) setting+=1;
							return false;
					}
					setting=valuen2+1;
					return false;
					break;

				//AR Ascending Right Justified
				case L'\x02':
					value=var_mvstr.substr(start_pos,nextstart_pos-start_pos);
	//what is this?!
	if (!value.isnum())
	{
		setting=-999;
		return false;
	}
					if (value>=target2)
					{
						setting=valuen2;
						if (value==target2)
							return true;
						else
							return false;
					}
					setting=valuen2+1;
					return false;
					break;

				//DL Descending Left Justified
				case L'\x03':
					if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)<=target)
					{
						setting=valuen2;
						if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)==target)
							return true;
						else
							return false;
					}
					setting=valuen2+1;
					return false;
					break;

				//DR Descending Right Justified
				case L'\x04':
					value=var_mvstr.substr(start_pos,nextstart_pos-start_pos);
	//what is this?!
	if (!value.isnum())
	{
		setting=-999;
		return false;
	}
					if (value<=target2)
					{
						setting=valuen2;
						if (value==target2)
							return true;
						else
							return false;
					}
					setting=valuen2+1;
					return false;
					break;

				default:
					throw MVException(L"locateat() invalid mode " ^ var(order));
			}
		}

		//if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)==target)
		//should only test for target up to next sep
		//but arev (by accidental error?) at least checks for the whole target
		//even if the target contains the sep character
		//if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)==target)
		//(int) is to avoid warning of unsigned integer
		switch (order)
		{
			//No order
			case L'\x00':
				if (var_mvstr.substr(start_pos,targetlen)==target)
				{
					bool x=(int) (nextstart_pos-start_pos)<=targetlen;
					if (x)
					{
						setting=valuen2;
						return true;
					}
				}
				break;

			//AL Ascending Left Justified
			case L'\x01':
//				//arev strangeness? to locate a field whereever it is regardless of order even ""?
//				if (!targetlen&&nextstart_pos==start_pos) break;

				if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)>=target)
				{
					setting=valuen2;
					if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)==target)
						return true;
					else
						return false;
				}
				break;

			//DL Descending Left Justified
			case L'\x02':
//				//arev strangeness? to locate a field whereever it is regardless of order even ""?
//				if (!targetlen&&nextstart_pos==start_pos) break;

				value=var_mvstr.substr(start_pos,nextstart_pos-start_pos);
	//what is this?!
	if (!value.isnum())
	{
		setting=-999;
		return false;
	}
				if (value>=target2)
				{
					setting=valuen2;
					if (value==target2)
						return true;
					else
						return false;
				}
				break;

			//AR Ascending Right Justified
			case L'\x03':
//				//arev strangeness? to locate a field whereever it is regardless of order even ""?
//				if (!targetlen&&nextstart_pos==start_pos) break;

				if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)<=target)
				{
					setting=valuen2;
					if (var_mvstr.substr(start_pos,nextstart_pos-start_pos)==target)
						return true;
					else
						return false;
				}
				break;

			//DR Descending Right Justified
			case L'\x04':
//				//arev strangeness? to locate a field whereever it is regardless of order even ""?
//				if (!targetlen&&nextstart_pos==start_pos) break;

				value=var_mvstr.substr(start_pos,nextstart_pos-start_pos);
	//what is this?!
	if (!value.isnum())
	{
		setting=-999;
		return false;
	}
				if (value<=target2)
				{
					setting=valuen2;
					if (value==target2)
						return true;
					else
						return false;
				}
				break;

			default:
				throw MVException(L"locateat() invalid order" ^ var(order));
		}
		//skip over any sep character
		start_pos=nextstart_pos+1;
		valuen2++;
	}while (true);

}

///////////////////////////////////////////
//EXTRACT
///////////////////////////////////////////

var var::operator() (int fieldno, int valueno, int subvalueno) const
{
	return extract(fieldno,valueno,subvalueno);
}

/* sadly no way to get a different operator() function called when on the left hand side of assign
   therefore not possible to create syntax like "xyz(1,2)="xx";" to REPLACE fields, values and subvalues
   so operator() is defined only for the more common xyz=extract(1,2,3); i.e. on the right hand side.
   http://codepad.org/MzzhlRkb
*/

/*
var& var::operator() (int fieldno, int valueno, int subvalueno)
{
	return extract(fieldno,valueno,subvalueno);
}
*/

//extract int int int
//NB declared with value=0 and subvalue=0
var var::extract(const int argfieldn, const int argvaluen, const int argsubvaluen) const
{
	THISIS(L"var var::extract(const int argfieldn, const int argvaluen, const int argsubvaluen) const")
	THISISSTRING()

	//any negatives at all returns ""
	//done inline since unusual
	//if (fieldno<0||valueno<0||subvalueno<0) return var(L"")

	//FIND FIELD

	int fieldno=argfieldn;
	int valueno=argvaluen;
	int subvalueno=argsubvaluen;
	//zero means all, negative return L""
	if (fieldno<=0)
	{
		if (fieldno<0)
			return var(L"");
		if (valueno||subvalueno)
			fieldno=1;
		else
			return var(var_mvstr);
	}

	//find the starting position of the field or return L""
	std::wstring::size_type start_pos=0;
	int fieldn2=1;
	while (fieldn2<fieldno)
	{
		start_pos=var_mvstr.find(FM_,start_pos);
		//past of of string?
		if (start_pos==std::wstring::npos)
			return var(L"");
		start_pos++;
		fieldn2++;
	}

	//find the end of the field (or string)
	std::wstring::size_type field_end_pos;
	field_end_pos=var_mvstr.find(FM_ ,start_pos);
	if (field_end_pos==std::wstring::npos) field_end_pos=var_mvstr.length();

	//FIND VALUE

	//zero means all, negative return L""
	if (valueno<=0)
	{
		if (valueno<0)
			return var(L"");
		if (subvalueno)
			valueno=1;
		else
			return var(var_mvstr.substr(start_pos,field_end_pos-start_pos));
	}

	//find the starting position of the value or return L""
	//using start_pos and end_pos of
	int valuen2=1;
	while (valuen2<valueno)
	{
		start_pos=var_mvstr.find(VM_ ,start_pos);
		//past end of string?
		if (start_pos==std::wstring::npos)
			return var(L"");
		start_pos++;
		//past end of field?
		if (start_pos>field_end_pos)
			return var(L"");
		valuen2++;
	}

	//find the end of the value (or string)
	std::wstring::size_type value_end_pos;
	value_end_pos=var_mvstr.find(VM_ ,start_pos);
	if (value_end_pos==std::wstring::npos||value_end_pos>field_end_pos)
	       	value_end_pos=field_end_pos;

	//FIND SUBVALUE

	//zero means all, negative means ""
	if (subvalueno==0)
		return var(var_mvstr.substr(start_pos,value_end_pos-start_pos));
	if (subvalueno<0)
		return var(L"");

	//find the starting position of the subvalue or return L""
	//using start_pos and end_pos of
	int subvaluen2=1;
	while (subvaluen2<subvalueno)
	{
		start_pos=var_mvstr.find(SM_ ,start_pos);
		//past end of string?
		if (start_pos==std::wstring::npos)
			return var(L"");
		start_pos++;
		//past end of value?
		if (start_pos>value_end_pos)
			return var(L"");
		subvaluen2++;
	}

	//find the end of the subvalue (or string)
	std::wstring::size_type subvalue_end_pos;
	subvalue_end_pos=var_mvstr.find(SM_ ,start_pos);
	if (subvalue_end_pos==std::wstring::npos||subvalue_end_pos>value_end_pos)
	       	return var(var_mvstr.substr(start_pos,value_end_pos-start_pos));

	return var(var_mvstr.substr(start_pos,subvalue_end_pos-start_pos));

}

/////////////////////////////////////////////////
//ERASE int fieldno, int valueno=0, int subvalueno=0
/////////////////////////////////////////////////


var var::erase(const int fieldno,const int valueno,const int subvalueno) const
{
	THISIS(L"var var::erase(const int fieldno,const int valueno,const int subvalueno) const")
	THISISSTRING()

	var newmv=*this;
	return newmv.eraser(fieldno,valueno,subvalueno);
}

var& var::eraser(int fieldno,int valueno,int subvalueno)
{
	THISIS(L"var& var::eraser(int fieldno,int valueno,int subvalueno)")
	THISISSTRING()

	//return L"" if replacing 0,0,0
	if (fieldno==0&&valueno==0&&subvalueno==0){
		//functionmode return var(L"");//var(var1);
		//proceduremode
		var_mvstr=L"";
		var_mvtyp=1;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::wstring::size_type start_pos=0;
	std::wstring::size_type field_end_pos;

	//negative means return all
	if (fieldno<0)
	{
		return *this;
	}
	else
	{

		//find the starting position of the field or append enough FM_ to satisfy
		int fieldn2=1;
		while (fieldn2<fieldno)
		{
			start_pos=var_mvstr.find(FM_,start_pos);
			//past of of string?
			if (start_pos==std::wstring::npos)
			{
				return *this;
			}
			start_pos++;
			fieldn2++;
		}

		//find the end of the field (or string)
		if (start_pos==std::wstring::npos)
		{
			start_pos=var_mvstr.length();
			field_end_pos=start_pos;
		}
		else
		{
			field_end_pos=var_mvstr.find(FM_ ,start_pos);
			if (field_end_pos==std::wstring::npos) field_end_pos=var_mvstr.length();
		}

	}

	////////////// FIND VALUE ///////////////////
	std::wstring::size_type value_end_pos;

	//zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno<0)
	{
		return *this;
	}
	else if (valueno==0&&subvalueno==0)
	{
		if (fieldno>1) start_pos--;
		else if (field_end_pos<var_mvstr.length()) field_end_pos++;
		var_mvstr.erase(start_pos,field_end_pos-start_pos);
		return *this;

	}
	else
	{

		//find the starting position of the value or insert enough VM_ to satisfy
		int valuen2=1;
		while (valuen2<valueno)
		{
			start_pos=var_mvstr.find(VM_,start_pos);
			//past end of string or field?
			if (start_pos>=field_end_pos||start_pos==std::wstring::npos)
			{
				return *this;
			}
			start_pos++;
			valuen2++;
		}

		//find the end of the value (or string)
		value_end_pos=var_mvstr.find(VM_,start_pos);
		if (value_end_pos==std::wstring::npos||value_end_pos>field_end_pos)
			value_end_pos=field_end_pos;

	}

	////////// FIND SUBVALUE  //////////////////////
	std::wstring::size_type subvalue_end_pos;

	//zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno<0)
	{
		return *this;
	}
	else if (subvalueno==0)
	{
		if (valueno>1) start_pos--;
		else if (value_end_pos<field_end_pos) value_end_pos++;
		var_mvstr.erase(start_pos,value_end_pos-start_pos);
		return *this;

	}
	else
	{
		//find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2=1;
		while (subvaluen2<subvalueno)
		{
			start_pos=var_mvstr.find(SM_,start_pos);
			//past end of string or value
			if (start_pos>=value_end_pos||start_pos==std::wstring::npos)
			{
				return *this;
			}
			start_pos++;
			subvaluen2++;
		}

		//find the end of the subvalue (or string)
		subvalue_end_pos=var_mvstr.find(SM_,start_pos);
		if (subvalue_end_pos==std::wstring::npos||subvalue_end_pos>value_end_pos)
	       		subvalue_end_pos=value_end_pos;

	}
//wcout<<start_pos<<L" "<<subvalue_end_pos<<L" "<<subvalue_end_pos-start_pos<<endl;

	if (subvalueno>1) start_pos--;
	else if(subvalue_end_pos<value_end_pos) subvalue_end_pos++;
	var_mvstr.erase(start_pos,subvalue_end_pos-start_pos);

	return *this;

}



///////////////////////////////////////////
//REPLACE int int int var
///////////////////////////////////////////

var var::replace(const int fieldno,const int valueno,const int subvalueno,const var& replacement) const
{
	THISIS(L"var var::replace(const int fieldno,const int valueno,const int subvalueno,const var& replacement) const")
	THISISSTRING()

	return var(*this).replacer(fieldno,valueno,subvalueno,replacement);
}

var var::replace(const int fieldno,const int valueno,const var& replacement) const
{
	THISIS(L"var var::replace(const int fieldno,const int valueno,const var& replacement) const")
	THISISSTRING()

	return var(*this).replacer(fieldno,valueno,0,replacement);
}

var var::replace(const int fieldno,const var& replacement) const
{
	THISIS(L"var var::replace(const int fieldno,const var& replacement) const")
	THISISSTRING()

	return var(*this).replacer(fieldno,0,0,replacement);
}

var& var::replacer(const int fieldno,const int valueno,const var& replacement)
{
	THISIS(L"var var::replacer(const int fieldno,const int valueno,const var& replacement")
	THISISSTRING()

	return replacer(fieldno,valueno,0,replacement);
}

var& var::replacer(const int fieldno,const var& replacement)
{
	THISIS(L"var var::replacer(const int fieldno,const var& replacement)")
	THISISSTRING()

	return replacer(fieldno,0,0,replacement);
}

var& var::replacer(int fieldno,int valueno,int subvalueno,const var& replacement)
{
	THISIS(L"var& var::replacer(int fieldno,int valueno,int subvalueno,const var& replacement)")
	THISISSTRING()
	ISSTRING(replacement)

	//return whole thing if replace 0,0,0
	if (fieldno==0&&valueno==0&&subvalueno==0)
	{
		//functionmode return var(replacement);
		//proceduremode
		var_mvstr=replacement.var_mvstr;
		var_mvtyp=pimpl::MVTYPE_STR;
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::wstring::size_type start_pos=0;
	std::wstring::size_type field_end_pos;

	//negative means append
	if (fieldno<0)
	{
		//append a FM_ only if there is existing data
		if (var_mvstr.length()!=0) var_mvstr+=FM_;
		start_pos=var_mvstr.length();
		field_end_pos=start_pos;
	}
	else
	{

		//find the starting position of the field or append enough FM_ to satisfy
		int fieldn2=1;
		while (fieldn2<fieldno)
		{
			start_pos=var_mvstr.find(FM_,start_pos);
			//past of of string?
			if (start_pos==std::wstring::npos)
			{
				var_mvstr.append(fieldno-fieldn2,FM_);
				//start_pos=var_mvstr.length();
				//fieldn2=fieldno
				break;
			}
			start_pos++;
			fieldn2++;
		}

		//find the end of the field (or string)
		if (start_pos==std::wstring::npos)
		{
			start_pos=var_mvstr.length();
			field_end_pos=start_pos;
		}
		else
		{
			field_end_pos=var_mvstr.find(FM_ ,start_pos);
			if (field_end_pos==std::wstring::npos) field_end_pos=var_mvstr.length();
		}

	}

	////////////// FIND VALUE ///////////////////
	std::wstring::size_type value_end_pos;

	//zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno<0)
	{
		if (field_end_pos-start_pos>0)
		{
			var_mvstr.insert(field_end_pos,_VM_);
			field_end_pos++;
		}
		start_pos=field_end_pos;
		value_end_pos=field_end_pos;
	}
	else if (valueno==0&&subvalueno==0)
	{
		var_mvstr.replace(start_pos,field_end_pos-start_pos,replacement.var_mvstr);
		return *this;

	}
	else
	{

		//find the starting position of the value or insert enough VM_ to satisfy
		int valuen2=1;
		while (valuen2<valueno)
		{
			start_pos=var_mvstr.find(VM_,start_pos);
			//past end of string or field?
			if (start_pos>=field_end_pos||start_pos==std::wstring::npos)
			{
				start_pos=field_end_pos;
				//var_mvstr.insert(field_end_pos,std::wstring(valueno-valuen2,VM_));
				var_mvstr.insert(field_end_pos,valueno-valuen2,VM_);
				field_end_pos+=valueno-valuen2;
				start_pos=field_end_pos;
				break;
			}
			start_pos++;
			valuen2++;
		}

		//find the end of the value (or string)
		value_end_pos=var_mvstr.find(VM_,start_pos);
		if (value_end_pos==std::wstring::npos||value_end_pos>field_end_pos)
			value_end_pos=field_end_pos;

	}

	////////// FIND SUBVALUE  //////////////////////
	std::wstring::size_type subvalue_end_pos;

	//zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno<0)
	{
		if (value_end_pos-start_pos>0)
		{
			var_mvstr.insert(value_end_pos,_SM_);
			value_end_pos++;
		}
		start_pos=value_end_pos;
		subvalue_end_pos=value_end_pos;
	}
	else if (subvalueno==0)
	{
		var_mvstr.replace(start_pos,value_end_pos-start_pos,replacement.var_mvstr);
		return *this;

	}
	else
	{
		//find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2=1;
		while (subvaluen2<subvalueno)
		{
			start_pos=var_mvstr.find(SM_,start_pos);
			//past end of string or value
			if (start_pos>=value_end_pos||start_pos==std::wstring::npos)
			{
				start_pos=value_end_pos;
				//var_mvstr.insert(value_end_pos,std::wstring(subvalueno-subvaluen2,SM_));
				var_mvstr.insert(value_end_pos,subvalueno-subvaluen2,SM_);
				value_end_pos+=subvalueno-subvaluen2;
				start_pos=value_end_pos;
				break;
			}
			start_pos++;
			subvaluen2++;
		}

		//find the end of the subvalue (or string)
		subvalue_end_pos=var_mvstr.find(SM_,start_pos);
		if (subvalue_end_pos==std::wstring::npos||subvalue_end_pos>value_end_pos)
	       		subvalue_end_pos=value_end_pos;

	}
//wcout<<start_pos<<L" "<<subvalue_end_pos<<L" "<<subvalue_end_pos-start_pos<<endl;

	var_mvstr.replace(start_pos,subvalue_end_pos-start_pos,replacement.var_mvstr);

	return *this;

}


///////////////////////////////////////////
//INSERT int int int var
///////////////////////////////////////////

var var::insert(const int fieldno,const int valueno,const int subvalueno,const var& insertion) const
{
	THISIS(L"var var::insert(const int fieldno,const int valueno,const int subvalueno,const var& insertion) const")
	THISISSTRING()

	var newmv=*this;
	return newmv.inserter(fieldno,valueno,subvalueno,insertion);
}

var& var::inserter(const int fieldno,const int valueno,const var& insertion)
{
	THISIS(L"var var::inserter(const int fieldno,const int valueno,const var& insertion")
	THISISSTRING()

	return inserter(fieldno,valueno,0,insertion);
}

var& var::inserter(const int fieldno,const var& insertion)
{
	THISIS(L"var var::inserter(const int fieldno,const var& insertion")
	THISISSTRING()

	return inserter(fieldno,0,0,insertion);
}

var& var::inserter(const int fieldno,const int valueno,const int subvalueno,const var& insertion)
{
	THISIS(L"var& var::inserter(const int fieldno,const int valueno,const int subvalueno,const var& insertion)")
	THISISSTRING()
	ISSTRING(insertion)

	//return whole thing if replace 0,0,0
	if (fieldno==0&&valueno==0&&subvalueno==0)
	{
		var_mvstr.insert(0,insertion.var_mvstr+FM_);
		return *this;
	}

	/////////////   FIND FIELD  /////////////////
	std::wstring::size_type start_pos=0;
	std::wstring::size_type field_end_pos;

	int pad=false;

	//negative means append
	if (fieldno<0)
	{
		//append a FM_ only if there is existing data
		if (var_mvstr.length()!=0) var_mvstr+=FM_;
		pad=true;
		start_pos=var_mvstr.length();
		field_end_pos=start_pos;
	}
	else
	{

		//find the starting position of the field or append enough FM_ to satisfy
		int fieldn2=1;
		while (fieldn2<fieldno)
		{
			start_pos=var_mvstr.find(FM_,start_pos);
			//past of of string?
			if (start_pos==std::wstring::npos)
			{
				pad=true;
				 var_mvstr.append(fieldno-fieldn2,FM_);
				//start_pos=var_mvstr.length();
				//fieldn2=fieldno
				break;
			}
			start_pos++;
			fieldn2++;
		}

		//find the end of the field (or string)
		if (start_pos==std::wstring::npos)
		{
			start_pos=var_mvstr.length();
			field_end_pos=start_pos;
		}
		else
		{
			field_end_pos=var_mvstr.find(FM_ ,start_pos);
			if (field_end_pos==std::wstring::npos) field_end_pos=var_mvstr.length();
		}

	}

	////////////// FIND VALUE ///////////////////
	std::wstring::size_type value_end_pos;

	//zero means all, negative means append one mv ... regardless of subvalueno
	if (valueno<0)
	{
		if (field_end_pos-start_pos>0)
		{
			var_mvstr.insert(field_end_pos,_VM_);
			field_end_pos++;
		}
		start_pos=field_end_pos;
		value_end_pos=field_end_pos;
		pad=true;
	}
	else if (valueno==0&&subvalueno==0)
	{
		if (!pad) var_mvstr.insert(start_pos,_FM_);
		var_mvstr.insert(start_pos,insertion.var_mvstr);
		return *this;

	}
	else
	{

		//find the starting position of the value or insert enough VM_ to satisfy
		int valuen2=1;
		while (valuen2<valueno)
		{
			start_pos=var_mvstr.find(VM_,start_pos);
			//past end of string or field?
			if (start_pos>=field_end_pos||start_pos==std::wstring::npos)
			{
				pad=true;
				start_pos=field_end_pos;
				//var_mvstr.insert(field_end_pos,std::wstring(valueno-valuen2,VM_));
				var_mvstr.insert(field_end_pos,valueno-valuen2,VM_);
				field_end_pos+=valueno-valuen2;
				start_pos=field_end_pos;
				break;
			}
			start_pos++;
			valuen2++;
		}

		//find the end of the value (or string)
		value_end_pos=var_mvstr.find(VM_,start_pos);
		if (value_end_pos==std::wstring::npos||value_end_pos>field_end_pos)
			value_end_pos=field_end_pos;

	}

	////////// FIND SUBVALUE  //////////////////////
	std::wstring::size_type subvalue_end_pos;

	//zero means all, negative means append one sv ... regardless of subvalueno
	if (subvalueno<0)
	{
		if (value_end_pos-start_pos>0)
		{
			var_mvstr.insert(value_end_pos,_SM_);
			value_end_pos++;
		}
		start_pos=value_end_pos;
		subvalue_end_pos=value_end_pos;
		pad=true;
	}
	else if (subvalueno==0)
	{
		if (!pad&&(start_pos<field_end_pos||valueno>1)) var_mvstr.insert(start_pos,_VM_);
		var_mvstr.insert(start_pos,insertion.var_mvstr);
		return *this;

	}
	else
	{
		//find the starting position of the subvalue or insert enough SM_ to satisfy
		int subvaluen2=1;
		while (subvaluen2<subvalueno)
		{
			start_pos=var_mvstr.find(SM_,start_pos);
			//past end of string or value
			if (start_pos>=value_end_pos||start_pos==std::wstring::npos)
			{
				pad=true;
				start_pos=value_end_pos;
				//var_mvstr.insert(value_end_pos,std::wstring(subvalueno-subvaluen2,SM_));
				var_mvstr.insert(value_end_pos,subvalueno-subvaluen2,SM_);
				value_end_pos+=subvalueno-subvaluen2;
				start_pos=value_end_pos;
				break;
			}
			start_pos++;
			subvaluen2++;
		}

		//find the end of the subvalue (or string)
		subvalue_end_pos=var_mvstr.find(SM_,start_pos);
		if (subvalue_end_pos==std::wstring::npos||subvalue_end_pos>value_end_pos)
	       		subvalue_end_pos=value_end_pos;

	}

	if (!pad&&(start_pos<value_end_pos||subvalueno>1)) var_mvstr.insert(start_pos,_SM_);
	var_mvstr.insert(start_pos,insertion.var_mvstr);

	return *this;

}


////////
//SUBSTR
////////

var var::substr(const int startx) const
{
	THISIS(L"var var::substr(const int startx) const")
	THISISSTRING()

	return var(*this).substrer(startx);

}

//[x,y]
//var.s(start,length) substring
var var::substr(const int startx,const int length) const
{
	THISIS(L"var var::substr(const int startx,const int length) const")
	THISISSTRING()

	return var(*this).substrer(startx,length);

}

var& var::substrer(const int startx)
{
	THISIS(L"var& var::substrer(const int startx) const")
	THISISSTRING()

	//TODO look at using erase to speed up
	return substrer(startx,(int) var_mvstr.length());

}

//[x,y]
//var.s(start,length) substring
var& var::substrer(const int startx,const int length)
{
	THISIS(L"var& var::substrer(const int startx,const int length) const")
	THISISSTRING()

	//return L"" for ""
	int max=(int) var_mvstr.length();
	if (max==0)
	{
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_INTSTR;
		var_mvint=0;
		return *this;
	}

	int start=startx;

	//negative length means reverse the string
	if (length<=0)
	{
		if (length==0)
		{
			var_mvstr=L"";
			var_mvtyp=pimpl::MVTYPE_INTSTR;
			var_mvint=0;
			return *this;
		}

		//sya
		if (start<1)
		{
			if (start==0)
			{
				var_mvstr=L"";
				var_mvtyp=pimpl::MVTYPE_INTSTR;
				var_mvint=0;
				return *this;
			}
			start=max+start+1;
			if (start<1)
			{
				var_mvstr=L"";
				var_mvtyp=pimpl::MVTYPE_INTSTR;
				var_mvint=0;
				return *this;
			}
		}
		else if (start>max)
		{
			start=max;
		}

		int stop=start+length+1;
		if (stop<1) stop=1;

		std::wstring result=L"";
		for (int ii=start;ii>=stop;ii--)
			result+=var_mvstr[ii-1];

		var_mvstr=result;
		var_mvtyp=pimpl::MVTYPE_STR;
		return *this;

	}

	if (start<1)
	{
		if (start==0)
			start=1;
		else
		{
			start=max+start+1;
			if (start<1)
				start=1;
		}
	}
	else if (start>max)
	{
		var_mvstr=L"";
		var_mvtyp=pimpl::MVTYPE_INTSTR;
		var_mvint=0;
		return *this;
	}
	int stop=start+length;

	//wcout<<start-1<<stop-start<<endl;

	//TODO use erase for speed instead of copying whole string
	var_mvstr=var_mvstr.substr(start-1,stop-start);
	var_mvtyp=pimpl::MVTYPE_STR;

	return *this;

}

//[x]
//var.c(charno) character
var var::operator[](const int charno) const
{
	THISIS(L"var var::operator[](const int charno) const")
	THISISSTRING()

	int nchars=int(var_mvstr.length());

	//beyond end of string return ""
	//get this test out of the way first since it only has to be done later on anyway
	if (charno>nchars)
		return L"";

	//within string return the character
	//handle positive indexing first for speed on the assumption
	//that it is commoner than negative indexing
	if (charno>0)
		return var_mvstr[charno-1];

	//character 0 return the first character or "" if none
	//have to get this special case out of the way first
	//despite it being unusual
	//since it only has to be done later anyway
	if (charno==0)
	{
		if (nchars)
			return var_mvstr[0];
		else
			return L"";
	}

	//have to check this later so check it now
	if (!nchars)
		return L"";

	//convert negative index to positive index
	int charno2=nchars+charno;

	//if index is now 0 or positive then return the character
	if (charno2>=0)
		return var_mvstr[charno2];//no need for -1 here

	//otherwise so negative as to point before beginning of string
	//and rule is to return the first character in that case
	return var_mvstr[0];

}

////////
//REMOVE
////////

//returns the characters up to the next delimiter
var var::remove(var& startchar1, var& delimiterno) const
{
	THISIS(L"var var::remove(var& startchar1, var& delimiterno) const")
	THISISSTRING()
	ISNUMERIC(startchar1)
	ISDEFINED(delimiterno)

	std::wstring::size_type start_pos=startchar1.toInt()-1;

	//handle after end of string
	if (start_pos>=var_mvstr.length())
	{
		delimiterno=0;
		return L"";
	}

	//find the end of the field (or string)
	std::wstring::size_type end_pos;
	end_pos=var_mvstr.find_first_of(_RM_ _FM_ _VM_ _SM_ _TM_ _STM_ _SSTM_,start_pos);

	//past of of string?
	if (end_pos==std::wstring::npos)
	{
		startchar1=(int) (var_mvstr.length()+2);
		delimiterno=0;
		return var(var_mvstr.substr(start_pos,var_mvstr.length()-start_pos));
	}

	//delimiters returned as numbers FF=1 FE=2, FD=3 etc to F9=7
	delimiterno=int(LASTDELIMITERCHARNOPLUS1-var_mvstr[end_pos]);

	startchar1=int(end_pos+2);

	//extract and return the substr as well
	return var(var_mvstr.substr(start_pos,end_pos-start_pos));

}

//////
//SUM
/////

var var::sum() const
{
	return sum(VM);
}

var var::sum(const var& sepchar) const
{
	THISIS(L"var var::sum(const var& sepchar) const")
	THISISSTRING()

	//TODO make this work with a mixture of separator characters
	var result=L"0";
	int nn=this->dcount(sepchar);

	//static var allsepchars=_SSTM_ _STM_ _TM_ _SM_ _VM_ _FM_ _RM_;
	//var sepcharn=allsepchars.index(sepchar);
	//if (!sepcharn) return var1*var2;

	//TODO make this faster using remove or index?
	for (int ii=1;ii<=nn;++ii)
		result+= field(sepchar,ii);
	return result;
}

}//namespace exodus
