#ifndef EXODUSDLFUNC_ADDCENT_H
#define EXODUSDLFUNC_ADDCENT_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_addcent;

//a member function with the right arguments, returning a var
var addcent(in type)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_addcent.pmemberfunction_==NULL)
  efb_addcent.init("addcent","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_addcent.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_addcent.pmemberfunction_)))
  (type);

}
#endif
