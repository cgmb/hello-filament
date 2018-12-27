cmake_minimum_required(VERSION 2.8)
project(FilamentExampleGLFW)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS} SYSTEM)
include_directories(3rdparty/glad/include SYSTEM)
include_directories(.)

# this was just hacked together to make it work
set(FILAMENT_DIR "/home/cgmb/filament/out/release/filament")
set(FILAMENT_INCLUDE_DIRS "${FILAMENT_DIR}/include")
set(FILAMENT_LIB_DIR "${FILAMENT_DIR}/lib/x86_64")
set(FILAMENT_LIBRARIES "${FILAMENT_LIB_DIR}/libfilament.a" "${FILAMENT_LIB_DIR}/libbluegl.a" "${FILAMENT_LIB_DIR}/libbluevk.a" "${FILAMENT_LIB_DIR}/libfilabridge.a" "${FILAMENT_LIB_DIR}/libfilaflat.a" "${FILAMENT_LIB_DIR}/libutils.a")
include_directories(${FILAMENT_INCLUDE_DIRS} SYSTEM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_FORTIFY_SOURCE=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS")
endif()

set(SOURCES
 main.cxx
 native_window.cxx
 3rdparty/glad/src/glad.c
)

add_executable(flGLFW ${SOURCES})
target_link_libraries(flGLFW
  ${FILAMENT_LIBRARIES}
  ${GLFW_STATIC_LIBRARIES}
)
