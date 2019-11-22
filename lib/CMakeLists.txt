
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED IMPORTED_TARGET "libusb-1.0")
pkg_check_modules(X11 REQUIRED IMPORTED_TARGET "x11")
pkg_check_modules(XRANDR REQUIRED IMPORTED_TARGET "xrandr")

add_library(mrpdi SHARED core.cpp basedriver.cpp input.cpp utils.cpp)
target_link_libraries(mrpdi PkgConfig::X11 PkgConfig::XRANDR PkgConfig::LIBUSB)
