find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE REFLECTION_SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

if(MSVC)
  set(STRICT_WARNINGS "/WX")
else()
  set(STRICT_WARNINGS
    # "-Wall"
    # "-Wextra"
    # "-Wshadow"
    # "-Wno-float-conversion"
    # "-Wno-sign-conversion"
    # "-Wformat=2"
    # "-Wstrict-prototypes"
    # "-Wmissing-prototypes"
    # "-Wcast-align"
    # "-Wundef"
    # "-fno-strict-aliasing"
    "-Wnull-dereference"
  )
endif()

add_executable(example ${REFLECTION_SOURCE_FILES})
target_sources(example PRIVATE src/main.c src/refl_impl.c)

set(REFL_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h")

cmy_add_reflection(example
    INPUTS "${CMAKE_CURRENT_SOURCE_DIR}/src/types.h"
    OUTPUT "${REFL_OUTPUT}"
    STD_PLUGINS format json
    PLUGINS "${PROJECT_SOURCE_DIR}/plugins/raytypes.py"
)

target_compile_definitions(raylib PRIVATE SUPPORT_FILEFORMAT_JPG=1)
target_link_libraries(example PUBLIC ${PROJECT_NAME} PRIVATE raylib cmyreflection)
target_include_directories(example PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_compile_options(example PRIVATE ${STRICT_WARNINGS})
