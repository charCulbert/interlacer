# Interlacer

TIFF image interlacing tool. Combines two TIFF images by alternating pixels.

## Build

### Native CLI
```bash
mkdir -p build && cd build
cmake ..
make
cd ..
./interlacer input1.tif input2.tif -r 1
```

### WebAssembly
```bash
mkdir -p wasm_build && cd wasm_build
emcmake cmake ..
emmake make
cd ..
python3 -m http.server 8000
# Open http://localhost:8000/web/
```

## Usage

### CLI
```bash
./interlacer <file1> <file2> [options]

Options:
  -o <filename>   output file
  -r <N>          row interval (default: 1)
  -c <N>          column interval (default: 0)

Examples:
  ./interlacer a.tif b.tif                # alternate rows
  ./interlacer a.tif b.tif -c 1           # alternate columns
  ./interlacer a.tif b.tif -r 2 -c 2      # 2x2 checkerboard
```

### Web
Drag and drop two TIFF files, set intervals, process, and download.

## Architecture

- **libtiff 4.6.0** included as source - used for both native and WASM builds
- **src/main.c** - native CLI with colored terminal output
- **src/main_wasm.c** - WASM version exposing `interlace_files()` function
- **web/index.html** - browser UI with TIFF preview support

Same C code, same libtiff, guaranteed identical output.

## Clean
```bash
rm -rf build wasm_build
```
