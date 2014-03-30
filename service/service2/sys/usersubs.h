#ifndef EXODUSDLFUNC_USERSUBS_H
#define EXODUSDLFUNC_USERSUBS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_usersubs;

//a member function with the right arguments, returning a var or void
var usersubs(in mode)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_usersubs.pmemberfunction_==NULL)
  efb_usersubs.init("usersubs","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_usersubs.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_usersubs.pmemberfunction_)))
  (mode);

}
#endif
