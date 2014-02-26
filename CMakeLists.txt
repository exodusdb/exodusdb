
cmake_minimum_required(VERSION 2.8)

#allow exodus executable and exodus library
#cmake_policy(SET CMP0002 OLD)

add_subdirectory( cli/src)
add_subdirectory( exodus/libexodus/exodus)
add_subdirectory( exodus/libpgexodus/src)
