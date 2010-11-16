#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.67)
#AC_INIT(exodus_all, 10.10, steve.bush@neosys.com)
AC_INIT(exodus, 10.10.12, steve.bush@neosys.com)
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR(config)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([exodus
                   cli])
AC_OUTPUT

echo
echo Congrats. You have configured:
echo 1. Exodus Library
echo 2. Exodus Postgres Plugin
echo 3. Exodus CLI
echo
echo Type 'make && sudo make install'
