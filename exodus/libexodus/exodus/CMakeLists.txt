find_package(PostgreSQL REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem date_time system regex thread)
#find_package(PostgreSQL REQUIRED COMPONENTS pqlib)
find_package(ICU 4.4 REQUIRED)

include_directories(. .. ${PostgreSQL_INCLUDE_DIRS})

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
 mvfuncs.cpp mvioconv.cpp mvlibs.cpp mvos.cpp
 mvutf.cpp naturalorder.cpp gettimeofday.cpp mv2.cpp mv.cpp
 mvdbpostgres.cpp mvenvironment.cpp mvglobalfuncs.cpp
 mvipc.cpp mvmv.cpp mvunicode.cpp mvipc_boost.cpp mvipc_posix.cpp
 mvipc_ms.cpp mvposix.cpp MurmurHash2_64.cpp
 mvfunctor.cpp mvprogram.cpp mvhandles.cpp mvdbconns.cpp
)

#dl is needed for dynamic link library code
target_link_libraries(exodus ${Boost_LIBRARIES} ${PostgreSQL_LIBRARIES} ${ICU_LIBRARIES} dl)
