#ifndef EXODUSDLFUNC_JOBANALYSIS_H
#define EXODUSDLFUNC_JOBANALYSIS_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_jobanalysis;

//a member function with the right arguments, returning a var
var jobanalysis(in mode0)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_jobanalysis.pmemberfunction_==NULL)
  efb_jobanalysis.init("jobanalysis","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_jobanalysis.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_jobanalysis.pmemberfunction_)))
  (mode0);

}
#endif
