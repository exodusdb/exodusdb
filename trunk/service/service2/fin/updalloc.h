#ifndef EXODUSDLFUNC_UPDALLOC_H
#define EXODUSDLFUNC_UPDALLOC_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_updalloc;

//a member function with the right arguments, returning a var
var updalloc(io mode, io payment, in paymentcode, io allocs)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_updalloc.pmemberfunction_==NULL)
  efb_updalloc.init("updalloc","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,io,in,io);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_updalloc.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_updalloc.pmemberfunction_)))
  (mode,payment,paymentcode,allocs);

}
#endif
