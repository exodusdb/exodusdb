//generated by exodus "compile ../add/decide"
//#ifndef EXODUSDLFUNC_DECIDE_H
#define EXODUSDLFUNC_DECIDE_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_decide;
class efb_decide : public ExodusFunctorBase
{
public:

efb_decide(MvEnvironment& mv) : ExodusFunctorBase("decide", "exodusprogrambasecreatedelete_", mv) {}

efb_decide& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in question0, in options0, io reply)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_decide.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_decide.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (question0,options0,reply);

}

var operator() () {
 var question0_in;
 var options0_in;
 var reply_io;
 return operator()(question0_in, options0_in, reply_io);
}

var operator() (in question0) {
 var options0_in;
 var reply_io;
 return operator()(question0, options0_in, reply_io);
}

var operator() (in question0, in options0) {
 var reply_io;
 return operator()(question0, options0, reply_io);
}

};
efb_decide decide{mv};
//#endif
