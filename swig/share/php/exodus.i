%module(docstring="An interface to Exodus") exodus

%include <swig.swg>

%include "php/php.swg"

//%include "php/phpinit.swg"
//%include "php/phprun.swg"
//%include "php/phpkw.swg"

%include "php/typemaps.i"
%include "php/std_string.i"

%include "exception.i"
%include "php/utils.i"

%rename(mvar) var;
%rename(__toString) toString;

%{
//#include "exodus/mvenvironment.h"
//#include "exodus/exodusfuncs.h"
//#include "exodus/exodusmacros.h"
#include "exodus/mv.h"
%}

%include <exception.i>

namespace exodus {

    //copied from std:string but full significance of %naturalvar not fully considered
    %naturalvar var;

    class var;

    //declare that anything can be converted to a var with top priority
    //otherwise overloaded methods will fail with "no overload available" unless
    //arguments are provided as var ... despite the fact that typemaps define convertibility
    %typemap(typecheck,precedence=0) exodus::var, exodus::var&, const exodus::var, const exodus::var& {$1=1;}

    //currently using same method to pass const and non-const var references to exodus
    //const var&
    //var&
    %typemap(in) var& (exodus::var tempvar)
    %{
    switch ((*($input))->type) {
	case IS_DOUBLE:
		//tempvar=(exodus::var) (*$input)->value.dval;
		tempvar=Z_DVAL_PP($input);
		break;
	case IS_STRING:
		//tempvar.assign(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
		//TODO provide var::assign() to avoid creation of temporary
		tempvar=std::string(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
		break;
	case IS_LONG:
		//convert_to_long_ex($input);
		//tempvar=(exodus::var) (long long int) (*$input)->value.lval;
		tempvar=exodus::var((int) (Z_LVAL_PP($input)));
		//tempvar=123456;
		break;
	default:
		convert_to_string_ex($input);
		tempvar=std::string(Z_STRVAL_PP($input), Z_STRLEN_PP($input));		
    }
    $1=&tempvar;
    %}

    //pass by value probably doesnt occur anywhere in exodus
    %typemap(in) var
    %{
        convert_to_string_ex($input);
        //$1.assign(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
	{
		exodus::var tempvar(Z_STRVAL_PP($input), Z_STRLEN_PP($input));
		$1=tempvar;
	}
    %}

/* this would convert all vars returned by functions (not function arguments) to be converted to php strings
    %typemap(out) var %{
	{
		std::string tempstr($1.toString());
		ZVAL_STRINGL($result, const_cast<char*>(tempstr.data()), tempstr.size(), 1);
	}
    %}

    %typemap(out) const var & %{
	{
		std::string tempstr($1->toString());
		ZVAL_STRINGL($result, const_cast<char*>(tempstr.data()),tempstr.size(), 1);
	}
    %}
*/

/* would cause all var& function arguments to be converted to php strings on return

    %typemap(argout) var & %{
	{
		std::string tempstr=$1->toString();
		ZVAL_STRINGL(*($input), const_cast<char*>(tempstr.data()), tempstr.size(), 1);
	}
    %}

    //SWIG will apply the non-const typemap above to const var& without
    // this more specific typemap. 
    %typemap(argout) const var & "";
*/

    //not sure how this works, currently using unassigned var crashes out of php ... TODO
    %typemap(throws) var, const var& %{
        zend_throw_exception(NULL, const_cast<char*>($1.toString().c_str()), 0 TSRMLS_CC);
        return;
    %}

}









//%include "exodus/mvenvironment.h"
//%include "exodus/exodusfuncs.h"
//%include "exodus/exodusmacros.h"
%include "exodus/mv.h"
