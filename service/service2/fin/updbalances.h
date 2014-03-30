//#ifndef EXODUSDLFUNC_UPDBALANCES_H
#define EXODUSDLFUNC_UPDBALANCES_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_updbalances;

//a member function with the right arguments, returning a var or void
var updbalances(in balancesfile, in balanceskey, in deleting, in balanceperiod, in amount, in baseamount, in unused, in basecurrency, in vouchertype)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_updbalances.pmemberfunction_==NULL)
  efb_updbalances.init("updbalances","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,in,in,in,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_updbalances.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_updbalances.pmemberfunction_)))
  (balancesfile,balanceskey,deleting,balanceperiod,amount,baseamount,unused,basecurrency,vouchertype);

}
//#endif
