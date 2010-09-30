## Source directory

### not for now .. if restore then ALSO restore LIBADD= below
#SUBDIRS = sub
SUBDIRS = 

#some header files
h_sources = exodus.h exodusmacros.h mvenvironment.h mv.h mvlibs.h \
exodusfuncs.h mvexceptions.h mvimpl.h mvutf.h mvipc.h mvconfig.h \
mvlink.h mvfunctor.h

#some more headers
buildonly_h_sources = ConvertUTF.h NullCodecvt.h TConvert.h gettimeofday.h neo.h mvconfig.h MurmuHash2_64.h

#source files
cc_sources = ConvertUTF.c howto.cpp mvarray.cpp mvdatetime.cpp  mvdebug.cpp mvfuncs.cpp mvioconv.cpp mvlibs.cpp mvos.cpp \
mvutf.cpp naturalorder.cpp gettimeofday.cpp mv2.cpp mv.cpp mvdbpostgres.cpp mvenvironment.cpp mvglobalfuncs.cpp  \
mvipc.cpp mvmv.cpp mvunicode.cpp mvipc_boost.cpp mvipc_posix.cpp \
mvipc_ms.cpp mvposix.cpp MurmurHash2_64.cpp \
mvfunctor.cpp

#where to install the libraries include headers
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)

#name of the libraries include headers to be installed
library_include_HEADERS = $(h_sources)

#compiler flags for all compilations
AM_CPPFLAGS = -I$(top_srcdir) $(BOOST_CPPFLAGS) $(POSTGRESQL_CFLAGS)

#libs for all links
LIBS += \
	$(BOOST_DATE_TIME_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_REGEX_LIB)

#make a library with  libtools
##############################

lib_LTLIBRARIES= libexodus.la

#source code files for the library
libexodus_la_SOURCES= $(h_sources) $(cc_sources) $(buildonly_h_sources)

#compiler flags for the library - not used - using global AM_CPPFLAGS
#libexodus_la_CPPFLAGS=$(BOOST_CPPFLAGS)

#linker lib names for the library - not used - using global LIBS
#libexodus_la_LIBS= 

#sublibs - not for now. needs SUBDIRS above too
#libexodus_9_6_la_LIBADD= sub/libsub.la

#linker flags for the library
libexodus_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE) \
$(BOOST_LDFLAGS) \
$(BOOST_DATE_TIME_LDFLAGS) $(BOOST_FILESYSTEM_LDFLAGS) $(BOOST_REGEX_LDFLAGS) $(BOOST_SYSTEM_LDFLAGS) $(BOOST_THREAD_LDFLAGS) \
$(POSTGRESQL_LDFLAGS)

#additional symbolic links but ...
#doesnt work because cant workout exact library naming on all plaforms and libext is missing
#therefore we have to rely on libtool and Wl option
#libtool --config|grep ext
#actlibfile=lib$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)$(LIBEXT).$(GENERIC_LIBRARY_VERSION)
#symlibfile=lib$(GENERIC_LIBRARY_NAME)$(LIBEXT)
install-data-hook:
	( cd $(includedir) && { $(LN_S) -f exodus-$(GENERIC_RELEASE)/exodus exodus || { rm -f exodus && $(LN_S) exodus-$(GENERIC_RELEASE)/exodus exodus; }; })
	( { echo $(libdir) > /etc/ld.so.conf.d/exodus-$(GENERIC_RELEASE).conf 2>/dev/null ; } || echo "skipping /etc/ld.so.conf.d")
	( { ldconfig 2> /dev/null ; } || { /sbin/ldconfig 2> /dev/null ; } || echo "skipping ldconfig")
uninstall-hook:
	( cd $(includedir) && { rm -f exodus ; } )
	( cd $(includedir) && { rm -rf exodus-$(GENERIC_RELEASE) ; } )
	( rm -f /etc/ld.so.conf.d/exodus-$(GENERIC_RELEASE).conf || echo "/etc/ld.so.conf.d/exodus-$(GENERIC_RELEASE).conf doesnt exist or is already removed" )
	( { ldconfig 2> /dev/null ; } || { /sbin/ldconfig 2> /dev/null ; } || echo "skipping ldconfig" )
