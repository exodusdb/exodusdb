INCLUDES = -I$(top_srcdir)

dist_bin_SCRIPTS = installexodus-postgresql installexodus-postgresql2

AM_CPPFLAGS=-I$(top_srcdir) $(POSTGRESQL_CFLAGS) $(BOOST_CPPFLAGS) \
 -Wall -Wextra  -Wno-unknown-pragmas
# -pedantic

# library to build and how #

#build a library pgexodus #and install it in lib (default prefix/libdir /usr/local/lib)
lib_LTLIBRARIES= pgexodus.la

#sources of the library
pgexodus_la_SOURCES= mvconfig.h callexodus.cpp callexodus_ms.cpp callexodus_posix.cpp \
 naturalorder.cpp pgnaturalorder.cpp extract.c pgexodus.c

#linker flags for the library
#-module to allow library without lib prefix since postgres doesnt seem to like it
pgexodus_la_LDFLAGS= -module -version-info $(GENERIC_LIBRARY_VERSION) \
 -release $(GENERIC_RELEASE) $(POSTGRESQL_LDFLAGS) $(BOOST_LDFLAGS)

install-data-hook:
	(	(rm -f `pg_config --pkglibdir`/pgexodus.so \
			&& echo "removed all pgexodus.so link" \
		) \
		|| echo "cannot remove pgexodus.so link" \
	)
	( $(LN_S) $(libdir)/pgexodus-$(GENERIC_RELEASE).so.0 `pg_config --pkglibdir`/pgexodus.so \
		|| echo "CANT INSTALL pgexodus.so. Are you not root/sudo?" \
	)
uninstall-hook:
	(	(rm -f `pg_config --pkglibdir`/pgexodus.so \
			&& echo "removed all pgexodus.so link" \
		) \
		|| echo "cannot remove pgexodus.so link" \
	)





