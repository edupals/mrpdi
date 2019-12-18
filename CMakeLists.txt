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

#policy kit
install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/polkit/lliurex.mrpdi.gui.policy
    DESTINATION share/polkit-1/actions/
)

install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/polkit/lliurex.mrpdi.gui.pkla
    DESTINATION /var/lib/polkit-1/localauthority/10-vendor.d/
)