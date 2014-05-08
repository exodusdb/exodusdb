cmake_minimum_required(VERSION 2.8)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "cmake/")
message(CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH})

add_definitions(-DEXODUS_IPC_BOOST)
add_definitions(-DEXODUS_IPC_POSIX)
add_definitions(-DEXODUS_IPC_WINPIPES)

### POSTGRES ###
################
#set(PostgreSQL_DEBUG 1)
#find_package(PostgreSQL REQUIRED)
#find_package(PostgreSQL REQUIRED COMPONENTS pqlib)

# set(PostgreSQL_ADDITIONAL_VERSIONS "")
#for development version
#set(PostgreSQL_INCLUDE_DIR "/usr/local/pgsql/include/")
#set(PostgreSQL_LIBRARY_DIR "/usr/local/pgsql/lib/")

execute_process(
        COMMAND                 pg_config --includedir
        OUTPUT_VARIABLE         PostgreSQL_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(PostgreSQL_INCLUDE_DIR=${PostgreSQL_INCLUDE_DIR})

#execute_process(
#        COMMAND                 pg_config --libs
#        OUTPUT_VARIABLE         PostgreSQL_LIBRARIES
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#message(PostgreSQL_LIBRARIES=${PostgreSQL_LIBRARIES})


### BOOST ###
#############
find_package(Boost REQUIRED COMPONENTS filesystem date_time system regex thread)


### ICU ###
###########
find_package(ICU 4.4 REQUIRED)

include_directories(. .. ${PostgreSQL_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIR})

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)

add_library(exodus SHARED
 exodus.h exodusmacros.h mvenvironment.h mv.h mvlibs.h
 exodusfuncs.h mvexceptions.h mvimpl.h mvutf.h mvipc.h mvconfig.h
 mvlink.h mvfunctor.h program.h library.h dict.h mvprogram.h
 xfunctorf0.h xfunctorf1.h xfunctorf10.h xfunctorf11.h xfunctorf12.h
 xfunctorf13.h xfunctorf14.h xfunctorf15.h xfunctorf16.h xfunctorf17.h
 xfunctorf18.h xfunctorf19.h xfunctorf2.h xfunctorf20.h xfunctorf3.h
 xfunctorf4.h xfunctorf5.h xfunctorf6.h xfunctorf7.h xfunctorf8.h
 xfunctorf9.h xfunctors0.h xfunctors1.h xfunctors10.h xfunctors11.h
 xfunctors12.h xfunctors13.h xfunctors14.h xfunctors15.h xfunctors16.h
 xfunctors17.h xfunctors18.h xfunctors19.h xfunctors2.h xfunctors20.h
 xfunctors3.h xfunctors4.h xfunctors5.h xfunctors6.h xfunctors7.h
 xfunctors8.h xfunctors9.h

 ConvertUTF.h NullCodecvt.h TConvert.h gettimeofday.h neo.h mvconfig.h
 MurmurHash2_64.h mvhandles.h mvdbconns.h

 ConvertUTF.c howto.cpp mvarray.cpp mvdatetime.cpp  mvdebug.cpp
 mvfuncs.cpp mvioconv.cpp mvlibs.cpp
 mvutf.cpp naturalorder.cpp gettimeofday.cpp mv2.cpp mv.cpp
 mvdbpostgres.cpp mvenvironment.cpp mvglobalfuncs.cpp
 mvipc.cpp mvmv.cpp mvunicode.cpp mvipc_boost.cpp mvipc_posix.cpp
 mvipc_ms.cpp mvposix.cpp MurmurHash2_64.cpp
 mvfunctor.cpp mvprogram.cpp mvhandles.cpp mvdbconns.cpp mvos.cpp
)

#set_target_properties(exodus PROPERTIES SOVERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_VERSION}")
set_target_properties(exodus PROPERTIES SOVERSION "14.01")

#dl is needed for dynamic link library code
target_link_libraries(exodus ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} ${PostgreSQL_LIBRARY_DIR} pq ${ICU_LIBRARIES} dl)

install(TARGETS exodus DESTINATION lib)

install(FILES 
 exodus.h exodusmacros.h mvenvironment.h mv.h mvlibs.h 
 exodusfuncs.h mvexceptions.h mvimpl.h mvutf.h mvipc.h mvconfig.h 
 mvlink.h mvfunctor.h program.h library.h dict.h common.h mvprogram.h
 xfunctorf0.h xfunctorf1.h xfunctorf10.h xfunctorf11.h xfunctorf12.h 
 xfunctorf13.h xfunctorf14.h xfunctorf15.h xfunctorf16.h xfunctorf17.h 
 xfunctorf18.h xfunctorf19.h xfunctorf2.h xfunctorf20.h xfunctorf3.h 
 xfunctorf4.h xfunctorf5.h xfunctorf6.h xfunctorf7.h xfunctorf8.h 
 xfunctorf9.h xfunctors0.h xfunctors1.h xfunctors10.h xfunctors11.h 
 xfunctors12.h xfunctors13.h xfunctors14.h xfunctors15.h xfunctors16.h 
 xfunctors17.h xfunctors18.h xfunctors19.h xfunctors2.h xfunctors20.h 
 xfunctors3.h xfunctors4.h xfunctors5.h xfunctors6.h xfunctors7.h 
 xfunctors8.h xfunctors9.h
DESTINATION include/exodus)
