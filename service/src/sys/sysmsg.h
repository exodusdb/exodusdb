//generated by exodus "compile sysmsg"
//#ifndef EXODUSDLFUNC_SYSMSG_H
#define EXODUSDLFUNC_SYSMSG_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_sysmsg;
class efb_sysmsg : public ExodusFunctorBase
{
public:

efb_sysmsg(MvEnvironment& mv) : ExodusFunctorBase("sysmsg", "exodusprogrambasecreatedelete_", mv) {}

efb_sysmsg& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in msg0, in subject0="", in username0="")
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_sysmsg.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_sysmsg.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (msg0,subject0,username0);

}

var operator() () {
 var msg0_in;
 var subject0_in = "";
 var username0_in = "";
 return operator()(msg0_in, subject0_in, username0_in);
}

};
efb_sysmsg sysmsg{mv};
//#endif
