cmake_minimum_required(VERSION 2.8)

# declare project configuration
project(libvpvl2)

# library version setting
set(VPVL2_VERSION_MAJOR 0)
set(VPVL2_VERSION_COMPAT 11)
set(VPVL2_VERSION_MINOR 1)

# include external cmake scripts
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/vpvl2.cmake")

# declare options
option(BUILD_SHARED_LIBS "Build Shared Libraries (default is OFF)" OFF)
option(VPVL2_BUILD_QT_RENDERER "Build a renderer program using Qt 4.8 (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_BUILD_IOS "Build a library for iOS with workaround (default is OFF)" OFF)
option(VPVL2_COORDINATE_OPENGL "Use OpenGL coordinate system (default is ON)" ON)

option(VPVL2_ENABLE_NVIDIA_CG "Include OpenGL renderer class using NVIDIA Cg (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENCL "Enable OpenCL acceleration (default is OFF)" OFF)
option(VPVL2_ENABLE_OPENMP "Enable OpenMP acceleration (default is OFF)" OFF)
option(VPVL2_ENABLE_EXTENSIONS_ARCHIVE "Include classes of loading zip files (default is ON)" ON)
option(VPVL2_ENABLE_EXTENSIONS_PROJECT "Include classes of loading and saving project files (default is ON)" ON)
option(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT "Include classes of base implementation of IRenderContext (default is ON)" ON)
option(VPVL2_ENABLE_EXTENSIONS_STRING "Include classes of implementation of IEncoding/IString based on ICU (default is ON)" ON)
option(VPVL2_ENABLE_EXTENSIONS_WORLD "Include classes of physics world for model based on Bullet Physics (default is ON)" ON)
option(VPVL2_ENABLE_GLES2 "Build a library for GLES2 compliant (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required default is OFF)" OFF)
option(VPVL2_ENABLE_OSMESA "Build a library using Offscreen Mesa3D software rasterizer instead of OS provided OpenGL runtime (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_ENABLE_TEST "Build a unit test runtime (enabling VPVL2_LINK_QT is required, default is OFF)" OFF)
option(VPVL2_ENABLE_COMPILER_TLS "Build library with compiler TLS (default is OFF)" OFF)

option(VPVL2_LINK_SDL1 "Link against SDL 1.2 (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_SDL2 "Link against SDL 2.0 (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_ASSIMP "Link against Open Asset Import Library (default is OFF)" OFF)
option(VPVL2_LINK_DEVIL "Link against DevIL (default is OFF)" OFF)
option(VPVL2_LINK_GLEW "Link against GLEW (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is ON)" ON)
option(VPVL2_LINK_NVTT "Link against NVIDIA Texture Tools a.k.a nvtt (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_VPVL "Link against libvpvl (default is ON)" ON)
option(VPVL2_LINK_INTEL_TBB "Link against Intel Threading Building Blocks (default is OFF)" OFF)
option(VPVL2_LINK_SFML "Link against SFML 2.0 (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_EGL "Link against EGL (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT is required, default is OFF)" OFF)
option(VPVL2_LINK_QT "Link against Qt 4.8 (enabling VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT required, default is OFF)" OFF)

option(VPVL2_PLATFORM_EMSCRIPTEN "Build a library for Emscripten (VPVL2_ENABLE_GLES2 is required, default is OFF)" OFF)
option(VPVL2_PLATFORM_RASPBERRY_PI "Build a library for Raspberry Pi (VPVL2_ENABLE_GLES2 is required, default is OFF)" OFF)

# set library version
set(VPVL2_VERSION "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.${VPVL2_VERSION_MINOR}")
set(VPVL2_VERSION_COMPATIBLE "${VPVL2_VERSION_MAJOR}.${VPVL2_VERSION_COMPAT}.0")

# source configuration
file(GLOB vpvl2_sources_core "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cc")
file(GLOB vpvl2_sources_base "${CMAKE_CURRENT_SOURCE_DIR}/src/core/base/*.cc")
source_group("VPVL2 Core Classes" FILES ${vpvl2_sources_core} ${vpvl2_sources_base})
file(GLOB vpvl2_sources_internal "${CMAKE_CURRENT_SOURCE_DIR}/src/core/internal/*.cc")
file(GLOB vpvl2_headers_internal "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/internal/*.h")
source_group("VPVL2 Internal Classes" FILES ${vpvl2_sources_internal} ${vpvl2_headers_internal})
file(GLOB vpvl2_sources_asset "${CMAKE_CURRENT_SOURCE_DIR}/src/core/asset/*.cc")
file(GLOB vpvl2_headers_asset "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/asset/*.h")
source_group("VPVL2 Asset Classes" FILES ${vpvl2_sources_asset} ${vpvl2_headers_asset})
file(GLOB vpvl2_sources_vmd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/vmd/*.cc")
file(GLOB vpvl2_headers_vmd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/vmd/*.h")
source_group("VPVL2 VMD Classes" FILES ${vpvl2_sources_vmd} ${vpvl2_headers_vmd})
file(GLOB vpvl2_sources_mvd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/mvd/*.cc")
file(GLOB vpvl2_headers_mvd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/mvd/*.h")
source_group("VPVL2 MVD Classes" FILES ${vpvl2_sources_mvd} ${vpvl2_headers_mvd})
if(VPVL2_LINK_VPVL)
  file(GLOB vpvl2_sources_pmd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd/*.cc")
  file(GLOB vpvl2_headers_pmd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd/*.h")
else()
  file(GLOB vpvl2_sources_pmd "${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmd2/*.cc")
  file(GLOB vpvl2_headers_pmd "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmd2/*.h")
endif()
source_group("VPVL2 PMD Classes" FILES ${vpvl2_sources_pmd} ${vpvl2_headers_pmd})
file(GLOB vpvl2_sources_pmx "${CMAKE_CURRENT_SOURCE_DIR}/src/core/pmx/*.cc")
file(GLOB vpvl2_headers_pmx "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/pmx/*.h")
source_group("VPVL2 PMX Classes" FILES ${vpvl2_sources_pmx} ${vpvl2_headers_pmx})

list(APPEND vpvl2_sources ${vpvl2_sources_core} ${vpvl2_sources_base} ${vpvl2_sources_internal}
                          ${vpvl2_sources_asset} ${vpvl2_sources_vmd} ${vpvl2_sources_mvd}
                          ${vpvl2_sources_pmd} ${vpvl2_sources_pmx})

# extensions configuration
if(VPVL2_ENABLE_EXTENSIONS_ARCHIVE)
  if(APPLE)
    add_definitions(-DUSE_FILE32API)
  endif()
  file(GLOB vpvl2_sources_archive "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/Archive.cc")
  if(BUILD_SHARED_LIBS OR MSVC)
    file(GLOB vpvl2_sources_minizip "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/minizip/*")
  endif()
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/minizip")
  source_group("VPVL2 Archive Classes" FILES ${vpvl2_sources_archive} ${vpvl2_sources_minizip})
  list(APPEND vpvl2_sources ${vpvl2_sources_archive} ${vpvl2_sources_minizip})
endif()

if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT)
  file(GLOB vpvl2_sources_render_context "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/BaseRenderContext.cc")
  file(GLOB vpvl2_headers_render_context "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/BaseRenderContext.h")
  source_group("VPVL2 RenderContext Classes" FILES ${vpvl2_sources_render_context} ${vpvl2_headers_render_context})
  list(APPEND vpvl2_sources ${vpvl2_sources_render_context} ${vpvl2_headers_render_context})
endif()

if(VPVL2_ENABLE_EXTENSIONS_PROJECT)
  file(GLOB vpvl2_sources_project "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/Project.cc")
  file(GLOB vpvl2_headers_project "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/Project.h")
  source_group("VPVL2 Project Classes" FILES ${vpvl2_sources_project} ${vpvl2_headers_project})
  list(APPEND vpvl2_sources ${vpvl2_sources_project} ${vpvl2_headers_project})
endif()

if(VPVL2_ENABLE_EXTENSIONS_STRING)
  file(GLOB vpvl2_sources_string "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/Encoding.cc"
                                 "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/String*.cc")
  file(GLOB vpvl2_headers_string "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/icu/*.h")
  source_group("VPVL2 String Classes" FILES ${vpvl2_sources_string} ${vpvl2_headers_string})
  list(APPEND vpvl2_sources ${vpvl2_sources_string} ${vpvl2_headers_string})
endif()

if(VPVL2_ENABLE_EXTENSIONS_WORLD)
  file(GLOB vpvl2_sources_world "${CMAKE_CURRENT_SOURCE_DIR}/src/ext/World.cc")
  file(GLOB vpvl2_headers_world "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/World.h")
  source_group("VPVL2 World Classes" FILES ${vpvl2_sources_world} ${vpvl2_headers_world})
  list(APPEND vpvl2_sources ${vpvl2_sources_world} ${vpvl2_headers_world})
endif()

file(GLOB vpvl2_public_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/*.h")
file(GLOB_RECURSE vpvl2_public_headers_extensions "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/*.h")
list(APPEND vpvl2_internal_headers ${vpvl2_headers_internal} ${vpvl2_headers_asset} ${vpvl2_headers_vmd}
                                   ${vpvl2_headers_mvd} ${vpvl2_headers_pmd} ${vpvl2_headers_pmx})

# dynamic library or static library
if(NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE "Release")
endif()
if(BUILD_SHARED_LIBS)
  set(VPVL2_LIB_TYPE SHARED)
else()
  set(VPVL2_LIB_TYPE STATIC)
endif()

# intercept to add source (OpenGL)
if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT)
  file(GLOB vpvl2_sources_engines_gl "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/gl2/*.cc")
  file(GLOB vpvl2_headers_engines_gl "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/gl2/*.h")
  source_group("VPVL2 sources for OpenGL Render Engines" FILES ${vpvl2_sources_engines_gl} ${vpvl2_headers_engines_gl})
  list(APPEND vpvl2_internal_headers ${vpvl2_headers_engines_gl})
  list(APPEND vpvl2_sources ${vpvl2_sources_engines_gl})
  if(VPVL2_ENABLE_NVIDIA_CG)
    file(GLOB vpvl2_sources_engines_cg "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cg/*.cc")
    file(GLOB vpvl2_headers_engines_cg "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cg/*.h")
    source_group("VPVL2 sources for OpenGL Render Engines with Cg" FILES ${vpvl2_sources_engines_cg} ${vpvl2_headers_engines_cg})
    list(APPEND vpvl2_internal_headers ${vpvl2_headers_engines_cg})
    list(APPEND vpvl2_sources ${vpvl2_sources_engines_cg})
  endif()
  if(VPVL2_ENABLE_OPENCL)
    file(GLOB vpvl2_sources_accelerator_cl "${CMAKE_CURRENT_SOURCE_DIR}/src/engine/cl/*.cc")
    file(GLOB vpvl2_headers_accelerator_cl "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/cl/*.h")
    source_group("VPVL2 sources for OpenCL Accelerator" FILES ${vpvl2_sources_accelerator_cl} ${vpvl2_headers_accelerator_cl})
    list(APPEND vpvl2_internal_headers ${vpvl2_headers_accelerator_cl})
    list(APPEND vpvl2_sources ${vpvl2_sources_accelerator_cl})
  endif()
  set(VPVL2_COORDINATE_OPENGL ON BOOL)
endif()

# check compiler static tls
if(VPVL2_ENABLE_COMPILER_TLS)
  include(CheckCXXSourceCompiles)
  check_cxx_source_compiles("__thread int i; int main() { return 0; }" VPVL2_HAS_STATIC_TLS_GNU)
  if(NOT VPVL2_HAS_STATIC_TLS_GNU)
    check_cxx_source_compiles("__declspec(thread) int i; int main() { return 0; }" VPVL2_HAS_STATIC_TLS_MSVC)
  endif()
endif()

# generate configuration file
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")
set(vpvl2_public_headers ${vpvl2_public_headers} "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2/config.h")

# create libvpvl2 project
add_library(vpvl2 ${VPVL2_LIB_TYPE} ${vpvl2_sources} ${vpvl2_public_headers}
                  ${vpvl2_public_headers_extensions} ${vpvl2_internal_headers})
set(VPVL2_OUTPUT_NAME "vpvl2")
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")

# configure library properties
set_target_properties(vpvl2 PROPERTIES VERSION ${VPVL2_VERSION} SOVERSION ${VPVL2_VERSION_COMPATIBLE})
set_target_properties(vpvl2 PROPERTIES OUTPUT_NAME ${VPVL2_OUTPUT_NAME})
vpvl2_set_library_properties(vpvl2 ${vpvl2_public_headers})
vpvl2_set_warnings()

# project include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
# this for vpvl2/config.h
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# find and link libraries to libvpvl2
vpvl2_find_all()
if(BUILD_SHARED_LIBS OR MSVC)
  vpvl2_link_all(vpvl2)
endif()

# link against Qt
if(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT AND (VPVL2_BUILD_QT_RENDERER OR VPVL2_LINK_QT))
  find_package(Qt5Core QUIET)
  if (Qt5Core_FOUND)
    qt5_use_modules(vpvl2 OpenGL)
    qt5_add_resources(vpvl2qtcommon_rc_src "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/resources/libvpvl2qtcommon.qrc")
  else()
    find_package(Qt4 4.8 REQUIRED QtCore QtGui QtOpenGL)
    include(${QT_USE_FILE})
    target_link_libraries(vpvl2 ${QT_LIBRARIES})
    qt4_add_resources(vpvl2qtcommon_rc_src "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/resources/libvpvl2qtcommon.qrc")
  endif()
  file(GLOB vpvl2qtcommon_sources_common "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/common/*.cc")
  file(GLOB vpvl2qtcommon_sources_unzip "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/unzip/*.c")
  file(GLOB vpvl2qtcommon_headers_extensions "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/extensions/details/*.h")
  file(GLOB vpvl2qtcommon_headers "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt/*.h")
  source_group("VPVL2 for Qt sources" FILES ${vpvl2qtcommon_sources_common}
                                            ${vpvl2qtcommon_sources_unzip}
                                            ${vpvl2qtcommon_headers_extensions}
                                            ${vpvl2qtcommon_headers})

  set(vpvl2_qt_sources "${CMAKE_CURRENT_SOURCE_DIR}/render/qt/main.cc"
                       "${CMAKE_CURRENT_SOURCE_DIR}/render/qt/UI.cc")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/render/qt"
                      "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt")
  if(APPLE)
    if (NOT Qt5Core_FOUND)
      find_library(COCOA_LIBRARY Cocoa)
      find_path(COCOA_INCLUDE_DIR Cocoa/Cocoa.h)
      file(GLOB vpvl2qtcommon_sources_osx "${CMAKE_CURRENT_SOURCE_DIR}/src/qt/osx/*.mm")
      include_directories(${COCOA_INCLUDE_DIR})
    endif()
  endif()
  add_library(vpvl2qtcommon ${VPVL2_LIB_TYPE} ${vpvl2qtcommon_sources_common} ${vpvl2qtcommon_sources_unzip}
                                              ${vpvl2qtcommon_headers} ${vpvl2qtcommon_headers_extensions}
                                              ${vpvl2qtcommon_sources_osx} ${vpvl2qtcommon_rc_src})
  vpvl2_set_library_properties(vpvl2qtcommon ${vpvl2qtcommon_headers})
  if(APPLE)
    target_link_libraries(vpvl2qtcommon ${COCOA_LIBRARY})
  elseif(MSVC)
    find_package(ZLIB)
    include_directories(${ZLIB_INCLUDE_DIRS})
    target_link_libraries(vpvl2qtcommon ${ZLIB_LIBRARIES})
  endif()
  target_link_libraries(vpvl2qtcommon vpvl2)
  if (Qt5Core_FOUND)
    qt5_use_modules(vpvl2qtcommon OpenGL Concurrent)
  else()
    target_link_libraries(vpvl2qtcommon vpvl2 ${QT_LIBRARIES})
  endif()
  set(VPVL2QTCOMMON_OUTPUT_NAME "vpvl2qtcommon")
  set_target_properties(vpvl2qtcommon PROPERTIES OUTPUT_NAME ${VPVL2QTCOMMON_OUTPUT_NAME}
                                                 VERSION ${VPVL2_VERSION}
                                                 SOVERSION ${VPVL2_VERSION_COMPATIBLE})
  if(VPVL2_BUILD_QT_RENDERER)
    set(VPVL2_EXECUTABLE vpvl2_qt)
    add_executable(${VPVL2_EXECUTABLE} ${vpvl2_qt_sources} ${vpvl2_internal_headers})
    add_dependencies(${VPVL2_EXECUTABLE} vpvl2qtcommon)
    vpvl2_link_alsoft(${VPVL2_EXECUTABLE})
    vpvl2_link_alure(${VPVL2_EXECUTABLE})
    if (Qt5Core_FOUND)
      qt5_use_modules(${VPVL2_EXECUTABLE} OpenGL Concurrent Widgets)
    else()
      target_link_libraries(${VPVL2_EXECUTABLE} vpvl2 vpvl2qtcommon ${QT_LIBRARIES})
    endif()
  endif()
elseif(VPVL2_ENABLE_EXTENSIONS_RENDERCONTEXT)
  if(VPVL2_LINK_SDL1 OR VPVL2_LINK_SDL2)
    if(VPVL2_LINK_SDL2)
      find_library(SDLIMAGE_LIBRARY SDL2_image REQUIRED)
      find_program(SDL2_CONFIG_SCRIPT sdl2-config REQUIRED)
      exec_program(${SDL2_CONFIG_SCRIPT} ARGS --cflags OUTPUT_VARIABLE SDL2_CFLAGS)
      exec_program(${SDL2_CONFIG_SCRIPT} ARGS --static-libs OUTPUT_VARIABLE SDL_LIBRARY)
      string(REGEX MATCHALL "-I[^ ]*" SDL2_INCLUDE_DIR ${SDL2_CFLAGS})
      string(REGEX MATCHALL "-D[^ ]*" SDL2_DEFINITIONS ${SDL2_CFLAGS})
      string(REPLACE "-I" "" SDL2_INCLUDE_DIR "${SDL2_INCLUDE_DIR}")
      add_definitions(${SDL2_DEFINITIONS})
      include_directories(${SDL2_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
    elseif(VPVL2_LINK_SDL1)
      find_package(SDL REQUIRED)
      find_package(SDL_image REQUIRED)
      include_directories(${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR})
      if(APPLE)
        list(APPEND vpvl2_sdl_sources "render/sdl/SDLMain.m" "render/sdl/SDLMain.h")
      endif()
    endif()
    #find_package(Boost 1.35.0)
    #if(Boost_FOUND)
    #  include_directories(${Boost_INCLUDE_DIR})
    #endif()
    set(vpvl2_sdl_sources "render/sdl/main.cc")
    set(VPVL2_EXECUTABLE vpvl2_sdl)
    add_executable(${VPVL2_EXECUTABLE} ${vpvl2_sdl_sources})
    target_link_libraries(${VPVL2_EXECUTABLE} ${SDL_LIBRARY} ${SDLIMAGE_LIBRARY})
  elseif(VPVL2_LINK_SFML)
    vpvl2_find_sfml()
    set(vpvl2_sfml_sources "render/sfml/main.cc")
    set(VPVL2_EXECUTABLE vpvl2_sfml)
    add_executable(${VPVL2_EXECUTABLE} ${vpvl2_sfml_sources})
    target_link_libraries(${VPVL2_EXECUTABLE} ${SFML_GRAPHICS_LIBRARY}
                                              ${SFML_WINDOW_LIBRARY}
                                              ${SFML_SYSTEM_LIBRARY})
    if(APPLE AND NOT VPVL2_LINK_GLEW)
      find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
      find_path(COREFOUNDATION_INCLUDE_PATH "CoreFoundation/CoreFoundation.h")
      set(VPVL2_EXECUTABLE vpvl2_sfml)
      target_link_libraries(${VPVL2_EXECUTABLE} ${COREFOUNDATION_FRAMEWORK})
      include_directories(${COREFOUNDATION_INCLUDE_PATH})
    endif()
  elseif(VPVL2_LINK_EGL)
    find_path(EGL_INCLUDE_DIR EGL/egl.h)
    find_library(EGL_LIBRARY EGL)
    set(vpvl2_egl_sources "render/egl/main.cc")
    set(VPVL2_EXECUTABLE vpvl2_egl)
    add_executable(${VPVL2_EXECUTABLE} ${vpvl2_egl_sources})
    target_link_libraries(${VPVL2_EXECUTABLE} ${EGL_LIBRARY})
    include_directories(${EGL_INCLUDE_DIR})
    if(VPVL2_PLATFORM_RASPBERRY_PI)
      find_path(VCOS_INCLUDE_DIR vcos_platform_types.h)
      find_library(BCM_HOST_LIBRARY bcm_host)
      find_library(VCOS_LIBRARY vcos)
      find_library(VCHIQ_ARM_LIBRARY vchiq_arm)
      include_directories(${VCOS_INCLUDE_DIR})
      target_link_libraries(${VPVL2_EXECUTABLE} ${BCM_HOST_LIBRARY} ${VCOS_LIBRARY} ${VCHIQ_ARM_LIBRARY})
    endif()
  endif()
endif()

if(VPVL2_EXECUTABLE)
  add_dependencies(${VPVL2_EXECUTABLE} vpvl2)
endif()

# build test
if(VPVL2_LINK_QT AND VPVL2_ENABLE_TEST)
  file(GLOB vpvl2_test_sources "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cc")
  source_group("VPVL2 Test Case Classes" FILES ${vpvl2_test_sources})
  file(GLOB vpvl2_mock_headers "${CMAKE_CURRENT_SOURCE_DIR}/test/mock/*.h")
  source_group("VPVL2 Mock Classes" FILES ${vpvl2_mock_headers})
  file(GLOB gtest_source "${CMAKE_CURRENT_SOURCE_DIR}/test/gtest-1.6.0/src/gtest-all.cc")
  file(GLOB gmock_source "${CMAKE_CURRENT_SOURCE_DIR}/test/gmock-1.6.0/src/gmock-all.cc")
  qt4_add_resources(vpvl2_test_qrc "${CMAKE_CURRENT_SOURCE_DIR}/test/fixtures.qrc")
  add_executable(vpvl2_test ${vpvl2_test_sources} ${vpvl2_mock_headers} ${vpvl2_test_qrc}
                            ${gtest_source} ${gmock_source})
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/test/gtest-1.6.0")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/test/gtest-1.6.0/include")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/test/gmock-1.6.0")
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/test/gmock-1.6.0/include")
  target_link_libraries(vpvl2_test vpvl2qtcommon vpvl2 ${QT_LIBRARIES})
  add_dependencies(vpvl2_test vpvl2 vpvl2qtcommon)
  vpvl2_link_all(vpvl2_test)
endif()

# generate pkg-config
if(NOT WIN32)
  set(PKGCONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/pkgconfig" CACHE STRING
      "Base directory for pkgconfig files")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/vpvl2.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc" @ONLY)
  install(FILES "${CMAKE_CURRENT_BINARY_DIR}/vpvl2.pc" DESTINATION "${PKGCONFIG_INSTALL_PREFIX}")
endif()

# install configuration
install(TARGETS vpvl2 DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
if(vpvl2qtcommon_sources_common)
  install(TARGETS vpvl2qtcommon DESTINATION "${CMAKE_INSTALL_PREFIX}/lib")
endif()
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/include/vpvl2" DESTINATION "${CMAKE_INSTALL_PREFIX}/include")
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2" DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
        PATTERN "*.h"
        PATTERN ".svn" EXCLUDE
        PATTERN "config.h.in" EXCLUDE
        PATTERN "internal" EXCLUDE
        PATTERN "asset" EXCLUDE
        PATTERN "vmd" EXCLUDE
        PATTERN "mvd" EXCLUDE
        PATTERN "pmd" EXCLUDE
        PATTERN "pmd2" EXCLUDE
        PATTERN "pmx" EXCLUDE
        PATTERN "cg" EXCLUDE
        PATTERN "cl" EXCLUDE
        PATTERN "gl2" EXCLUDE
        PATTERN "qt" EXCLUDE)
if(VPVL2_LINK_QT)
  install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/vpvl2/qt"
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/vpvl2")
endif()
