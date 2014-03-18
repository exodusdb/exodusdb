#ifndef EXODUSDLFUNC_UPDVOUCHER2_H
#define EXODUSDLFUNC_UPDVOUCHER2_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_updvoucher2;

//a member function with the right arguments, returning a var
var updvoucher2(io mode, io voucher, io vouchercode, io allocs, in username0="")
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_updvoucher2.pmemberfunction_==NULL)
  efb_updvoucher2.init("updvoucher2","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,io,io,io,in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_updvoucher2.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_updvoucher2.pmemberfunction_)))
  (mode,voucher,vouchercode,allocs,username0);

}
#endif
