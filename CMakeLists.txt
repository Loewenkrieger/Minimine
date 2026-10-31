cmake_minimum_required(VERSION 3.15)
project(CasioSandbox VERSION 0.2.0 LANGUAGES C)

if(NOT "${FXSDK_PLATFORM_LONG}" STREQUAL "fxCP")
  message(FATAL_ERROR "Build for ClassPad II with: fxsdk build-cp")
endif()

include(GenerateHH3)
find_package(Gint 2.11 REQUIRED)

add_executable(casio_sandbox
  src/main.c
  src/game.c
  src/perf.c
  src/render.c
)
set_target_properties(casio_sandbox PROPERTIES
  C_STANDARD 11
  C_STANDARD_REQUIRED YES
  C_EXTENSIONS YES
)
target_include_directories(casio_sandbox PRIVATE src)
target_compile_options(casio_sandbox PRIVATE
  -Wall -Wextra -Wshadow -Os -g
  -ffunction-sections -fdata-sections
)
target_link_options(casio_sandbox PRIVATE
  -Wl,--gc-sections
  -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/CasioSandbox.map
)
target_link_libraries(casio_sandbox Gint::Gint)
generate_hh3(TARGET casio_sandbox
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/CasioSandbox.hh3")

