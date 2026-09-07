project(consolme C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(RAYLIB_VERSION 6.0)
include(FetchContent)

FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
  cmyreflection
  GIT_REPOSITORY https://github.com/oonamo/CMyReflection.git
  GIT_TAG main
)

FetchContent_MakeAvailable(cmyreflection)

add_library(${PROJECT_NAME} INTERFACE)

if (PROJECT_IS_TOP_LEVEL)
  add_subdirectory(examples)
endif()
