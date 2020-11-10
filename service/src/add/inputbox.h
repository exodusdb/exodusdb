//generated by exodus "compile inputbox"
//#ifndef EXODUSDLFUNC_INPUTBOX_H
#define EXODUSDLFUNC_INPUTBOX_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_inputbox;
class efb_inputbox : public ExodusFunctorBase
{
public:

efb_inputbox(MvEnvironment& mv) : ExodusFunctorBase("inputbox", "exodusprogrambasecreatedelete_", mv) {}

efb_inputbox& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in msg, in maxlen, in show, in allowablechars, io data, in escx)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,io,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_inputbox.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_inputbox.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (msg,maxlen,show,allowablechars,data,escx);

}

var operator() () {
 var msg_in;
 var maxlen_in;
 var show_in;
 var allowablechars_in;
 var data_io;
 var escx_in;
 return operator()(msg_in, maxlen_in, show_in, allowablechars_in, data_io, escx_in);
}

var operator() (in msg) {
 var maxlen_in;
 var show_in;
 var allowablechars_in;
 var data_io;
 var escx_in;
 return operator()(msg, maxlen_in, show_in, allowablechars_in, data_io, escx_in);
}

var operator() (in msg, in maxlen) {
 var show_in;
 var allowablechars_in;
 var data_io;
 var escx_in;
 return operator()(msg, maxlen, show_in, allowablechars_in, data_io, escx_in);
}

var operator() (in msg, in maxlen, in show) {
 var allowablechars_in;
 var data_io;
 var escx_in;
 return operator()(msg, maxlen, show, allowablechars_in, data_io, escx_in);
}

var operator() (in msg, in maxlen, in show, in allowablechars) {
 var data_io;
 var escx_in;
 return operator()(msg, maxlen, show, allowablechars, data_io, escx_in);
}

var operator() (in msg, in maxlen, in show, in allowablechars, io data) {
 var escx_in;
 return operator()(msg, maxlen, show, allowablechars, data, escx_in);
}

};
efb_inputbox inputbox{mv};
//#endif
