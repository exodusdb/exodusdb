#ifndef EXODUSDLFUNC_NEXTKEY_H
#define EXODUSDLFUNC_NEXTKEY_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_nextkey;

//a member function with the right arguments, returning a var
var nextkey(in mode, in previous=var(""))
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_nextkey.pmemberfunction_==NULL)
  efb_nextkey.init("nextkey","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_nextkey.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_nextkey.pmemberfunction_)))
  (mode,previous);

}
#endif
