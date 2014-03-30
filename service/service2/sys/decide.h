//#ifndef EXODUSDLFUNC_DECIDE_H
#define EXODUSDLFUNC_DECIDE_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_decide;

//a member function with the right arguments, returning a var or void
var decide(in question0, in options0, io reply)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_decide.pmemberfunction_==NULL)
  efb_decide.init("decide","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_decide.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_decide.pmemberfunction_)))
  (question0,options0,reply);

}
//#endif
