//#ifndef EXODUSDLFUNC_INITGENERAL_H
#define EXODUSDLFUNC_INITGENERAL_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_initgeneral;

//a member function with the right arguments, returning a var or void
var initgeneral()
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_initgeneral.pmemberfunction_==NULL)
  efb_initgeneral.init("initgeneral","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_initgeneral.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_initgeneral.pmemberfunction_)))
  ();

}
//#endif
