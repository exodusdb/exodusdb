#ifndef EXODUSDLFUNC_GETAGP_H
#define EXODUSDLFUNC_GETAGP_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_getagp;

//a member function with the right arguments, returning a var or void
var getagp(in agpparams, in period0)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_getagp.pmemberfunction_==NULL)
  efb_getagp.init("getagp","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_getagp.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_getagp.pmemberfunction_)))
  (agpparams,period0);

}
#endif
