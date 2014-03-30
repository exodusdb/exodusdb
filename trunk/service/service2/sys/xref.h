#ifndef EXODUSDLFUNC_XREF_H
#define EXODUSDLFUNC_XREF_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_xref;

//a member function with the right arguments, returning a var or void
var xref(in instr, in sepchars, in stoplist, in mode)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_xref.pmemberfunction_==NULL)
  efb_xref.init("xref","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_xref.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_xref.pmemberfunction_)))
  (instr,sepchars,stoplist,mode);

}
#endif
