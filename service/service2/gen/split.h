#ifndef EXODUSDLFUNC_SPLIT_H
#define EXODUSDLFUNC_SPLIT_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_split;

//a member function with the right arguments, returning a var
var split(in input0, out unitx )
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_split.pmemberfunction_==NULL)
  efb_split.init("split","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,out);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_split.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_split.pmemberfunction_)))
  (input0,unitx);

}
#endif
