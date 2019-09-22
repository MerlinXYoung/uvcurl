set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
find_package(PkgConfig)
pkg_check_modules(PC_LIBUV QUIET libuv)

find_library(Libuv_LIBRARY NAMES libuv.so libuv.dylib libuv.dll
    PATHS ${PC_LIBUV_LIBDIR} ${PC_LIBUV_LIBRARY_DIRS})
find_library(Libuv_STATIC_LIBRARY NAMES libuv.a libuv_a.a libuv.dll.a
    PATHS ${PC_LIBUV_LIBDIR} ${PC_LIBUV_LIBRARY_DIRS})
message(STATUS "Libuv_LIBRARY:${Libuv_LIBRARY}")

if( True 
#NOT Libuv_LIBRARY and NOT Libuv_STATIC_LIBRARY 
)
    set(Libuv_FOUND ON)
    set(Libuv_VERSION ${PC_LIBUV_VERSION})
    set(Libuv_INCLUDE_DIR ${PC_LIBUV_INCLUDE_DIRS})
endif()

if( TARGET libuv)
    return()
endif()

add_library(libuv SHARED IMPORTED )
set_property(TARGET libuv PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PC_LIBUV_INCLUDE_DIRS})
set_property(TARGET libuv PROPERTY IMPORTED_LOCATION ${Libuv_LIBRARY})

add_library(libuv-static STATIC IMPORTED )
set_property(TARGET libuv-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PC_LIBUV_INCLUDE_DIRS})
set_property(TARGET libuv-static PROPERTY IMPORTED_LOCATION ${Libuv_STATIC_LIBRARY})