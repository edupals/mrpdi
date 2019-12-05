cmake_minimum_required(VERSION 3.0)
project(mrpdi)

add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(gui)
add_subdirectory(drivers)

#desktop
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/desktop/mrpdi.desktop
    DESTINATION usr/share/applications/
)

#icon
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/desktop/mrpdi.svg
    DESTINATION usr/share/icons/hicolor/scalable/apps/
)
