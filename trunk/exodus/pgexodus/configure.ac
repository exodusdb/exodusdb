# Process this file with autoreconf or autoconf to produce a configure script.

AC_PREREQ(2.67)

dnl --------------------------------
dnl Initialization macros.
dnl --------------------------------


AC_INIT
AC_CONFIG_SRCDIR([src/mvconfig.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_AUX_DIR(config)

dnl -----------------------------------------------
dnl Package name and version number (user defined)
dnl -----------------------------------------------

GENERIC_LIBRARY_NAME=pgexodus

#release versioning
GENERIC_MAJOR_VERSION=9
GENERIC_MINOR_VERSION=6
GENERIC_MICRO_VERSION=0

#API version (often = GENERIC_MAJOR_VERSION.GENERIC_MINOR_VERSION)
GENERIC_API_VERSION=9.6
AC_SUBST(GENERIC_API_VERSION)

#shared library versioning
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

AM_INIT_AUTOMAKE($PACKAGE, $VERSION)

AC_DISABLE_STATIC
AC_ENABLE_SHARED
AM_DISABLE_STATIC
AM_ENABLE_SHARED
AC_PROG_LIBTOOL

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CC
AC_PROG_CXX

AM_PROG_LIBTOOL
AM_SANITY_CHECK

AC_LANG([C++])
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.

dnl ---------------------------------------
dnl http://www.nongnu.org/autoconf-archive/
dnl see macros ./m4/*.m4
dnl ---------------------------------------

#NB patch for postgres SERVER in m4/ax_lib_postgresql.m4
#            POSTGRESQL_CFLAGS="-I`$PG_CONFIG --includedir` -I`$PG_CONFIG --includedir-server`"
AX_LIB_POSTGRESQL([8.1.0])

libdir=`pg_config --pkglibdir`

#only needed if no windows pipes and no unix sockets

AX_BOOST_BASE([1.32.0])

#AX_BOOST_DATE_TIME
#AX_BOOST_FILESYSTEM
#AX_BOOST_REGEX
#AX_BOOST_SYSTEM
#AX_BOOST_THREAD

# Checks for header files.
AC_CHECK_HEADERS([netinet/in.h stdlib.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([memset socket strchr])


dnl -----------------------------------------------
dnl Generates Makefile's, configuration files and scripts
dnl -----------------------------------------------

AC_CONFIG_FILES([Makefile \
          src/Makefile
])
AC_OUTPUT

dnl
dnl Congrats. You have configured pgexodus.
dnl
dnl Type 'make && sudo make install'
