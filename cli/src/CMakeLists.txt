cmake_minimum_required(VERSION 2.8)

#2014/06/01 exodus doesnt require c++11 but we enable it for
#removing this to avoid problem linking to filesystem at the moment with c++11

#works using c++14 default on Ubuntu 18.04 so no need for this
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic")

message("")
message("--EXODUS CLI CMAKE INIT --")

include_directories(
	../../exodus/libexodus
)

project(exodus)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}../../exodus/libexodus/exodus)


### 1. COMPILE ###

#NB add any NEW programs here and in TWO MORE sections further down
#target_link_libraries(
#install(

### DONT FORGET TO ADD TO ALL STAGES (1, 2 AND 3) ###

# bin #

add_executable(compile compile.cpp)
add_executable(createfile createfile.cpp)
add_executable(createindex createindex.cpp)
add_executable(delete delete.cpp)
add_executable(clearfile clearfile.cpp)
add_executable(deletefile deletefile.cpp)
add_executable(deleteindex deleteindex.cpp)
add_executable(edir edir.cpp)
add_executable(edic edic.cpp)
add_executable(dict2sql dict2sql.cpp)
add_executable(list list.cpp printtext.h)
add_executable(listfiles listfiles.cpp)
add_executable(listdict listdict.cpp)
add_executable(listindexes listindexes.cpp)
add_executable(testsort testsort.cpp)
add_executable(configexodus configexodus.cpp)

#name is name only ... input and output files are exodus
add_executable(exodus-cli exodus.cpp)
set_target_properties(exodus-cli PROPERTIES OUTPUT_NAME exodus)

# lib #

add_library(dict_myclients SHARED dict_myclients.cpp)
add_library(number SHARED number.cpp)

### 2. LINK ###

#dl is needed for dynamic link library code

target_link_libraries(compile exodus stdc++fs ${LIBS})
target_link_libraries(createfile exodus stdc++fs ${LIBS})
target_link_libraries(createindex exodus stdc++fs ${LIBS})
target_link_libraries(delete exodus stdc++fs ${LIBS})
target_link_libraries(clearfile exodus stdc++fs ${LIBS})
target_link_libraries(deletefile exodus stdc++fs ${LIBS})
target_link_libraries(deleteindex exodus stdc++fs ${LIBS})
target_link_libraries(edir exodus stdc++fs ${LIBS})
target_link_libraries(edic exodus stdc++fs ${LIBS})
target_link_libraries(dict2sql exodus stdc++fs ${LIBS})
target_link_libraries(number exodus stdc++fs ${LIBS})
target_link_libraries(exodus-cli exodus stdc++fs ${LIBS})
target_link_libraries(list exodus stdc++fs ${LIBS})
target_link_libraries(listfiles exodus stdc++fs ${LIBS})
target_link_libraries(listdict exodus stdc++fs ${LIBS})
target_link_libraries(listindexes exodus stdc++fs ${LIBS})
target_link_libraries(testsort exodus stdc++fs ${LIBS})
target_link_libraries(configexodus exodus stdc++fs ${LIBS})


### 3. INSTALL BIN ###

install(TARGETS compile
createfile
createindex
delete
clearfile
deletefile
deleteindex
edir
edic
dict2sql
exodus-cli
list
listfiles
listdict
listindexes
testsort
configexodus
 DESTINATION bin)

### 4. INSTALL LIB ###

## CMAKE_PREFIX=/usr/local/
#install(TARGETS dict_myclients DESTINATION lib)
#push it into /usr/lib where it can be found by dlopen
install(TARGETS dict_myclients number DESTINATION /usr/lib)

message("--EXODUS CLI CMAKE EXIT --")
