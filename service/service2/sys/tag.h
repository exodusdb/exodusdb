//#ifndef EXODUSDLFUNC_TAG_H
#define EXODUSDLFUNC_TAG_H

//a member variable/object to cache a pointer/object for the shared library function
ExodusFunctorBase efb_tag;

//a member function with the right arguments, returning a var or void
var tag(in tag, in text)
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 if (efb_tag.pmemberfunction_==NULL)
  efb_tag.init("tag","exodusprogrambasecreatedelete_",mv);

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 return CALLMEMBERFUNCTION(*(efb_tag.pobject_),
 ((pExodusProgramBaseMemberFunction) (efb_tag.pmemberfunction_)))
  (tag,text);

}
//#endif
