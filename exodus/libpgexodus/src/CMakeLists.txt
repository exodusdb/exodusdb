cmake_minimum_required(VERSION 2.8)

find_package(PostgreSQL REQUIRED)

include_directories(
	.
	..
	${PostgreSQL_INCLUDE_DIRS}
	/usr/include/postgresql/9.1/server/
)

add_library(pgexodus SHARED

 mvconfig.h callexodus.cpp callexodus_ms.cpp callexodus_posix.cpp
 naturalorder.cpp pgnaturalorder.cpp extract.c pgexodus.c

)

install(TARGETS pgexodus DESTINATION ${PostgreSQL_LIBRARY_DIRS})
