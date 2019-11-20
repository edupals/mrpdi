
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET "libusb-1.0")

add_library(mrpdi core.cpp)
target_link_libraries(mrpdi-server PkgConfig::LIBUSB)