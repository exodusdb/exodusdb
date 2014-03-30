//#ifndef EXODUSDLFUNC_PRODUCTIONPROXY_H
#define EXODUSDLFUNC_PRODUCTIONPROXY_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_productionproxy;

//a member function with the right arguments, returning a var or void
var productionproxy()
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_productionproxy.pmemberfunction_==NULL)
  efb_productionproxy.init("productionproxy","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_productionproxy.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_productionproxy.pmemberfunction_)))
  ();

}
//#endif
