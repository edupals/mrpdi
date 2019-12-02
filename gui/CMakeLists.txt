
include_directories(${CMAKE_SOURCE_DIR}/lib)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET "gtkmm-3.0")
pkg_check_modules(GIOMM REQUIRED IMPORTED_TARGET "giomm-2.4")
pkg_check_modules(GLIBMM REQUIRED IMPORTED_TARGET "glibmm-2.4")
pkg_check_modules(LIBCONFIGCPP REQUIRED IMPORTED_TARGET "libconfig++")
pkg_check_modules(APPINDICATOR REQUIRED IMPORTED_TARGET "appindicator3-0.1")


add_custom_command(OUTPUT gresources.h COMMAND glib-compile-resources --generate-header --target=$ ${CMAKE_CURRENT_BINARY_DIR}/gresources.h ${CMAKE_CURRENT_SOURCE_DIR}/mrpdi-gui.xml WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_command(OUTPUT gresources.c COMMAND glib-compile-resources --generate-source --target=$ ${CMAKE_CURRENT_BINARY_DIR}/gresources.c ${CMAKE_CURRENT_SOURCE_DIR}/mrpdi-gui.xml WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(resources ALL DEPENDS gresources.h gresources.c)

set_source_files_properties(gresources.c PROPERTIES GENERATED TRUE)

add_executable(mrpdi-gui main.cpp gresources.c)
target_link_libraries(mrpdi-gui mrpdi PkgConfig::LIBCONFIGCPP PkgConfig::GTKMM PkgConfig::GIOMM PkgConfig::GLIBMM PkgConfig::APPINDICATOR)

install(TARGETS mrpdi-gui RUNTIME DESTINATION bin)