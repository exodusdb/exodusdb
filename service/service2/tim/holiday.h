#ifndef EXODUSDLFUNC_HOLIDAY_H
#define EXODUSDLFUNC_HOLIDAY_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_holiday;

//a member function with the right arguments, returning a var or void
var holiday(in mode, in idate0, in usercode, in user, in marketcode, in market, in agp, out holidaytype, out workdate)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_holiday.pmemberfunction_==NULL)
  efb_holiday.init("holiday","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,in,in,in,out,out);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_holiday.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_holiday.pmemberfunction_)))
  (mode,idate0,usercode,user,marketcode,market,agp,holidaytype,workdate);

}
#endif
