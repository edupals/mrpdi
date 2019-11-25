
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBHIDAPI REQUIRED IMPORTED_TARGET "hidapi-libusb")

add_library(whiteboard SHARED whiteboard.cpp)
target_link_libraries(whiteboard drivercommon PkgConfig::LIBHIDAPI)