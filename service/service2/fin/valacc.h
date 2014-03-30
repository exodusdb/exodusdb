#ifndef EXODUSDLFUNC_VALACC_H
#define EXODUSDLFUNC_VALACC_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_valacc;

//a member function with the right arguments, returning a var or void
var valacc(io mode, io is, io isorig, in validaccs, io msg, in acctype0)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_valacc.pmemberfunction_==NULL)
  efb_valacc.init("valacc","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,io,io,in,io,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_valacc.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_valacc.pmemberfunction_)))
  (mode,is,isorig,validaccs,msg,acctype0);

}
#endif
