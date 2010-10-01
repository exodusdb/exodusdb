#ifndef ExodusFunctorS1_H
#define ExodusFunctorS1_H

//generated by genfunctors.cpp

#include <exodus/mvfunctor.h>

template<class T1>
class ExodusFunctorS1 : private ExodusFunctorBase
{
public:
 ExodusFunctorS1(const std::string libname,const std::string funcname)
	: ExodusFunctorBase(libname,funcname){}

 void operator() ()
 {
	checkload();
	typedef void (*ExodusDynamic)(T1 arg1);
	 ((ExodusDynamic) _pfunction)(T1());
	return;
 }

 void operator() (T1 arg1)
 {
	checkload();
	typedef void (*ExodusDynamic)(T1 arg1);
	 ((ExodusDynamic) _pfunction)(arg1);
	return;
 }

};
#endif
