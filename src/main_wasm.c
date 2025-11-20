#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

uint32_t get_sample_value(uint8_t *data, uint16_t bps, uint32_t sample_index) {
  if (bps <= 8) {
    return data[sample_index];
  } else if (bps <= 16) {
    uint16_t *data16 = (uint16_t *)data;
    return data16[sample_index];
  } else if (bps <= 32) {
    uint32_t *data32 = (uint32_t *)data;
    return data32[sample_index];
  }
  return 0;
}

void set_sample_value(uint8_t *data, uint16_t bps, uint32_t sample_index,
                      uint32_t value) {
  if (bps <= 8) {
    data[sample_index] = value;
  } else if (bps <= 16) {
    uint16_t *data16 = (uint16_t *)data;
    data16[sample_index] = value;
  } else if (bps <= 32) {
    uint32_t *data32 = (uint32_t *)data;
    data32[sample_index] = value;
  }
}

void convert_pixel(uint8_t *src, uint16_t src_bps, uint16_t src_spp,
                   uint8_t *dst, uint16_t dst_bps, uint16_t dst_spp) {
  uint32_t i, value;
  uint32_t src_max = (1 << src_bps) - 1;
  uint32_t dst_max = (1 << dst_bps) - 1;

  for (i = 0; i < dst_spp; i++) {
    if (i < src_spp) {
      value = get_sample_value(src, src_bps, i);
      if (src_bps != dst_bps) {
        value = (value * dst_max + src_max / 2) / src_max;
      }
    } else {
      value = dst_max;
    }
    set_sample_value(dst, dst_bps, i, value);
  }
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
int interlace_files(const char *file1, const char *file2, const char *output,
                    int row_interval, int col_interval) {
  uint32_t w1, h1, w2, h2, outputWidth, outputHeight;
  void *raster1, *raster2, *raster3;
  TIFF *tif1, *tif2, *out;
  uint16_t bps1, bps2, spp1, spp2, photo1, photo2;
  uint16_t output_bps, output_spp, output_photo;
  uint32_t scanlineSize1, scanlineSize2, scanlineSize3;
  uint32_t bytesPerPixel1, bytesPerPixel2, bytesPerPixel3;
  int use_row_mode = (row_interval > 0);
  int use_col_mode = (col_interval > 0);

  printf("Opening %s...\n", file1);
  tif1 = TIFFOpen(file1, "r");
  if (!tif1) {
    printf("Error: could not open %s\n", file1);
    return 1;
  }

  TIFFGetField(tif1, TIFFTAG_IMAGEWIDTH, &w1);
  TIFFGetField(tif1, TIFFTAG_IMAGELENGTH, &h1);
  TIFFGetFieldDefaulted(tif1, TIFFTAG_BITSPERSAMPLE, &bps1);
  TIFFGetFieldDefaulted(tif1, TIFFTAG_SAMPLESPERPIXEL, &spp1);
  TIFFGetFieldDefaulted(tif1, TIFFTAG_PHOTOMETRIC, &photo1);
  scanlineSize1 = TIFFScanlineSize(tif1);

  printf("Loaded %s: %dx%d, %d-bit, %d channels\n", file1, w1, h1, bps1, spp1);

  // Handle palette images
  int converted_to_rgba = 0;
  if (photo1 == PHOTOMETRIC_PALETTE) {
    printf("Converting palette to RGBA...\n");
    raster1 = _TIFFmalloc(w1 * h1 * sizeof(uint32_t));
    if (!raster1) {
      printf("Error: out of memory\n");
      TIFFClose(tif1);
      return 1;
    }

    if (!TIFFReadRGBAImageOriented(tif1, w1, h1, (uint32_t *)raster1, ORIENTATION_TOPLEFT, 0)) {
      printf("Error: failed to read RGBA from %s\n", file1);
      _TIFFfree(raster1);
      TIFFClose(tif1);
      return 1;
    }

    converted_to_rgba = 1;
    bps1 = 8;
    spp1 = 4;
    photo1 = PHOTOMETRIC_RGB;
    scanlineSize1 = w1 * 4;
  } else {
    raster1 = _TIFFmalloc(scanlineSize1 * h1);
    if (!raster1) {
      printf("Error: out of memory\n");
      TIFFClose(tif1);
      return 1;
    }

    for (uint32_t row = 0; row < h1; row++) {
      if (TIFFReadScanline(tif1, (uint8_t *)raster1 + row * scanlineSize1, row, 0) < 0) {
        printf("Error: failed to read scanline from %s\n", file1);
        _TIFFfree(raster1);
        TIFFClose(tif1);
        return 1;
      }
    }
  }

  TIFFClose(tif1);

  printf("Opening %s...\n", file2);
  tif2 = TIFFOpen(file2, "r");
  if (!tif2) {
    printf("Error: could not open %s\n", file2);
    _TIFFfree(raster1);
    return 1;
  }

  TIFFGetField(tif2, TIFFTAG_IMAGEWIDTH, &w2);
  TIFFGetField(tif2, TIFFTAG_IMAGELENGTH, &h2);
  TIFFGetFieldDefaulted(tif2, TIFFTAG_BITSPERSAMPLE, &bps2);
  TIFFGetFieldDefaulted(tif2, TIFFTAG_SAMPLESPERPIXEL, &spp2);
  TIFFGetFieldDefaulted(tif2, TIFFTAG_PHOTOMETRIC, &photo2);
  scanlineSize2 = TIFFScanlineSize(tif2);

  printf("Loaded %s: %dx%d, %d-bit, %d channels\n", file2, w2, h2, bps2, spp2);

  int converted_to_rgba2 = 0;
  if (photo2 == PHOTOMETRIC_PALETTE) {
    printf("Converting palette to RGBA...\n");
    raster2 = _TIFFmalloc(w2 * h2 * sizeof(uint32_t));
    if (!raster2) {
      printf("Error: out of memory\n");
      _TIFFfree(raster1);
      TIFFClose(tif2);
      return 1;
    }

    if (!TIFFReadRGBAImageOriented(tif2, w2, h2, (uint32_t *)raster2, ORIENTATION_TOPLEFT, 0)) {
      printf("Error: failed to read RGBA from %s\n", file2);
      _TIFFfree(raster1);
      _TIFFfree(raster2);
      TIFFClose(tif2);
      return 1;
    }

    converted_to_rgba2 = 1;
    bps2 = 8;
    spp2 = 4;
    photo2 = PHOTOMETRIC_RGB;
    scanlineSize2 = w2 * 4;
  } else {
    raster2 = _TIFFmalloc(scanlineSize2 * h2);
    if (!raster2) {
      printf("Error: out of memory\n");
      _TIFFfree(raster1);
      TIFFClose(tif2);
      return 1;
    }

    for (uint32_t row = 0; row < h2; row++) {
      if (TIFFReadScanline(tif2, (uint8_t *)raster2 + row * scanlineSize2, row, 0) < 0) {
        printf("Error: failed to read scanline from %s\n", file2);
        _TIFFfree(raster1);
        _TIFFfree(raster2);
        TIFFClose(tif2);
        return 1;
      }
    }
  }
  TIFFClose(tif2);

  // Determine output format
  output_bps = bps1 > bps2 ? bps1 : bps2;
  output_spp = spp1 > spp2 ? spp1 : spp2;
  output_photo = photo1;

  if (bps1 != bps2 || spp1 != spp2) {
    printf("Different formats detected, converting to: %d-bit, %d channels\n",
           output_bps, output_spp);
  }

  if (w1 != w2 || h1 != h2) {
    printf("Different dimensions detected, using minimum: %dx%d\n",
           w1 < w2 ? w1 : w2, h1 < h2 ? h1 : h2);
  }

  outputWidth = w1 < w2 ? w1 : w2;
  outputHeight = h1 < h2 ? h1 : h2;

  if (use_row_mode && use_col_mode) {
    printf("Interlacing: rows every %d pixels, columns every %d pixels\n",
           row_interval, col_interval);
  } else if (use_row_mode) {
    printf("Interlacing: rows every %d pixels\n", row_interval);
  } else if (use_col_mode) {
    printf("Interlacing: columns every %d pixels\n", col_interval);
  }

  bytesPerPixel1 = spp1 * ((bps1 + 7) / 8);
  bytesPerPixel2 = spp2 * ((bps2 + 7) / 8);
  bytesPerPixel3 = output_spp * ((output_bps + 7) / 8);
  scanlineSize3 = outputWidth * bytesPerPixel3;

  raster3 = _TIFFmalloc(scanlineSize3 * outputHeight);
  if (!raster3) {
    printf("Error: out of memory\n");
    _TIFFfree(raster1);
    _TIFFfree(raster2);
    return 1;
  }

  printf("Processing...\n");

  for (uint32_t row = 0; row < outputHeight; row++) {
    for (uint32_t col = 0; col < outputWidth; col++) {
      int use_image1;

      if (use_row_mode && use_col_mode) {
        int row_choice = (row / row_interval) & 1;
        int col_choice = (col / col_interval) & 1;
        use_image1 = (row_choice == col_choice);
      } else if (use_row_mode) {
        use_image1 = ((row / row_interval) & 1) == 0;
      } else if (use_col_mode) {
        use_image1 = ((col / col_interval) & 1) == 0;
      } else {
        use_image1 = 1;
      }

      uint8_t *dst =
          (uint8_t *)raster3 + row * scanlineSize3 + col * bytesPerPixel3;

      if (use_image1) {
        uint8_t *src =
            (uint8_t *)raster1 + row * scanlineSize1 + col * bytesPerPixel1;
        convert_pixel(src, bps1, spp1, dst, output_bps, output_spp);
      } else {
        uint8_t *src =
            (uint8_t *)raster2 + row * scanlineSize2 + col * bytesPerPixel2;
        convert_pixel(src, bps2, spp2, dst, output_bps, output_spp);
      }
    }
  }

  printf("Writing output to %s...\n", output);
  out = TIFFOpen(output, "w");
  if (!out) {
    printf("Error: could not create %s\n", output);
    _TIFFfree(raster1);
    _TIFFfree(raster2);
    _TIFFfree(raster3);
    return 1;
  }

  TIFFSetField(out, TIFFTAG_IMAGEWIDTH, outputWidth);
  TIFFSetField(out, TIFFTAG_IMAGELENGTH, outputHeight);
  TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, output_spp);
  TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, output_bps);
  TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(out, TIFFTAG_PHOTOMETRIC, output_photo);
  TIFFSetField(out, TIFFTAG_COMPRESSION, COMPRESSION_NONE);

  for (uint32_t row = 0; row < outputHeight; row++) {
    TIFFWriteScanline(out, (uint8_t *)raster3 + row * scanlineSize3, row, 0);
  }

  TIFFClose(out);

  printf("Success! Output: %dx%d, %d-bit, %d channels\n",
         outputWidth, outputHeight, output_bps, output_spp);

  _TIFFfree(raster1);
  _TIFFfree(raster2);
  _TIFFfree(raster3);

  return 0;
}

int main(int argc, char *argv[]) {
  printf("INTERLACER WASM\n");

  if (argc < 3) {
    printf("Usage: ./interlacer <file1> <file2> [options]\n");
    printf("Options:\n");
    printf("  -o <filename>   output file (default: interlacedOutput.tif)\n");
    printf("  -r <N>          switch images every N rows\n");
    printf("  -c <N>          switch images every N columns\n");
    return 1;
  }

  char *outputFilename = "interlacedOutput.tif";
  int row_interval = 1;
  int col_interval = 0;
  int user_specified_mode = 0;

  for (int i = 3; i < argc; i++) {
    if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
      user_specified_mode = 1;
      row_interval = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      user_specified_mode = 1;
      col_interval = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      outputFilename = argv[i + 1];
      i++;
    }
  }

  if (!user_specified_mode) {
    row_interval = 1;
  }

  return interlace_files(argv[1], argv[2], outputFilename, row_interval, col_interval);
}
