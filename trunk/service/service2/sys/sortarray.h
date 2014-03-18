#ifndef EXODUSDLFUNC_SORTARRAY_H
#define EXODUSDLFUNC_SORTARRAY_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_sortarray;

//a member function with the right arguments, returning a var
var sortarray(io array, in fns, in order="A", in justification="L")
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_sortarray.pmemberfunction_==NULL)
  efb_sortarray.init("sortarray","exodusprogrambasecreatedelete",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,in,in,in);

 //call the shared library object main function with the right args, returning a var
 return CALLMEMBERFUNCTION(*(efb_sortarray.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_sortarray.pmemberfunction_)))
  (array,fns,order,justification);

}
#endif
