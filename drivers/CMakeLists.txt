 
include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/drivers/common)

add_subdirectory(common)
add_subdirectory(tablet)
add_subdirectory(whiteboard)
add_subdirectory(multiclass)
add_subdirectory(iqboard)
add_subdirectory(promethean)