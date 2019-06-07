#if 0

//following is an implementation of functors with zero arguments
//but for now we leave such beasts implemented with a class
//template with one UNUSED template argument
//since templates without arguments are not allowed

#ifndef ExodusFunctorF0_H
#define ExodusFunctorF0_H

//NOT generated by genfunctors.cpp

#include <exodus/mvfunctor.h>

namespace exodus {

//template<>
class ExodusFunctorF0 : private ExodusFunctorBase
{
public:
 ExodusFunctorF0(const std::string libname,const std::string funcname);

 var operator() ();

};

}//namespace exodus

#endif


/// what follows is the cpp file

#define MV_NO_NARROW

#include <exodus/mv.h>
#include <exodus/xfunctorf0.h>
#include <string>

namespace exodus
{

ExodusFunctorF0::ExodusFunctorF0(const std::string libname,const std::string funcname)
	: ExodusFunctorBase(libname,funcname){}

var ExodusFunctorF0::operator() ()
 {
	checkload();
	typedef var (*ExodusDynamic)();
	return  ((ExodusDynamic) pfunction_)();
 }

}//namespace exodus

#endif