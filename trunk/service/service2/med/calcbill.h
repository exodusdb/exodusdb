//#ifndef EXODUSDLFUNC_CALCBILL_H
#define EXODUSDLFUNC_CALCBILL_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_calcbill;

//a member function with the right arguments, returning a var or void
var calcbill(io grossbill, in size, in extrasx, in unused, io msg, in roundingx, io mult, io amounts)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_calcbill.pmemberfunction_==NULL)
  efb_calcbill.init("calcbill","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,in,in,in,io,in,io,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_calcbill.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_calcbill.pmemberfunction_)))
  (grossbill,size,extrasx,unused,msg,roundingx,mult,amounts);

}
//#endif
