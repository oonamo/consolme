find_package(Python3 REQUIRED COMPONENTS Interpreter)

file(GLOB_RECURSE REFLECTION_SOURCE_FILES CONFIGURE_DEPENDS "src/*.h" "src/*.c")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h"
  COMMAND Python3::Interpreter "${cmyreflection_SOURCE_DIR}/cmy_reflector.py" "${CMAKE_CURRENT_SOURCE_DIR}/src" -o "${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h"
  DEPENDS "${cmyreflection_SOURCE_DIR}/cmy_reflector.py" ${REFLECTION_SOURCE_FILES}
  COMMENT "Generating C reflection data..."
)

add_executable(example ${REFLECTION_SOURCE_FILES} ${CMAKE_CURRENT_BINARY_DIR}/refl.generated.h)
target_compile_definitions(raylib PRIVATE SUPPORT_FILEFORMAT_JPG=1)
target_link_libraries(example PUBLIC ${PROJECT_NAME} raylib cmyreflection)
target_include_directories(example PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR})
