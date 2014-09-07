message("")
message("--LIBPGEXODUS CMAKE INIT--")

cmake_minimum_required(VERSION 2.8)


# CONFIGURE PostgreSQL

find_package(PostgreSQL REQUIRED)

execute_process(
	COMMAND 		pg_config --pkglibdir
	OUTPUT_VARIABLE		PostgreSQL_PKGLIBDIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(PostgreSQL_PKGLIBDIR=${PostgreSQL_PKGLIBDIR})

#pg_config does seem to find the right folder so add it!
#ONLY FOR LOCALLY INSTALLED postgres 9.4 on ubuntu 14.04
#set(PostgreSQL_PKGLIBDIR ${PostgreSQL_PKGLIBDIR}/postgresql/)
#message(PostgreSQL_PKGLIBDIR=${PostgreSQL_PKGLIBDIR})

IF(NOT ${PostgreSQL_PKGLIBDIR} MATCHES ".*")
message(FATAL_ERROR "cannot find PostgreSQL pg_config program. Set PATH and try again.")
#message(FATAL_ERROR "nb sudo on ubuntu ignores PATH environment variable")
endif()

execute_process(
	COMMAND 		pg_config --includedir-server
	OUTPUT_VARIABLE		PostgreSQL_INCLUDEDIR_SERVER
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(PostgreSQL_INCLUDEDIR_SERVER=${PostgreSQL_INCLUDEDIR_SERVER})


include_directories(
	.
	..
	${PostgreSQL_INCLUDE_DIRS}
	${PostgreSQL_INCLUDEDIR_SERVER}
)


# BUILD libpgexodus

#postgresql shared object library c functions are not prefixed with "lib"
set (CMAKE_SHARED_LIBRARY_PREFIX "")

add_library(pgexodus SHARED

 mvconfig.h callexodus.cpp callexodus_ms.cpp callexodus_posix.cpp
 naturalorder.cpp pgnaturalorder.cpp extract.c pgexodus.c

)

#set_target_properties(pgexodus PROPERTIES OUTPUT_NAME pgexodus)


# INSTALL libpgexodus postgres server shared object and postinstall scripts

#place pgexodus.so in server package lib directory (not ordinary lib)
install(TARGETS pgexodus
DESTINATION ${PostgreSQL_PKGLIBDIR})
#also install in another location in case of dev install and pg_config pkglibdir is wrong
install(TARGETS pgexodus
DESTINATION ${PostgreSQL_PKGLIBDIR}/postgresql)

#script to add exodus functions into all new databases and exodus database
install(PROGRAMS installexodus-postgresql installexodus-postgresql2
DESTINATION ${PostgreSQL_PKGLIBDIR})

#				pg_config --pkglibdir
install(
	CODE "
		execute_process(
			COMMAND 
				ldconfig
				OUTPUT_VARIABLE OUTVAR
				ERROR_VARIABLE ERRVAR
				RESULT_VARIABLE RESVAR
			
		)
		message(ldconfig <\${OUTVAR}>\n
			<\${ERRVAR}>\n
			<\${RESVAR}>\n
		)
	"
)
install(
	CODE "
		execute_process(
			COMMAND 
				${PostgreSQL_PKGLIBDIR}/installexodus-postgresql
#				OUTPUT_VARIABLE OUTVAR
#				ERROR_VARIABLE ERRVAR
				RESULT_VARIABLE RESVAR
			
		)
		message(<\${OUTVAR}>\n
			<\${ERRVAR}>\n
			<\${RESVAR}>\n
		)
	"
)

#install (SCRIPT "${CMAKE_SOURCE_DIR}/PostInstall.cmake")

message("--LIBPGEXODUS CMAKE EXIT--")
