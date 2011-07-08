# Process this file with autoconf to produce a configure script.


dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------

#require autoconf version 2.65 (why?) and Libtool version 2.2 (why?)
AC_PREREQ([2.65])
LT_PREREQ([2.2])

AC_INIT
#AC_PREREQ([2.65])
AC_CONFIG_SRCDIR([exodus/mv.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR(config)

dnl -----------------------------------------------
dnl Package name and version number (user defined)
dnl -----------------------------------------------

GENERIC_LIBRARY_NAME=exodus

#release versioning
GENERIC_MAJOR_VERSION=11
GENERIC_MINOR_VERSION=6
GENERIC_MICRO_VERSION=1

#API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=11.6
AC_SUBST(GENERIC_API_VERSION)

#shared library versioning
#http://www.gnu.org/software/libtool/manual/libtool.html#Versioning
GENERIC_LIBRARY_VERSION=11:6:0
#                       | | |
#                +------+ | +---+
#                |        |     |
#             current:revision:age
#                |        |     |
#                |        |     +- increment if interfaces have been added
#                |        |        set to zero if interfaces have been removed
#                                  or changed
#                |        +- increment if source code has changed
#                |           set to zero if current is incremented
#                +- increment if interfaces have been added, removed or changed


dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

AC_SUBST(GENERIC_LIBRARY_VERSION)

#PACKAGE=$GENERIC_LIBRARY_NAME
PACKAGE=lib$GENERIC_LIBRARY_NAME

AC_SUBST(GENERIC_LIBRARY_NAME)

GENERIC_VERSION=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION.$GENERIC_MICRO_VERSION
GENERIC_RELEASE=$GENERIC_MAJOR_VERSION.$GENERIC_MINOR_VERSION
AC_SUBST(GENERIC_RELEASE)
AC_SUBST(GENERIC_VERSION)

VERSION=$GENERIC_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

LT_INIT
AC_PROG_CXX
AM_SANITY_CHECK
AC_LANG([C++])

dnl ---------------------------------------
dnl http://www.nongnu.org/autoconf-archive/
dnl see macros ./m4/*.m4
dnl ---------------------------------------
dnl https://github.com/tsuna/boost.m4 maybe better than
dnl autoconf macro archive

dnl checking for PostgreSQL libraries 8.1+
m4_include([m4/ax_lib_postgresql.m4])
AX_LIB_POSTGRESQL([8.1.0])

dnl checking for ICU libraries 3.0 +
dnl TODO should be optional if BOOST_HASNT_ICU is set
m4_include([m4/ax_check_icu.m4])
AX_CHECK_ICU([3.0])

m4_include([m4/ax_boost_base.m4])
m4_include([m4/ax_boost_date_time.m4])
m4_include([m4/ax_boost_filesystem.m4])
m4_include([m4/ax_boost_regex.m4])
m4_include([m4/ax_boost_system.m4])
m4_include([m4/ax_boost_thread.m4])

ls -la -d /usr/include/boost*
ls -la /usr/lib/libboost*

#utf8_codecvt fixed in filesystem library from 1.40
#redhat 5 = 1.33, redhat 6 = 1.41
#AX_BOOST_BASE([1.32.0])
dnl boost base checks if include can be be found
AX_BOOST_BASE([1.40.0],,exit 1)
#TODO FIND A WAY TO STOP IF THE CORRECT LIBS ARE NOT AVAILABLE DESPITE BASE DETECTION SUCCEEDING!
AX_BOOST_DATE_TIME
dnl system is required for filesystem boost v39+
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_REGEX
AX_BOOST_THREAD
#AX_BOOST_SERIALIZATION

# Checks for header files.
AC_CHECK_HEADERS([netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_MODE_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_FUNC_STRCOLL
AC_DIAGNOSE([obsolete],[your code may safely assume C89 semantics that RETSIGTYPE is void.
Remove this warning and the `AC_CACHE_CHECK' when you adjust the code.])dnl
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])


# Checks for library functions.
AC_FUNC_MALLOC
AC_DIAGNOSE([obsolete],[your code may safely assume C89 semantics that RETSIGTYPE is void.
Remove this warning and the `AC_CACHE_CHECK' when you adjust the code.])dnl
AC_CACHE_CHECK([return type of signal handlers],[ac_cv_type_signal],[AC_COMPILE_IFELSE(
[AC_LANG_PROGRAM([#include <sys/types.h>
#include <signal.h>
],
		 [return *(signal (0, 0)) (0) == 1;])],
		   [ac_cv_type_signal=int],
		   [ac_cv_type_signal=void])])
AC_DEFINE_UNQUOTED([RETSIGTYPE],[$ac_cv_type_signal],[Define as the return type of signal handlers
		    (`int' or `void').])

AC_FUNC_STRCOLL
AC_CHECK_FUNCS([atexit floor getcwd gettimeofday memset pow putenv select setenv socket sqrt strchr strerror strstr backtrace])

dnl -----------------------------------------------
dnl Generates Makefile's, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
          exodus.pc \
          exodus/Makefile 
])
AC_OUTPUT

dnl 
dnl Congrats. You have configured the Exodus library
dnl
dnl Type 'make && sudo make install'
