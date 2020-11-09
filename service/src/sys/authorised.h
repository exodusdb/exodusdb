//generated by exodus "compile authorised"
//#ifndef EXODUSDLFUNC_AUTHORISED_H
#define EXODUSDLFUNC_AUTHORISED_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_authorised;
class efb_authorised : public ExodusFunctorBase
{
public:

efb_authorised(MvEnvironment& mv) : ExodusFunctorBase("authorised", "exodusprogrambasecreatedelete_", mv) {}

efb_authorised& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in task0, out msg, in defaultlock="", in username0="")
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,out,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_authorised.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_authorised.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (task0,msg,defaultlock,username0);

}

var operator() () {
 var task0_in;
 var msg_out;
 var defaultlock_in = "";
 var username0_in = "";
 return operator()(task0_in, msg_out, defaultlock_in, username0_in);
}

var operator() (in task0) {
 var msg_out;
 var defaultlock_in = "";
 var username0_in = "";
 return operator()(task0, msg_out, defaultlock_in, username0_in);
}

};
efb_authorised authorised{mv};
//#endif
