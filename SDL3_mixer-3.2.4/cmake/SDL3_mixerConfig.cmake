# SDL3_mixer CMake configuration file:
# This file is meant to be placed in a cmake subfolder of SDL3_mixer-devel-3.2.4-VC.zip

include(FeatureSummary)
set_package_properties(SDL3_mixer PROPERTIES
    URL "https://www.libsdl.org/projects/SDL_mixer/"
    DESCRIPTION "SDL_mixer is a sample multi-channel audio mixer library"
)

cmake_minimum_required(VERSION 3.0...3.28)

# Copied from `configure_package_config_file`
macro(check_required_components _NAME)
    foreach(comp ${${_NAME}_FIND_COMPONENTS})
        if(NOT ${_NAME}_${comp}_FOUND)
            if(${_NAME}_FIND_REQUIRED_${comp})
                set(${_NAME}_FOUND FALSE)
            endif()
        endif()
    endforeach()
endmacro()

set(SDL3_mixer_FOUND                TRUE)

set(SDLMIXER_VENDORED              TRUE)

set(SDLMIXER_FLAC_LIBFLAC          FALSE)
set(SDLMIXER_FLAC_DRFLAC           TRUE)

set(SDLMIXER_GME                   FALSE)

set(SDLMIXER_MOD                   TRUE)
set(SDLMIXER_MOD_XMP               TRUE)
set(SDLMIXER_MOD_XMP_LITE          FALSE)

set(SDLMIXER_MP3                   TRUE)
set(SDLMIXER_MP3_DRMP3             TRUE)
set(SDLMIXER_MP3_MPG123            FALSE)

set(SDLMIXER_MIDI                  TRUE)
set(SDLMIXER_MIDI_FLUIDSYNTH       FALSE)
set(SDLMIXER_MIDI_TIMIDITY         TRUE)

set(SDLMIXER_OPUS                  TRUE)

set(SDLMIXER_VORBIS                STB)
set(SDLMIXER_VORBIS_STB            TRUE)
set(SDLMIXER_VORBIS_TREMOR         FALSE)
set(SDLMIXER_VORBIS_VORBISFILE     FALSE)

set(SDLMIXER_WAVE                  TRUE)


if(SDL_CPU_X86)
    set(_sdl3_mixer_arch_subdir "x86")
elseif(SDL_CPU_X64 OR SDL_CPU_ARM64EC)
    set(_sdl3_mixer_arch_subdir "x64")
elseif(SDL_CPU_ARM64)
    set(_sdl3_mixer_arch_subdir "arm64")
else()
    set(SDL3_mixer_FOUND FALSE)
    return()
endif()

set(_sdl3_mixer_incdir      "${CMAKE_CURRENT_LIST_DIR}/../include")
set(_sdl3_mixer_library     "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl3_mixer_arch_subdir}/SDL3_mixer.lib")
set(_sdl3_mixer_dll         "${CMAKE_CURRENT_LIST_DIR}/../lib/${_sdl3_mixer_arch_subdir}/SDL3_mixer.dll")

# All targets are created, even when some might not be requested though COMPONENTS.
# This is done for compatibility with CMake generated SDL3_mixer-target.cmake files.

set(SDL3_mixer_SDL3_mixer-shared_FOUND TRUE)
if(NOT TARGET SDL3_mixer::SDL3_mixer-shared)
    add_library(SDL3_mixer::SDL3_mixer-shared SHARED IMPORTED)
    set_target_properties(SDL3_mixer::SDL3_mixer-shared
        PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_sdl3_mixer_incdir}"
            IMPORTED_IMPLIB "${_sdl3_mixer_library}"
            IMPORTED_LOCATION "${_sdl3_mixer_dll}"
            COMPATIBLE_INTERFACE_BOOL "SDL3_SHARED"
            INTERFACE_SDL3_SHARED "ON"
    )
endif()

set(SDL3_mixer_SDL3_mixer-static_FOUND FALSE)

if(SDL3_mixer_SDL3_mixer-shared_FOUND OR SDL3_mixer_SDL3_mixer-static_FOUND)
    set(SDL3_mixer_SDL3_mixer_FOUND TRUE)
endif()

function(_sdl_create_target_alias_compat NEW_TARGET TARGET)
    if(CMAKE_VERSION VERSION_LESS "3.18")
        # Aliasing local targets is not supported on CMake < 3.18, so make it global.
        add_library(${NEW_TARGET} INTERFACE IMPORTED)
        set_target_properties(${NEW_TARGET} PROPERTIES INTERFACE_LINK_LIBRARIES "${TARGET}")
    else()
        add_library(${NEW_TARGET} ALIAS ${TARGET})
    endif()
endfunction()

# Make sure SDL3_mixer::SDL3_mixer always exists
if(NOT TARGET SDL3_mixer::SDL3_mixer)
    if(TARGET SDL3_mixer::SDL3_mixer-shared)
        _sdl_create_target_alias_compat(SDL3_mixer::SDL3_mixer SDL3_mixer::SDL3_mixer-shared)
    endif()
endif()

unset(_sdl3_mixer_arch_subdir)
unset(_sdl3_mixer_incdir)
unset(_sdl3_mixer_library)
unset(_sdl3_mixer_dll)

check_required_components(SDL3_mixer)
