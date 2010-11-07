#include <stdio.h>
#include <string.h>
//#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FM_ '\xFE'
#define VM_ '\xFD'
#define SM_ '\xFC'
//#define FM_ 'x'
//#define VM_ 'y'
//#define SM_ 'z'

void extract(char * instring, int inlength, int fieldno, int valueno, int subvalueno, int* outstart, int* outlength)
{
	int start_pos;
	int fieldn2;
	int field_end_pos;
	int valuen2;
	int value_end_pos;
	int subvaluen2;
	int subvalue_end_pos;

//	assert(assigned());
	
	//any negatives at all returns "" but since this is unexpected
	//do it inline instead of special case to avoid wasting time
	//if (fieldno<0||valueno<0||subvalueno<0) return;
	
	//FIND FIELD

	//default to returning nothing
	*outstart=0;
	*outlength=0;

	//zero means all, negative return ""
	if (fieldno<=0)
	{
		if (fieldno<0) return;
		if (valueno||subvalueno) fieldno=1; else
		{
			*outlength=inlength;
			return;
		}
	}

	//find the starting position of the field or return ""
	start_pos=0;
	fieldn2=1;

	while (fieldn2<fieldno)
	{
		for (;start_pos<inlength && instring[start_pos] != FM_ ; start_pos++){};
		//past of of string?
		if (start_pos>=inlength) return;
		start_pos++;
		fieldn2++;
	}

	//find the end of the field (or one after the end of the string)
	for (field_end_pos=start_pos;field_end_pos<inlength&&instring[field_end_pos]!=FM_;field_end_pos++){};

	
	//FIND VALUE
	
	//zero means all, negative return ""
	if (valueno<=0)
	{
		if (valueno<0) return;
		if (subvalueno) valueno=1; else
		{
			*outstart=start_pos;
			*outlength=field_end_pos-start_pos;
			return;
		}
	}
				
	//find the starting position of the value or return ""
	//using start_pos and end_pos of 
	valuen2=1;
	while (valuen2<valueno)
	{
		for (start_pos=start_pos;start_pos<inlength&&instring[start_pos]!=VM_;start_pos++){};
		//past end of string?
		if (start_pos>=inlength) return;
		start_pos++;
		//past end of field?
		if (start_pos>field_end_pos) return;
		valuen2++;
	}
	
	//find the end of the value (or string)
	for (value_end_pos=start_pos;value_end_pos<field_end_pos&&instring[value_end_pos]!=VM_;value_end_pos++){
	}


	//FIND SUBVALUE
	
	//zero means all, negative means ""
	if (subvalueno<=0)
	{
		if (subvalueno<0) return;
		*outstart=start_pos;
		*outlength=value_end_pos-start_pos;
	}
				
	//find the starting position of the subvalue or return ""
	//using start_pos and end_pos of 
	subvaluen2=1;
	while (subvaluen2<subvalueno)
	{
		for (start_pos=start_pos;start_pos<field_end_pos&&instring[start_pos]!=SM_;start_pos++){};
		//past end of string?
		if (start_pos>=inlength) return;
		start_pos++;
		//past end of value?
		if (start_pos>value_end_pos);
		subvaluen2++;
	}
	
	//find the end of the subvalue (or string)
	for (subvalue_end_pos=start_pos;subvalue_end_pos<value_end_pos&&instring[subvalue_end_pos]!=SM_;subvalue_end_pos++){};
	if (subvalue_end_pos>=value_end_pos)
	{
		*outstart=start_pos;
		*outlength=value_end_pos-start_pos;
		return;
	}						
	*outstart=start_pos;
	*outlength=subvalue_end_pos-start_pos;
	return;

}

#ifdef __cplusplus
}//extern "C"
#endif
