//generated by exodus "compile popsession"
//#ifndef EXODUSDLFUNC_POPSESSION_H
#define EXODUSDLFUNC_POPSESSION_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_popsession;
class efb_popsession : public ExodusFunctorBase
{
public:

efb_popsession(MvEnvironment& mv) : ExodusFunctorBase("popsession", "exodusprogrambasecreatedelete_", mv) {}

efb_popsession& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (io v1, io v2, io v3, io v4, io v5, io v6)
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
 typedef var (ExodusProgramBase::*pExodusProgramBaseMemberFunction)(io,io,io,io,io,io);

 //call the shared library object main function with the right args,
 // returning a var or void
 //return CALLMEMBERFUNCTION(*(efb_popsession.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_popsession.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (v1,v2,v3,v4,v5,v6);

}

var operator() () {
 var v1_io;
 var v2_io;
 var v3_io;
 var v4_io;
 var v5_io;
 var v6_io;
 return operator()(v1_io, v2_io, v3_io, v4_io, v5_io, v6_io);
}

var operator() (io v1) {
 var v2_io;
 var v3_io;
 var v4_io;
 var v5_io;
 var v6_io;
 return operator()(v1, v2_io, v3_io, v4_io, v5_io, v6_io);
}

var operator() (io v1, io v2) {
 var v3_io;
 var v4_io;
 var v5_io;
 var v6_io;
 return operator()(v1, v2, v3_io, v4_io, v5_io, v6_io);
}

var operator() (io v1, io v2, io v3) {
 var v4_io;
 var v5_io;
 var v6_io;
 return operator()(v1, v2, v3, v4_io, v5_io, v6_io);
}

var operator() (io v1, io v2, io v3, io v4) {
 var v5_io;
 var v6_io;
 return operator()(v1, v2, v3, v4, v5_io, v6_io);
}

var operator() (io v1, io v2, io v3, io v4, io v5) {
 var v6_io;
 return operator()(v1, v2, v3, v4, v5, v6_io);
}

};
efb_popsession popsession{mv};
//#endif
