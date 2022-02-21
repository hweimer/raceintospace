cmake_minimum_required (VERSION 2.8)
project(raceintospace)

enable_testing()

set (raceintospace_VERSION_MAJOR 2)
set (raceintospace_VERSION_MINOR 0)
set (raceintospace_VERSION_PATCH 0)
set (raceintospace_VERSION_EXTRA rc1)
set (raceintospace_VERSION
	"${raceintospace_VERSION_MAJOR}.${raceintospace_VERSION_MINOR}.${raceintospace_VERSION_PATCH}${raceintospace_VERSION_EXTRA}")
if (NOT DEFINED CMAKE_PROJECT_VERSION)
# TODO: use project VERSION & cmake 3.0
  set (CMAKE_PROJECT_VERSION "${raceintospace_VERSION}")
endif (NOT DEFINED CMAKE_PROJECT_VERSION)

# Platform detection
if (WIN32 AND NOT WINDOWS)
  set (WINDOWS TRUE)
endif (WIN32 AND NOT WINDOWS)

if (WINDOWS)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Race Into Space")
endif (WINDOWS)

option(PBEM "Enable experimental Play-by-EMail feature" OFF)

string(TOLOWER "${CMAKE_BUILD_TYPE}" lc_CMAKE_BUILD_TYPE)
if("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_definitions(-DDEBUG)
endif("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")

add_subdirectory(lib)
add_subdirectory(src)
install(PROGRAMS icons/raceintospace.desktop DESTINATION share/applications)
install(FILES icons/raceintospace.xpm icons/raceintospace.png DESTINATION share/pixmaps)
install(FILES doc/org.raceintospace.Raceintospace.metainfo.xml
	DESTINATION share/metainfo)

#set(CPACK_GENERATOR TGZ)
#set(CPACK_SOURCE_GENERATOR TGZ)
#include(CPack)

find_program(GIT git DOC "Git program path")

add_custom_target(dist
	COMMAND ${CMAKE_COMMAND} -D ARCHIVE_PREFIX="${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}" -D SOURCE_DIR="${CMAKE_SOURCE_DIR}" -D OUTPUT_DIR="${CMAKE_BINARY_DIR}" -P ${CMAKE_CURRENT_SOURCE_DIR}/make_archive.cmake
)
add_custom_target(run
	COMMAND ${CMAKE_BINARY_DIR}/src/game/raceintospace BARIS_DATA=${CMAKE_SOURCE_DIR}/data
	DEPENDS raceintospace
	)

if("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_definitions(-DDEBUG)
	add_custom_target(gdb
		COMMAND gdb -ex run --args ${CMAKE_BINARY_DIR}/src/game/raceintospace BARIS_DATA=${CMAKE_SOURCE_DIR}/data
		DEPENDS raceintospace
		)
else("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")
	add_custom_target(gdb
		COMMENT "Set CMAKE_BUILD_TYPE=Debug for debugging information (e.g., \"cmake -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}\")"
		COMMAND false
		)
endif("${lc_CMAKE_BUILD_TYPE}" STREQUAL "debug")

add_custom_target(tag
	COMMAND ${GIT} tag -a v${CMAKE_PROJECT_VERSION})

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Raceintospace Team")
# ldd src/game/raceintospace|sed -e 's/ =.*//g'|xargs dpkg -S|sed -e 's/:.*//g'|sort|uniq|sed -e ':a; N; $!ba; s/\n/, /g'
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libasound2, libasyncns0, libbrotli1, libbsd0, libc6, libcaca0, libcairo2, libcom-err2, libdbus-1-3, libexpat1, libflac8, libfontconfig1, libfreetype6, libgcc-s1, libgcrypt20, libgpg-error0, libgssapi-krb5-2, libjsoncpp24, libk5crypto3, libkeyutils1, libkrb5-3, libkrb5support0, liblz4-1, liblzma5, libmd0, libncursesw6, libnsl2, libogg0, libopus0, libphysfs1, libpixman-1-0, libpng16-16, libpulse0, libsdl1.2debian, libslang2, libsndfile1, libstdc++6, libsystemd0, libtheora0, libtinfo6, libtirpc3, libuuid1, libvorbis0a, libvorbisenc2, libwrap0, libx11-6, libxau6, libxcb1, libxcb-render0, libxcb-shm0, libxdmcp6, libxext6, libxrender1, libzstd1, zlib1g")
include(CPack)
