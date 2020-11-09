//generated by exodus "compile listen5"
//#ifndef EXODUSDLFUNC_LISTEN5_H
#define EXODUSDLFUNC_LISTEN5_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_listen5;
class efb_listen5 : public ExodusFunctorBase
{
public:

efb_listen5(MvEnvironment& mv) : ExodusFunctorBase("listen5", "exodusprogrambasecreatedelete_", mv) {}

efb_listen5& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in request1, in request2in, in request3in, in request4in, in request5in, in request6in)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(in,in,in,in,in,in);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_listen5.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_listen5.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (request1,request2in,request3in,request4in,request5in,request6in);

}

var operator() () {
 var request1_in;
 var request2in_in;
 var request3in_in;
 var request4in_in;
 var request5in_in;
 var request6in_in;
 return operator()(request1_in, request2in_in, request3in_in, request4in_in, request5in_in, request6in_in);
}

var operator() (in request1) {
 var request2in_in;
 var request3in_in;
 var request4in_in;
 var request5in_in;
 var request6in_in;
 return operator()(request1, request2in_in, request3in_in, request4in_in, request5in_in, request6in_in);
}

var operator() (in request1, in request2in) {
 var request3in_in;
 var request4in_in;
 var request5in_in;
 var request6in_in;
 return operator()(request1, request2in, request3in_in, request4in_in, request5in_in, request6in_in);
}

var operator() (in request1, in request2in, in request3in) {
 var request4in_in;
 var request5in_in;
 var request6in_in;
 return operator()(request1, request2in, request3in, request4in_in, request5in_in, request6in_in);
}

var operator() (in request1, in request2in, in request3in, in request4in) {
 var request5in_in;
 var request6in_in;
 return operator()(request1, request2in, request3in, request4in, request5in_in, request6in_in);
}

var operator() (in request1, in request2in, in request3in, in request4in, in request5in) {
 var request6in_in;
 return operator()(request1, request2in, request3in, request4in, request5in, request6in_in);
}

};
efb_listen5 listen5{mv};
//#endif
