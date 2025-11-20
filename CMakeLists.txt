cmake_minimum_required(VERSION 3.14)
project(interlacer C)

# Build libtiff from source for both native and WASM
# This guarantees identical behavior and removes external dependencies

# Collect libtiff sources
file(GLOB TIFF_SOURCES "tiff-4.6.0/libtiff/*.c")
# Exclude build tools and platform-specific files
list(FILTER TIFF_SOURCES EXCLUDE REGEX "mkg3states\\.c")
list(FILTER TIFF_SOURCES EXCLUDE REGEX "tif_win32\\.c")

# Create libtiff library
add_library(tiff STATIC ${TIFF_SOURCES})
target_include_directories(tiff PUBLIC
  tiff-4.6.0/libtiff
  tiff-4.6.0/port
)

# Check if building for WebAssembly or native
if(EMSCRIPTEN)
  message(STATUS "Building for WebAssembly")
  add_executable(interlacer src/interlacer.c)

  # Emscripten-specific settings
  set_target_properties(interlacer PROPERTIES
    LINK_FLAGS "\
      -s WASM=1 \
      -s ALLOW_MEMORY_GROWTH=1 \
      -s EXPORTED_FUNCTIONS='[\"_interlace_files\",\"_get_tiff_info\",\"_tiff_to_rgba_file\",\"_malloc\",\"_free\"]' \
      -s EXPORTED_RUNTIME_METHODS='[\"FS\",\"ccall\",\"cwrap\",\"getValue\"]' \
      -s MODULARIZE=1 \
      -s EXPORT_NAME='InterlacerModule' \
      -s ENVIRONMENT='web,worker' \
      -s INVOKE_RUN=0 \
      -s EXIT_RUNTIME=0 \
      -O3"
    SUFFIX ".js"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/wasm_build"
  )
else()
  message(STATUS "Building native executable")
  add_executable(interlacer src/interlacer.c)
endif()

# Link libtiff (same for both native and WASM)
target_link_libraries(interlacer PRIVATE tiff)
