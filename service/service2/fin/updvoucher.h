//#ifndef EXODUSDLFUNC_UPDVOUCHER_H
#define EXODUSDLFUNC_UPDVOUCHER_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_updvoucher;

//a member function with the right arguments, returning a var or void
var updvoucher(in mode0, io voucher, io vouchercode, io allocs)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_updvoucher.pmemberfunction_==NULL)
  efb_updvoucher.init("updvoucher","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io,io,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_updvoucher.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_updvoucher.pmemberfunction_)))
  (mode0,voucher,vouchercode,allocs);

}
//#endif
