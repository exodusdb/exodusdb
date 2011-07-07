# postinstall scripts to setup postgres
#--------------------------------------
dist_bin_SCRIPTS = installexodus-postgresql installexodus-postgresql2

# compiler options
#-----------------

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS=-I$(top_srcdir) $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) \
 -Wall -Wextra  -Wno-unknown-pragmas

# library to build
#-----------------

#build a library pgexodus #and install it in lib (default prefix/libdir /usr/local/lib)
lib_LTLIBRARIES= pgexodus.la

#sources of the library
pgexodus_la_SOURCES= mvconfig.h callexodus.cpp callexodus_ms.cpp callexodus_posix.cpp \
 naturalorder.cpp pgnaturalorder.cpp extract.c pgexodus.c

#linker flags for the library
#-module to allow library without lib prefix since postgres doesnt seem to like it
pgexodus_la_LDFLAGS= -module -version-info $(GENERIC_LIBRARY_VERSION) \
 -release $(GENERIC_RELEASE) $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)

# postinstall
#------------
#add a symbolic link in postgres package libdir pointing to the lib in \usr\lib
install-data-hook:
	rm -f $(DESTDIR)`pg_config --pkglibdir`/pgexodus.so \
		|| echo "cannot remove pgexodus.so link"
	mkdir -p $(DESTDIR)`pg_config --pkglibdir`
	$(LN_S) $(DESTDIR)$(libdir)/pgexodus-$(GENERIC_RELEASE).so.0 $(DESTDIR)`pg_config --pkglibdir`/pgexodus.so
	( { ldconfig 2> /dev/null ; } || { /sbin/ldconfig 2> /dev/null ; } \
		|| echo "cannot ldconfig")

# postuninstall
#--------------
#remove the symbolic link in postgres package dir
uninstall-hook:
	rm -f $(DESTDIR)`pg_config --pkglibdir`/pgexodus.so \
		|| echo "cannot remove pgexodus.so link"
	( { ldconfig 2> /dev/null ; } || { /sbin/ldconfig 2> /dev/null ; } \
		|| echo "cannot ldconfig")
