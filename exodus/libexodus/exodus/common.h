#define BUILDING_LIBRARY
#include <exodus/exodus.h>

// Provides commoninit() and commonexit() which create blocks of "named common" variables.
//
// A common section is a just a C++ class that can be used by other programs.
//
// Named commons inherit from ExoCommon so that they can be stored in mv.namedcommon[]
//
// An xxx_common.h file may have multiple commoninit/exit sections.
//

////////////////////////////////////////
// Open a class derived from ExoCommon
////////////////////////////////////////
#define commoninit(COMMON_CODE, COMMON_NO)           \
                                                     \
_Pragma("GCC diagnostic push")                       \
_Pragma("GCC diagnostic ignored \"-Wweak-vtables\"") \
                                                     \
class COMMON_CODE##_common : public ExoCommon {      \
                                                     \
_Pragma("GCC diagnostic push")                       \
                                                     \
 public:

//////////////////////////////////////////////////////////////////////
// Close the class and define a variable to access it and its members.
//////////////////////////////////////////////////////////////////////
#define commonexit(COMMON_CODE, COMMON_NO)                                                              \
                                                                                                        \
~COMMON_CODE##_common() = default;/*{std::cout << __PRETTY_FUNCTION__ << std::endl;}*/                  \
                                                                                                        \
};                                                                                                      \
                                                                                                        \
_Pragma("GCC diagnostic push")                                                                          \
_Pragma("GCC diagnostic ignored \"-Wpadded\"")                                                          \
                                                                                                        \
[[maybe_unused]]                                                                                        \
	COMMON_CODE##_common& COMMON_CODE = static_cast<COMMON_CODE##_common&>(*mv.namedcommon[COMMON_NO]); \
                                                                                                        \
_Pragma("GCC diagnostic pop")

//e.g.
//	agy_common& agy = static_cast<agy_common&>(*mv.namedcommon[3]);

///////////
// Comments
///////////

// if (!COMMON_CODE) mv.namedcommon[COMMON_NO]=new COMMON_CODE##_common;
//
// Works but is hard to to debug since there is no variable gen
//
//  #define gen (*((gen_common*) mv.namedcommon[gen_common_no]))
//
// Cannot run conditional code during class member initialisation
// but can only create new common if not already done
//
//  if (mv.namedcommon[gen_common_no]==0)
//      mv.namedcommon[gen_common_no]=new gen_common;
//
// Works nicely but only if common already created otherwise points to nothing
// and if you try to reset it later, it tried to replace out nothing, causing segfault
//
//  gen_common& gen = static_cast<gen_common&>(*mv.namedcommon[gen_common_no]);
//
// Could use pointers but syntax is ugly gen->companies
//
//  gen_common*
//  #define gen_isdefined (mv.namedcommon[gen_common_no] != nullptr)
