#define BUILDING_LIBRARY
#include <exodus/exodus.h>

//a common section is a just a class section that can be instantiated by other programs.

//common can have MULTIPLE commoninit/exit sections and all are publically available
//by including the libraryfilename.h file name in other programs and libraries.

#undef subroutine
#undef function
#define subroutine public: void
#define function public: var

//a common section is just a class plus a definition into the common array
#define commoninit(COMMON_NAME,COMMON_NO) \
class COMMON_NAME##_common : public LabelledCommon{ \
public:

#define commonexit(COMMON_NAME,COMMON_NO) \
}; \
COMMON_NAME##_common&& COMMON_NAME=reinterpret_cast<COMMON_NAME##_common&&> (*mv.labelledcommon[COMMON_NO]);

/*
//works but is hard to to debug since there is no variable gen
//#define gen (*((gen_common*) mv.labelledcommon[gen_common_no]))

//cannot run conditional code during class member initialisation
//but can only create new common if not already done
//if (mv.labelledcommon[gen_common_no]==0)
//      mv.labelledcommon[gen_common_no]=new gen_common;

//works nicely but only if common already created otherwise points to nothing
//and if you try to reset it later, it tried to swap out nothing, causing segfault
gen_common&& gen=reinterpret_cast<gen_common&&> (*mv.labelledcommon[gen_common_no]);

//could use pointers but syntax is ugly gen->companies (and adecom must not recognise -> as an operator)
//gen_common*

//#define gen_isdefined (mv.labelledcommon[gen_common_no] != NULL)
*/
