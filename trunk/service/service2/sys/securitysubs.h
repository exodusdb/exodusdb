#ifndef EXODUSDLFUNC_SECURITYSUBS_H
#define EXODUSDLFUNC_SECURITYSUBS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_securitysubs;

//a member function with the right arguments, returning a var
var securitysubs(in mode)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_securitysubs.pmemberfunction_==NULL)
  efb_securitysubs.init("securitysubs","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_securitysubs.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_securitysubs.pmemberfunction_)))
  (mode);

}
#endif
