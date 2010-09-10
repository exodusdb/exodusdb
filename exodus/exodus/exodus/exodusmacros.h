/*
Copyright (c) 2009 Stephen John Bush

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef EXODUSMACROS_H
#define EXODUSMACROS_H 1

#include <exodus/mv.h>

//please include <exodus> AFTER <iostream> etc. For example:
// #include <iostream>
// #include <exodus>
//OR insert "#undef eq" after #include <exodus> etc. For example:
// #include <exodus>
// #undef eq
//OR insert #undef eq before #include <iostream> etc. For example:
// #undef eq
// #include <iostream>
//you can regain the use of the keyword "eq" by inserting the following after any other includes
// #define eq ==
//(regrettably eq is defined in global namespace in some libraries)

//capture global _SENTENCE in wrapper function exodus_main
//main calls main2 so that opening { is required after program() macro
//TODO get exodus_main to call main2 directly - need to pass it a function pointer
//int exodus_main(int exodus__argc, char *exodus__argv[]); 
#define program() \
void main2(int exodus__argc, char *exodus__argv[]); \
int main(int exodus__argc, char *exodus__argv[]) \
{ \
	exodus_main(exodus__argc, exodus__argv); \
	main2(exodus__argc, exodus__argv); \
} \
void main2(int exodus__argc, char *exodus__argv[])

//allow pseudo pick syntax
#define sentence() _SENTENCE
#define subroutine void
#define function var
#define in const var&
#define out var&

//forcibly redefine "eq" even if already previously defined in some other library like iostream
//to generate a compilation error so that the issue can be corrected (see heading) and the "eq" keyword remain available
#define eq ==
//alternative
#define EQ ==
#define ne !=
#define gt >
#define lt <
#define le <=
#define ge >=

#define _IM "\xFF"
#define _RM "\xFF"
#define _FM "\xFE"
#define _AM "\xFE"
#define _VM "\xFD"
#define _SM "\xFC"
#define _SVM "\xFC"
#define _TM "\xFB"
#define _STM "\xFA"
#define _SSTM "\xF9"
#define _DQ "\""
#define _SQ "\'"

#endif //EXODUSMACROS_H
