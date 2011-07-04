#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([exodus-cli],[11.5],[steve.bush@neosys.com])
AC_CONFIG_SRCDIR([src/exodus.cpp])
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_AUX_DIR(config)

# Checks for programs.
AC_PROG_CXX
#AC_PROG_CC
AC_PROG_INSTALL

AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE(exodus-cli, 11.5, no-define)
AM_SANITY_CHECK

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL

# Checks for library functions.
AC_CHECK_FUNCS([pow select])

AC_CONFIG_FILES([Makefile \
 src/Makefile \
])

AC_OUTPUT

echo
echo "Congrats. You have configured the exodus-cli"
echo
echo "Type 'make && sudo make install'"
