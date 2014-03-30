#ifndef EXODUSDLFUNC_UPDSECINDEX_H
#define EXODUSDLFUNC_UPDSECINDEX_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_updsecindex;

//a member function with the right arguments, returning a var or void
var updsecindex(in mode, in filename, in keys, in fieldnames, in oldvalues, in newvalues=, io valid, io msg)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_updsecindex.pmemberfunction_==NULL)
  efb_updsecindex.init("updsecindex","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,in,in,io,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_updsecindex.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_updsecindex.pmemberfunction_)))
  (mode,filename,keys,fieldnames,oldvalues,newvalues,valid,msg);

}
#endif
