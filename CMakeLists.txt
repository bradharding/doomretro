#
#========================================================================
#
#                           D O O M  R e t r o
#         The classic, refined DOOM source port. For Windows PC.
#
#========================================================================
#
#  Copyright © 1993-2012 by id Software LLC, a ZeniMax Media company.
#  Copyright © 2013-2019 by Brad Harding.
#
#  DOOM Retro is a fork of Chocolate DOOM. For a list of credits, see
#  <https://github.com/bradharding/doomretro/wiki/CREDITS>.
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

CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(doomretro C)

INCLUDE(GNUInstallDirs)

FILE(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)

IF (APPLE)
	ADD_DEFINITIONS(-D__MACOSX__=1)
	SET(OBJECTIVE_C_FILES
		${CMAKE_SOURCE_DIR}/src/d_main.c
		${CMAKE_SOURCE_DIR}/src/m_misc.c)
	SET_SOURCE_FILES_PROPERTIES(${OBJECTIVE_C_FILES} COMPILE_FLAGS "-x objective-c")
ENDIF (APPLE)

ADD_EXECUTABLE(doomretro ${SOURCES})

SET_TARGET_PROPERTIES(doomretro PROPERTIES LINKER_LANGUAGE C)

INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2_MIXER REQUIRED SDL2_mixer)
PKG_SEARCH_MODULE(SDL2_IMAGE REQUIRED SDL2_image)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src ${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})

IF (APPLE)
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
ENDIF (APPLE)

IF (UNIX)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu99")
ENDIF (UNIX)

TARGET_LINK_LIBRARIES(
	doomretro
	${SDL2_LIBRARIES}
	${SDL2_MIXER_LIBRARIES}
	${SDL2_IMAGE_LIBRARIES}
	${COCOA_LIBRARY}
	m
)

FILE(GLOB RESOURCES LIST_DIRECTORIES false ${CMAKE_SOURCE_DIR}/res/*)
FILE(COPY ${RESOURCES} DESTINATION ${CMAKE_BINARY_DIR})

INSTALL(TARGETS doomretro DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(FILES ${RESOURCES} DESTINATION ${CMAKE_INSTALL_DATADIR}/doomretro)
