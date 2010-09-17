#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.62)
AC_INIT(exodus_all, 9.6, steve.bush@neosys.com)
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
echo "You have configured the Exodus Library, Exodus Postgres plugin and Exodus CLI"
echo
echo "If you have root privileges then type 'make && make install'"
echo "otherwise type 'make && sudo make install'"
