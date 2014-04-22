//#ifndef EXODUSDLFUNC_CONVCSV_H
#define EXODUSDLFUNC_CONVCSV_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_convcsv;

//a member function with the right arguments, returning a var or void
var convcsv()
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_convcsv.pmemberfunction_==NULL)
  efb_convcsv.init("convcsv","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_convcsv.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_convcsv.pmemberfunction_)))
  ();

}
//#endif
