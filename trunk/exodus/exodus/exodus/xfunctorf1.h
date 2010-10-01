#ifndef ExodusFunctorF1_H
#define ExodusFunctorF1_H

//generated by genfunctors.cpp

#include <exodus/mvfunctor.h>

template<class T1>
class ExodusFunctorF1 : private ExodusFunctorBase
{
public:
 ExodusFunctorF1(const std::string libname,const std::string funcname)
	: ExodusFunctorBase(libname,funcname){}

 var operator() ()
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1);
	return  ((ExodusDynamic) _pfunction)(T1());
 }

 var operator() (T1 arg1)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1);
	return  ((ExodusDynamic) _pfunction)(arg1);
 }

};
#endif
