#
#========================================================================
#
#                           D O O M  R e t r o
#         The classic, refined DOOM source port. For Windows PC.
#
#========================================================================
#
#  Copyright © 1993-2012 id Software LLC, a ZeniMax Media company.
#  Copyright © 2013-2017 Brad Harding.
#
#  DOOM Retro is a fork of Chocolate DOOM.
#  For a list of credits, see <http://wiki.doomretro.com/credits>.
#
#  This file is part of DOOM Retro.
#
#  DOOM Retro is free software: you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
#
#  DOOM Retro is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with DOOM Retro. If not, see <https://www.gnu.org/licenses/>.
#
#  DOOM is a registered trademark of id Software LLC, a ZeniMax Media
#  company, in the US and/or other countries and is used without
#  permission. All other trademarks are the property of their respective
#  holders. DOOM Retro is in no way affiliated with nor endorsed by
#  id Software.
#
#========================================================================
#

# Use this to build DOOM Retro in Linux or OS X. Then run the executable.

cmake_minimum_required(VERSION 3.6)

project(doomretro LANGUAGES C)

include(GNUInstallDirs)

file(GLOB SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)

add_executable(doomretro ${SOURCES})

if(APPLE)
	target_compile_definitions(doomretro PRIVATE __MACOSX__=1)
	find_library(COCOA_LIBRARY Cocoa)
	set_source_files_properties(
		${CMAKE_SOURCE_DIR}/src/d_main.c
		${CMAKE_SOURCE_DIR}/src/m_misc.c
		COMPILE_FLAGS
		"-x objective-c"
	)
endif (APPLE)

set_property(TARGET doomretro PROPERTY C_STANDARD 99)

find_package(PkgConfig)
pkg_search_module(SDL2 REQUIRED sdl2 IMPORTED_TARGET)
pkg_search_module(SDL2_MIXER REQUIRED SDL2_mixer IMPORTED_TARGET)
pkg_search_module(SDL2_IMAGE REQUIRED SDL2_image IMPORTED_TARGET)

find_library(MATH_LIBRARY m)

target_link_libraries(
	doomretro
	PkgConfig::SDL2
	PkgConfig::SDL2_MIXER
	PkgConfig::SDL2_IMAGE
	${MATH_LIBRARY}
	${COCOA_LIBRARY}
)

file(GLOB RESOURCES LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/res/*)
file(COPY ${RESOURCES} DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS doomretro DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${RESOURCES} DESTINATION ${CMAKE_INSTALL_DATADIR}/doomretro)
