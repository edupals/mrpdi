include_directories(${CMAKE_SOURCE_DIR}/lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCONFIGCPP REQUIRED IMPORTED_TARGET "libconfig++")

add_executable(mrpdi-cli cli.cpp)
target_link_libraries(mrpdi-cli mrpdi PkgConfig::LIBCONFIGCPP)
install(TARGETS mrpdi-cli RUNTIME DESTINATION bin)
