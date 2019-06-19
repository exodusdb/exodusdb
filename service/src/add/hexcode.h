//generated by exodus "compile ../add/hexcode"
//#ifndef EXODUSDLFUNC_HEXCODE_H
#define EXODUSDLFUNC_HEXCODE_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_hexcode;
class efb_hexcode : public ExodusFunctorBase
{
public:

efb_hexcode(MvEnvironment& mv) : ExodusFunctorBase("hexcode", "exodusprogrambasecreatedelete_", mv) {}

efb_hexcode& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in mode, io text)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_hexcode.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_hexcode.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (mode,text);

}

var operator() () {
 var mode_in;
 var text_io;
 return operator()(mode_in, text_io);
}

var operator() (in mode) {
 var text_io;
 return operator()(mode, text_io);
}

};
efb_hexcode hexcode{mv};
//#endif