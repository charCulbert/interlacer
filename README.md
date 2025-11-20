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
./interlacer a.tif b.tif              # alternate rows
./interlacer a.tif b.tif -c 1         # alternate columns
./interlacer a.tif b.tif -r 2 -c 2    # 2x2 checkerboard
```

The tool handles different image dimensions, bit depths, and color formats automatically.

A WebAssembly version is in development on the `wasm` branch that will compile with Emscripten.
