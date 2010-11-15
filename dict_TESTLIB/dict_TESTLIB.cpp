//
// For the dictionary TESTLIB, this file should be called dict_TESTLIB.cpp and
//	the project should be dict_TESTLIB
//
#include <exodus/dict.h>

dict(AGE_IN_DAYS) {
        ANS=date()-RECORD(1);
}

dict(AGE_IN_YEARS) {
        ANS=calculate("AGE_IN_DAYS")/365.25;
}