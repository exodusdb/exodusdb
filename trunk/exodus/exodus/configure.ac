# Process this file with autoconf to produce a configure script.


dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------

AC_INIT
AC_PREREQ(2.67)
AC_CONFIG_SRCDIR([exodus/mv.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_AUX_DIR(config)

dnl -----------------------------------------------
dnl Package name and version number (user defined)
dnl -----------------------------------------------

GENERIC_LIBRARY_NAME=exodus

#release versioning
GENERIC_MAJOR_VERSION=10
GENERIC_MINOR_VERSION=10
GENERIC_MICRO_VERSION=0

#API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=10.10
AC_SUBST(GENERIC_API_VERSION)

#shared library versioning
#http://www.gnu.org/software/libtool/manual/libtool.html#Versioning
GENERIC_LIBRARY_VERSION=0:0:0
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

PACKAGE=$GENERIC_LIBRARY_NAME
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

#AC_DISABLE_STATIC
#AC_DISABLE_SHARED
#AC_ENABLE_SHARED
AC_PROG_LIBTOOL

#AM_ENABLE_STATIC
#AM_ENABLE_SHARED

AC_PROG_CC
AC_PROG_CXX
AM_PROG_LIBTOOL
AM_SANITY_CHECK

AC_LANG([C++])

dnl ---------------------------------------
dnl http://www.nongnu.org/autoconf-archive/
dnl see macros ./m4/*.m4
dnl ---------------------------------------

AX_BOOST_BASE([1.32.0])
AX_BOOST_DATE_TIME
AX_BOOST_FILESYSTEM
AX_BOOST_REGEX
#AX_BOOST_SYSTEM
AX_BOOST_THREAD

AX_LIB_POSTGRESQL([8.1.0])

#icu only required in boost at the moment
#AC_CHECK_ICU(version, action-if, action-if-not)
#AC_CHECK_ICU([3])

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
AC_TYPE_SIGNAL

# Checks for library functions.
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_STRCOLL
AC_CHECK_FUNCS([atexit floor getcwd gettimeofday memset pow putenv select setenv socket sqrt strchr strerror strstr backtrace])

dnl -----------------------------------------------
dnl Generates Makefile's, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
          exodus-10.10.pc \
          exodus/Makefile 
])
AC_OUTPUT

echo 
echo Congrats. You have configured the Exodus library
echo
echo Type 'make && sudo make install'
