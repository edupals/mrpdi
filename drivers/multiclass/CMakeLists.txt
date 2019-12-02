
find_package(PkgConfig REQUIRED)

add_library(multiclass SHARED multiclass.cpp)
target_link_libraries(multiclass drivercommon)

install(TARGETS multiclass LIBRARY DESTINATION lib/mrpdi)