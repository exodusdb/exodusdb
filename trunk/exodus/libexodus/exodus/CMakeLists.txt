cmake_minimum_required(VERSION 2.8)

#2014/06/01 exodus doesnt require c++11 but we enable it for
#config is set to look in /usr/local/include and /usr/local/lib first for any source version of boost compiled for c++11
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-DEXODUS_IPC_BOOST)
add_definitions(-DEXODUS_IPC_POSIX)
add_definitions(-DEXODUS_IPC_WINPIPES)

### POSTGRES ###
################
##set(PostgreSQL_ADDITIONAL_VERSIONS "9.3" "9.3.4")
##find_package(PostgreSQL REQUIRED)
##find_package(PostgreSQL REQUIRED COMPONENTS pqlib)
#message(PostgreSQL_INCLUDE_DIRS=${PostgreSQL_INCLUDE_DIRS})
execute_process(
        COMMAND                 pg_config --includedir
        OUTPUT_VARIABLE         PostgreSQL_INCLUDE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(PostgreSQL_INCLUDEDIR=${PostgreSQL_INCLUDE_DIR})

### BOOST ###
#############
#date_time appears to be header only in later boost versions eg 55
#find_package(Boost REQUIRED COMPONENTS filesystem date_time system regex thread)
find_package(Boost REQUIRED COMPONENTS filesystem system regex thread)
message("BUT LOOKING IN /usr/local/lib and /usr/local/include FIRST!")

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME     ON)

### ICU ###
###########
#FindICU.cmake in cmake directory look in three levels of parent directories of source too
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};cmake;${PROJECT_SOURCE_DIR}/cmake/;${PROJECT_SOURCE_DIR}/../cmake/;${PROJECT_SOURCE_DIR}/../../cmake;${PROJECT_SOURCE_DIR}/../../../cmake")
#note that displayed path confusingly shows no space or ; between multiple paths if multiple are specified either by space or ;
message(CMAKE_MODULE_PATH=${CMAKE_MODULE_PATH})

find_package(ICU 4.4 REQUIRED)

### include postgres ###

include_directories(. .. ${PostgreSQL_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIR})

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
target_link_libraries(exodus -L/usr/local/lib ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} ${PostgreSQL_LIBRARY_DIR} pq ${ICU_LIBRARIES} dl)

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
