#ifndef EXODUSDLFUNC_TESTD2_H
#define EXODUSDLFUNC_TESTD2_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_testd2;

//a member function with the right arguments, returning a var
var testd2()
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_testd2.pmemberfunction_==NULL)
  efb_testd2.init("testd2","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)();

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_testd2.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_testd2.pmemberfunction_)))
  ();

}
#endif
