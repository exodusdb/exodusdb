//generated by exodus "compile docmods"
//#ifndef EXODUSDLFUNC_DOCMODS_H
#define EXODUSDLFUNC_DOCMODS_H

//a member variable/object to cache a pointer/object for the shared library function
//ExodusFunctorBase efb_docmods;
class efb_docmods : public ExodusFunctorBase
{
public:

efb_docmods(MvEnvironment& mv) : ExodusFunctorBase("docmods", "exodusprogrambasecreatedelete_", mv) {}

efb_docmods& operator=(const var& newlibraryname) {
        closelib();
        libraryname_=newlibraryname.toString();
        return (*this);
}

//a member function with the right arguments, returning a var or void
var operator() (in letterheadopts, io tx)
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
 //return CALLMEMBERFUNCTION(*(efb_docmods.pobject_),
 //((pExodusProgramBaseMemberFunction) (efb_docmods.pmemberfunction_)))
 // (mode);
 return CALLMEMBERFUNCTION(*(this->pobject_),
 ((pExodusProgramBaseMemberFunction) (this->pmemberfunction_)))
  (letterheadopts,tx);

}

var operator() () {
 var letterheadopts_in;
 var tx_io;
 return operator()(letterheadopts_in, tx_io);
}

var operator() (in letterheadopts) {
 var tx_io;
 return operator()(letterheadopts, tx_io);
}

};
efb_docmods docmods{mv};
//#endif
