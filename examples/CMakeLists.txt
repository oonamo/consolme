find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE REFLECTION_SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

if(MSVC)
  set(STRICT_WARNINGS "/WX")
else()
  set(STRICT_WARNINGS
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wshadow"
    "-Wdouble-promotion"
    "-Wno-float-conversion"
    "-Wno-sign-conversion"
    "-Wformat=2"
    "-Wstrict-prototypes"
    "-Wmissing-prototypes"
    "-Wcast-align"
    "-Wundef"
    "-fno-strict-aliasing"
    "-Wnull-dereference"
  )
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h"
  COMMAND Python3::Interpreter "${cmyreflection_SOURCE_DIR}/cmy_reflector.py" "${CMAKE_CURRENT_SOURCE_DIR}/src" -o "${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h"
  DEPENDS "${cmyreflection_SOURCE_DIR}/cmy_reflector.py" ${REFLECTION_SOURCE_FILES}
  COMMENT "Generating C reflection data..."
)

add_executable(example ${REFLECTION_SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h)
target_compile_definitions(raylib PRIVATE SUPPORT_FILEFORMAT_JPG=1)
target_link_libraries(example PUBLIC ${PROJECT_NAME} PRIVATE raylib cmyreflection)
target_include_directories(example PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
target_compile_options(example PRIVATE ${STRICT_WARNINGS})
