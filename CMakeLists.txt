cmake_minimum_required(VERSION 3.26)
project(wifiparser)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} `pkg-config --cflags pango gdk-pixbuf-2.0 libnm`" )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} `pkg-config --cflags dbus-1`")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/atk-1.0 -I/usr/include/gtk-3.0 -liw " )
# -I/usr/include/libnma -lnl-route-3 -lnl-route-3 -lnl-nf-3 -lnl-cli-3 -lnl-genl-3 -I/usr/include/libnl3
add_executable(wifiparser
        src/main.cpp
        include/include.hpp
        include/parsing_iwlib.hpp
        include/custom.hpp
        include/dbus_comutator.hpp
        include/parsing_libnm.hpp
)
find_path(LibNL_INCLUDE_DIR method_netlink/method_netlink.h
        /usr/include
        /usr/include/libnl3
        /usr/local/include
        /usr/local/include/libnl3
)
find_path(LibNM_INCLUDE_DIR
        /usr/include
        /usr/include/libnm
)
find_path(DBus_INCLUDE dbus/dbus.h
        /usr/include
        /usr/include/dbus-1.0
)

#dbus/dbus.h
find_package(PkgConfig REQUIRED) # Include functions provided by PkgConfig module.
pkg_check_modules(DBUS REQUIRED dbus-1) # This calls pkgconfig with appropriate arguments
# Use results of pkg_check_modules() call.
include_directories(${DBUS_INCLUDE_DIRS})
link_directories(${DBUS_LIBRARY_DIRS})

#gio/gio.h
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
target_link_libraries(wifiparser PUBLIC PkgConfig::GLIB)

pkg_check_modules(LIBNM REQUIRED IMPORTED_TARGET libnm)
pkg_check_modules(GLIB REQUIRED glib-2.0)
link_libraries( ${LIBNM_LIBRARIES} ${GLIB_LIBRARIES})
include_directories( ${LIBNM_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
add_compile_options( ${LIBNM_CFLAGS_OTHER} ${GLIB_CFLAGS_OTHER})
target_link_libraries(wifiparser PUBLIC PkgConfig::LIBNM)

target_link_libraries(wifiparser PUBLIC ${DBUS_LIBRARIES})
#find_library(LibNL_LIBRARY NAMES nl nl-3)
#find_library(LibNL_ROUTE_LIBRARY NAMES nl-route nl-route-3)
#find_library(LibNL_NETFILTER_LIBRARY NAMES nl-nf nl-nf-3)
#find_library(LibNL_GENL_LIBRARY NAMES nl-genl nl-genl-3)
target_link_libraries(wifiparser PRIVATE /usr/include/libnm)