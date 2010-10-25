#ifndef ExodusFunctorF2_H
#define ExodusFunctorF2_H

//generated by genfunctors.cpp

#include <exodus/mvfunctor.h>

template<class T1, class T2>
class ExodusFunctorF2 : private ExodusFunctorBase
{
public:
 ExodusFunctorF2(const std::string libname,const std::string funcname)
	: ExodusFunctorBase(libname,funcname){}

 var operator() ()
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2);
	return  ((ExodusDynamic) pfunction_)(T1(), T2());
 }

 var operator() (T1 arg1)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2);
	return  ((ExodusDynamic) pfunction_)(arg1, T2());
 }

 var operator() (T1 arg1, T2 arg2)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2);
	return  ((ExodusDynamic) pfunction_)(arg1, arg2);
 }

};
#endif
