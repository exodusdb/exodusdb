%module(docstring="An interface to Exodus.") exodus

#ifdef SWIGJAVA
%include "java/java.swg"
%include "java/javakw.swg"
%include "java/typemaps.i"
%include "java/std_wstring.i"
%include "java/std_string.i" 
%include "exception.i"
%include "php/utils.i"
%include <std_string.i>
//%rename (tostring) toString;
#endif

#ifdef SWIGPYTHON

%include "python/pywstrings.swg"
%include "python/pystrings.swg"
%include "python/std_string.i"
%include "python/typemaps.i"

%fragment("SWIG_AsVal_wchar_t", "header", fragment="<wchar.h>") {
    SWIGINTERN int SWIG_AsVal_wchar_t(PyObject* p, wchar_t* c) {
        return SWIG_OK;
    }
}
%fragment("SWIG_From_wchar_t", "header", fragment="<wchar.h>") {
    SWIGINTERNINLINE PyObject* SWIG_From_wchar_t(wchar_t c) {
        return SWIG_Py_Void();
    }
} 

/*
// Python -> C
%typemap(in) wchar_t const * {
  $1 = PyString_to_wchar_t($input);
}

// C -> Python
%typemap(out) wchar_t * {
  $result = wchar_t_to_PyObject($1);
}
*/

//%rename(__repr__) toString;
%rename(__str__) toString;
#endif

#ifdef SWIGPHP
%include "php/php.swg"
//%include "php/phpinit.swg"
////%include "php/phprun.swg"
//%include "php/phpkw.swg"
%include "php/typemaps.i"
%include "php/std_string.i"
//%include "exception.i"
//%include "php/utils.i"

%rename(__toString) toString;

#endif

#ifdef SWIGPERL
%include "perl5/perl5.swg"
//%include "perl5/perlmain.i"
%include "perl5/typemaps.i"
%include "perl5/std_string.i"
//%include "std/std_basic_string.i"
//%include "std/std_wstring.i"

//http://perldoc.perl.org/overload.html

//swig interface is currently converting undefined var to string in some cases
//for now, return "" in that case
%rename(__str__) toString2;

//linking of swig gets confused and wants var:win32_connect and var:win32_select?!
//solution seems to be to undefine some macros (from where) in top of mv.h during swig generation
//%rename (connect) connect_renamed;
//%rename (select) select_renamed;
//%rename (abort) abort_renamed;

#endif

%{
#include "exodus/mv.h"
//#include "exodus/mvenvironment.h"
//#include "exodus/exodusfuncs.h"
%}

%include "exodus/mv.h"
//%include "exodus/mvenvironment.h"
//%include "exodus/exodusfuncs.h"
