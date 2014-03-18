#ifndef EXODUSDLFUNC_VALIDCODE3_H
#define EXODUSDLFUNC_VALIDCODE3_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_validcode3;

//a member function with the right arguments, returning a var
var validcode3(in marketcodex, in suppliercodex, in vehiclecodex, io vehicles, out msg)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_validcode3.pmemberfunction_==NULL)
  efb_validcode3.init("validcode3","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,io,out);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_validcode3.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_validcode3.pmemberfunction_)))
  (marketcodex,suppliercodex,vehiclecodex,vehicles,msg);

}
#endif
