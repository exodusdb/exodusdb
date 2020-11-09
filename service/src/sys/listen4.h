//generated by exodus "compile listen4"
//#ifndef EXODUSDLFUNC_LISTEN4_H
#define EXODUSDLFUNC_LISTEN4_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_listen4;
class efb_listen4 : public ExodusFunctorBase
{
public:

efb_listen4(MvEnvironment& mv) : ExodusFunctorBase("listen4", "exodusprogrambasecreatedelete_", mv) {}

efb_listen4& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in msgno, io msg, in params0="")
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_listen4.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_listen4.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (msgno,msg,params0);

}

var operator() () {
 var msgno_in;
 var msg_io;
 var params0_in = "";
 return operator()(msgno_in, msg_io, params0_in);
}

var operator() (in msgno) {
 var msg_io;
 var params0_in = "";
 return operator()(msgno, msg_io, params0_in);
}

};
efb_listen4 listen4{mv};
//#endif
