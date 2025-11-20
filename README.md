# Interlacer

A command-line tool written in C that uses libTIFF to interlace images horizontally and/or vertically.

<p align="center">
  <img src="https://github.com/user-attachments/assets/5933ba02-7f60-4ef4-be9d-3dc8a868f917" alt="house" width="30%">
  <strong>+</strong>
  <img src="https://github.com/user-attachments/assets/cbc91852-7c95-4233-8bd1-7b21188e4e7e" alt="metal" width="30%">
  <strong>=</strong>
  <img src="https://github.com/user-attachments/assets/0d1f22f5-0a1d-4dbc-a7a3-0d1fd1cbbddf" alt="house+metal" width="30%">
</p>
 
## Usage

```bash![metal](https://github.com/user-attachments/assets/8a12fc7e-b95e-4a03-b43a-c45cd0b3bb34)

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



