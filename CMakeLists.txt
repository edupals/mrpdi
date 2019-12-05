cmake_minimum_required(VERSION 3.0)
project(mrpdi)

add_subdirectory(lib)
add_subdirectory(cli)
add_subdirectory(gui)
add_subdirectory(drivers)

#desktop
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/desktop/mrpdi.desktop
    DESTINATION share/applications/
)

#icon
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/desktop/mrpdi.svg
    DESTINATION share/icons/hicolor/scalable/apps/
)

#settings directory
install(DIRECTORY DESTINATION /etc/mrpdi)