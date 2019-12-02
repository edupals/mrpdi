
find_package(PkgConfig REQUIRED)

add_library(iqboard SHARED iqboard.cpp)
target_link_libraries(iqboard drivercommon)

install(TARGETS iqboard LIBRARY DESTINATION lib/mrpdi)