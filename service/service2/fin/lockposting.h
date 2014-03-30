#ifndef EXODUSDLFUNC_LOCKPOSTING_H
#define EXODUSDLFUNC_LOCKPOSTING_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_lockposting;

//a member function with the right arguments, returning a var or void
var lockposting(in mode, io locklist, in ntries0, io msg)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_lockposting.pmemberfunction_==NULL)
  efb_lockposting.init("lockposting","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io,in,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_lockposting.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_lockposting.pmemberfunction_)))
  (mode,locklist,ntries0,msg);

}
#endif
