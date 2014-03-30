#ifndef EXODUSDLFUNC_UNLOCKALL_H
#define EXODUSDLFUNC_UNLOCKALL_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_unlockall;

//a member function with the right arguments, returning a var or void
var unlockall(io locklist)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_unlockall.pmemberfunction_==NULL)
  efb_unlockall.init("unlockall","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_unlockall.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_unlockall.pmemberfunction_)))
  (locklist);

}
#endif
