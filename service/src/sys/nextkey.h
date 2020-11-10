//generated by exodus "compile nextkey"
//#ifndef EXODUSDLFUNC_NEXTKEY_H
#define EXODUSDLFUNC_NEXTKEY_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_nextkey;
class efb_nextkey : public ExodusFunctorBase
{
public:

efb_nextkey(MvEnvironment& mv) : ExodusFunctorBase("nextkey", "exodusprogrambasecreatedelete_", mv) {}

efb_nextkey& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in mode, in previous0="")
{

 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 //first time link to the shared lib and create/cache an object from it
 //passing current standard variables in mv
 //if (efb_getlang.pmemberfunction_==NULL)
 // efb_getlang.init("getlang","exodusprogrambasecreatedelete_",mv);
 if (this->pmemberfunction_==NULL)
  this->init();

 //define a function type (pExodusProgramBaseMemberFunction)
 //that can call the shared library object member function
 //with the right arguments and returning a var or void
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_nextkey.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_nextkey.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (mode,previous0);

}

var operator() () {
 var mode_in;
 var previous0_in = "";
 return operator()(mode_in, previous0_in);
}

};
efb_nextkey nextkey{mv};
//#endif
