cmake_minimum_required(VERSION 3.0)

project(pscap
	DESCRIPTION "Tool to retrieve informations about the capabilities of a process."
	HOMEPAGE_URL "https://github.com/arthurguyader/pscap"
	VERSION "1.0"
	LANGUAGES C
)

include(GNUInstallDirs)
include(FindPkgConfig)

####################################################################

# Variables

SET(BINDIR "${CMAKE_INSTALL_FULL_BINDIR}" CACHE PATH "binary dir")
SET(DEBUG OFF)

####################################################################

link_libraries(-Wl,--as-needed -Wl,--gc-sections -Wl,--no-undefined)

add_compile_options(-Wall -Wextra -Wconversion -Wshadow)
add_compile_options(-Werror=maybe-uninitialized)
add_compile_options(-Werror=implicit-function-declaration)
add_compile_options(-ffunction-sections -fdata-sections)

# Beware that this can incorporate unwanted behavior !
add_compile_definitions(_FORTIFY_SOURCE=2)
add_compile_options(-Og)

####################################################################

PKG_CHECK_MODULES(libcap REQUIRED libcap)

if(DEBUG)
    message("Debugging mode ...")
    set(CMAKE_BUILD_TYPE "DEBUG")
else()
    set(CMAKE_BUILD_TYPE "RELEASE")
endif()

set(SRCS
    src/pscap.c
)

add_compile_definitions(_GNU_SOURCE)
add_executable(pscap ${SRCS})
target_link_libraries(pscap cap)


install(TARGETS pscap DESTINATION ${BINDIR})