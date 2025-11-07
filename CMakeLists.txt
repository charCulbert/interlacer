cmake_minimum_required(VERSION 3.14)
project(interlacer C)

add_executable(interlacer src/main.c)

# Directly specify static library paths
set(TIFF_STATIC /opt/homebrew/lib/libtiff.a)
set(JPEG_STATIC /opt/homebrew/lib/libjpeg.a)
set(ZSTD_STATIC /opt/homebrew/lib/libzstd.a)
set(LZMA_STATIC /opt/homebrew/lib/liblzma.a)
set(DEFLATE_STATIC /opt/homebrew/lib/libdeflate.a)

# Link everything static
target_link_libraries(interlacer PRIVATE
  ${TIFF_STATIC}
  ${JPEG_STATIC}
  ${DEFLATE_STATIC}
  ${ZSTD_STATIC}
  ${LZMA_STATIC}
  z  # System zlib (comes with macOS)
  "-framework CoreFoundation"  # Required by some libs
)

# Add include directories
target_include_directories(interlacer PRIVATE /opt/homebrew/include)
