#!/bin/bash

# Build script for compiling interlacer to WebAssembly
# Following the approach from MDN's WebAssembly guide

set -e

echo "Building interlacer for WebAssembly..."

# Create build directory
mkdir -p wasm_build

# Compile with Emscripten, including all libtiff source files
# Similar to: emcc -O3 -s WASM=1 webp.c libwebp/src/{dec,dsp}/*.c
emcc -O3 \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_FUNCTIONS='["_main","_interlace_files","_malloc","_free"]' \
  -s EXPORTED_RUNTIME_METHODS='["FS","callMain","ccall","cwrap"]' \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='InterlacerModule' \
  -s ENVIRONMENT='web,worker' \
  -s INVOKE_RUN=0 \
  -s EXIT_RUNTIME=0 \
  -I tiff-4.6.0/libtiff \
  -I tiff-4.6.0/port \
  src/main_wasm.c \
  $(ls tiff-4.6.0/libtiff/*.c | grep -v mkg3states | grep -v tif_win32 | tr '\n' ' ') \
  -o wasm_build/interlacer.js \
  2>&1 | tee wasm_build/build.log

if [ ${PIPESTATUS[0]} -eq 0 ]; then
  echo ""
  echo "✓ Build complete! Output files:"
  ls -lh wasm_build/interlacer.js wasm_build/interlacer.wasm
  echo ""
  echo "To test, run: python3 -m http.server 8000"
  echo "Then open: http://localhost:8000/test.html"
else
  echo ""
  echo "✗ Build failed. Check wasm_build/build.log for details."
  exit 1
fi
