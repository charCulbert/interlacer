# Interlacer

A command-line tool written in C that uses libTIFF to interlace images horizontally and/or vertically.

## Usage

```bash
./interlacer <file1> <file2> [options]
```

### Options

- `-o <filename>` - Output file (default: interlacedOutput.tif)
- `-r <N>` - Switch images every N rows (default: 1)
- `-c <N>` - Switch images every N columns
- `-r <N> -c <M>` - Switch on both axes

### Examples

```bash
./interlacer a.tif b.tif -r 1         # alternate every 1 rows
./interlacer a.tif b.tif -c 9         # alternate every 9 columns
./interlacer a.tif b.tif -r 2 -c 2    # 2x2 checkerboard
```

Most common tiff file types are handled including various image dimensions, bit depths, and color formats.

A WebAssembly version is in development on the `wasm` branch that compiles with Emscripten.
