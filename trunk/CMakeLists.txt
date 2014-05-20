#use VERBOSE=1 make to debug

cmake_minimum_required(VERSION 2.8)
set(CPACK_PACKAGE_CONTACT neo)
set(CPACK_PACKAGE_VERSION 14.01.1)
INCLUDE(CPack)

#allow exodus executable and exodus library
#cmake_policy(SET CMP0002 OLD)

#if using custom postgres then configure and build it to /usr/local
#./configure --prefix=/usr/local --eprefix=/usr/local
#otherwise it builds to rather none-standard /usr/local/pgsql
#and exodus make cannot find it easily

#ld -lpq --verbose to debug library paths ... also check /etc/ld.so.conf.d

#for VERY non-standard postgres installation path use
#message("export PostgreSQL_ROOT=/usr/local/pgsql/")
#message("export PATH=$PATH:/usr/local/pgsql/bin/")
#message("export LIBRARY_PATH=/usr/local/pgsql/lib/")

project(exodus)

#execute_process(
#        COMMAND                 pg_config --pkglibdir
#        OUTPUT_VARIABLE         PostgreSQL_PKGLIBDIR
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#message(PostgreSQL_PKGLIBDIR=${PostgreSQL_PKGLIBDIR})

#execute_process(
#        COMMAND                 pg_config --includedir-server
#        OUTPUT_VARIABLE         PostgreSQL_INCLUDEDIR_SERVER
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)
#message(PostgreSQL_INCLUDEDIR_SERVER=${PostgreSQL_INCLUDEDIR_SERVER})

add_subdirectory( exodus/libexodus/exodus)
add_subdirectory( exodus/libpgexodus/src)
add_subdirectory( cli/src)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
