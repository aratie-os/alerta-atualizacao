cmake_minimum_required(VERSION 3.22)

project(system-upgrade VERSION 2.0.3 LANGUAGES CXX)

include(CTest)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Widgets Svg)
find_package(LayerShellQt REQUIRED)

add_library(system_upgrade_logic STATIC
    src/discover_launcher.cpp
    src/discover_launcher.h
    src/startup.cpp
    src/startup.h
    src/update_state.cpp
    src/update_state.h
    src/window_placement.cpp
    src/window_placement.h
)
target_include_directories(system_upgrade_logic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(system_upgrade_logic PUBLIC Qt6::Core Qt6::Gui)

add_executable(system-upgrade
    src/main.cpp
    src/update_checker.cpp
    src/update_checker.h
    src/update_notification.cpp
    src/update_notification.h
)
target_compile_definitions(system-upgrade PRIVATE
    LAYERSHELLQTINTERFACE_NO_DEPRECATED_WARNINGS
)
target_link_libraries(system-upgrade PRIVATE
    system_upgrade_logic
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Svg
    LayerShellQt::Interface
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(system_upgrade_logic PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(system-upgrade PRIVATE -Wall -Wextra -Wpedantic)
endif()

install(TARGETS system-upgrade RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES data/applications/system-upgrade.desktop
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES data/autostart/system-upgrade.desktop
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart")
install(FILES data/metainfo/io.github.aratie_os.system_upgrade.metainfo.xml
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
install(FILES data/system-upgrade.sudoers
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}/sudoers.d"
        RENAME system-upgrade
        PERMISSIONS OWNER_READ GROUP_READ)
install(FILES docs/system-upgrade.1
        DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")

if(BUILD_TESTING)
    find_package(Qt6 6.6 REQUIRED COMPONENTS Test)
    add_executable(updatechecker_tests tests/updatechecker_tests.cpp)
    target_link_libraries(updatechecker_tests PRIVATE
        system_upgrade_logic
        Qt6::Core
        Qt6::Gui
        Qt6::Test
    )
    add_test(NAME updatechecker_tests COMMAND updatechecker_tests)
endif()
