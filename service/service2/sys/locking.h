//#ifndef EXODUSDLFUNC_LOCKING_H
#define EXODUSDLFUNC_LOCKING_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_locking;

//a member function with the right arguments, returning a var or void
var locking(in mode, in lockfilename, in lockkey, in lockdesc0, io locklist, int ntries, out msg)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_locking.pmemberfunction_==NULL)
  efb_locking.init("locking","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,io,int,out);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_locking.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_locking.pmemberfunction_)))
  (mode,lockfilename,lockkey,lockdesc0,locklist,ntries,msg);

}
//#endif
