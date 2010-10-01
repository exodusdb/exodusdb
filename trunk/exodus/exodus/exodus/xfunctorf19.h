#ifndef ExodusFunctorF19_H
#define ExodusFunctorF19_H

//generated by genfunctors.cpp

#include <exodus/mvfunctor.h>

template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10, class T11, class T12, class T13, class T14, class T15, class T16, class T17, class T18, class T19>
class ExodusFunctorF19 : private ExodusFunctorBase
{
public:
 ExodusFunctorF19(const std::string libname,const std::string funcname)
	: ExodusFunctorBase(libname,funcname){}

 var operator() ()
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(T1(), T2(), T3(), T4(), T5(), T6(), T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, T2(), T3(), T4(), T5(), T6(), T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, T3(), T4(), T5(), T6(), T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, T4(), T5(), T6(), T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, T5(), T6(), T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, T6(), T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, T7(), T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, T8(), T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, T9(), T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, T10(), T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, T11(), T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, T12(), T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, T13(), T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, T14(), T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, T15(), T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, T16(), T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, T17(), T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, T18(), T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, T19());
 }

 var operator() (T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19)
 {
	checkload();
	typedef var (*ExodusDynamic)(T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5, T6 arg6, T7 arg7, T8 arg8, T9 arg9, T10 arg10, T11 arg11, T12 arg12, T13 arg13, T14 arg14, T15 arg15, T16 arg16, T17 arg17, T18 arg18, T19 arg19);
	return  ((ExodusDynamic) _pfunction)(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, arg14, arg15, arg16, arg17, arg18, arg19);
 }

};
#endif
