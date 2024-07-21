#ifndef EXO_VARDEFS_H
#define EXO_VARDEFS_H

#define BACKTRACE_MAXADDRESSES 100

namespace exo {
	using VAR    =       var;
	using VARREF =       var&;
	using CVR    = const var&;
	using TVR    =       var&&;
}

// Info used in cli/compile
//
#define EXODUS_RELEASE "24.07"
#define EXODUS_PATCH "24.07.0"

// Use ASCII 0x1A-0x1F for PickOS separator chars instead
// of PickOS 0xFA-0xFF which are illegal utf-8 bytes

// Also defined in pgexodus in extract.c etc.

// The var versions of the following (without leading or trailing _)
// are defined AFTER the class declaration of "var"

// Leading _ char* versions of classic pick delimiters
// Using macros to allow use of space as compile time concatenation operator
// e.g. _FM _VM will compile directly to "\x1F\x1E"

#define _RM "\x1F"  // Record Mark
#define _FM "\x1E"  // Field Mark
#define _VM "\x1D"  // Value Mark
#define _SM "\x1C"  // Subvalue Mark
#define _TM "\x1B"  // Text Mark
#define _ST "\x1A"  // Subtext Mark

#define _BS "\\"
#define _DQ "\""
#define _SQ "\'"

// trailing _ char versions of classic pick delimiters
//

#define RM_ '\x1F'     // Record Mark
#define FM_ '\x1E'     // Field Mark
#define VM_ '\x1D'     // Value Mark
#define SM_ '\x1C'     // Subvalue Mark
#define TM_ '\x1B'     // Text Mark
#define ST_ '\x1A'     // Subtext Mark

#define BS_ '\\'
#define DQ_ '\"'
#define SQ_ '\''

// Visible versions of FM, VM etc.
//
// printx() converts FM etc to these characters. user literal conversion _var also has them but hard coded in fmiconverter()
//#define _VISIBLE_FMS "_^]\[Z"  //PickOS standard. Backslash not good since it is often used for escaping chars. Z is normal letter.
//#define _VISIBLE_FMS "<[{}]>" //logical but hard to read direction of brackets quickly
//#define _VISIBLE_FMS "_^]}`~" //all uncommon in natural language. first 3 _^] are identical to pickos
//#define _VISIBLE_FMS "_^]}|~"   //all uncommon in natural language. first 3 _^] are identical to pickos
#define _VISIBLE_FMS "`^]}|~"   //all uncommon in natural language. ^] are identical to pickos. Using ` for RM since _ common in IT
#define VISIBLE_RM_ '`'
#define VISIBLE_FM_ '^'
#define VISIBLE_VM_ ']'
#define VISIBLE_SM_ '}'
#define VISIBLE_TM_ '|'
#define VISIBLE_ST_ '~'

#define _ALL_FMS _RM _FM _VM _SM _TM _ST

// Useful TRACE() function for debugging
//
#define TRACE(EXPRESSION) \
	var(EXPRESSION).convert(_ALL_FMS, _VISIBLE_FMS).quote().logputl("TRACE: " #EXPRESSION "=");
#define TRACE2(EXPRESSION) \
	std::cerr << (EXPRESSION) << std::endl;

// Readability for defaults
//
#define DEFAULT_UNASSIGNED = var()
#define DEFAULT_EMPTY = ""
#define DEFAULT_DOT = "."
#define DEFAULT_SPACE = " "
#define DEFAULT_VM = VM_
#define DEFAULT_NULL = nullptr

#endif //EXO_VARDEFS_H