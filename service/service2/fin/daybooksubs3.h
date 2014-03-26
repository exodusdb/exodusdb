#ifndef EXODUSDLFUNC_DAYBOOKSUBS3_H
#define EXODUSDLFUNC_DAYBOOKSUBS3_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_daybooksubs3;

//a member function with the right arguments, returning a var
var daybooksubs3(in mode)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_daybooksubs3.pmemberfunction_==NULL)
  efb_daybooksubs3.init("daybooksubs3","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_daybooksubs3.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_daybooksubs3.pmemberfunction_)))
  (mode);

}
#endif
