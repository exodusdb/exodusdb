cmake_minimum_required(VERSION 2.8)

set(CPACK_PACKAGE_CONTACT neo)
set(CPACK_PACKAGE_VERSION 14.01.1)
INCLUDE(CPack)

#allow exodus executable and exodus library
#cmake_policy(SET CMP0002 OLD)

project(exodus)

execute_process(
        COMMAND                 pg_config --pkglibdir
        OUTPUT_VARIABLE         PostgreSQL_PKGLIBDIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(PostgreSQL_PKGLIBDIR=${PostgreSQL_PKGLIBDIR})

execute_process(
        COMMAND                 pg_config --includedir-server
        OUTPUT_VARIABLE         PostgreSQL_INCLUDEDIR_SERVER
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(PostgreSQL_INCLUDEDIR_SERVER=${PostgreSQL_INCLUDEDIR_SERVER})

add_subdirectory( cli/src)
add_subdirectory( exodus/libexodus/exodus)
add_subdirectory( exodus/libpgexodus/src)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
