
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHIDAPI REQUIRED IMPORTED_TARGET "hidapi-libusb")

add_library(tablet SHARED tablet.cpp)
target_link_libraries(tablet drivercommon PkgConfig::LIBHIDAPI)