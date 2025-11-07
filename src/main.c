#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiffio.h>

uint32_t get_sample_value(uint8_t *data, uint16_t bps, uint32_t sample_index) {
  if (bps <= 8) {
    return data[sample_index];
  } else if (bps <= 16) {
    uint16_t *data16 = (uint16_t *)data;
    return data16[sample_index];
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

int main(int argc, char *argv[]) {
  printf("==========INTERLACER==========\n");
  char *outputFilename = "interlacedOutput.tif";
  int use_row_mode = 1;
  int use_col_mode = 0;
  int row_interval = 1;
  int col_interval = 1;
  uint32_t w1, h1, w2, h2, outputWidth, outputHeight;
  void *raster1, *raster2, *raster3;
  TIFF *tif1, *tif2, *out;
  uint16_t bps1, bps2, spp1, spp2, photo1, photo2;
  uint16_t output_bps, output_spp, output_photo;
  uint32_t scanlineSize1, scanlineSize2, scanlineSize3;
  uint32_t bytesPerPixel1, bytesPerPixel2, bytesPerPixel3;
  int i;

  if (argc < 3) {
    printf("usage: ./interlacer <file1> <file2> [options]\n");
    printf(
        "  -o <filename>   output filename (default: interlacedOutput.tif)\n");
    printf("  -r <N>          interlace rows every N rows\n");
    printf("  -c <N>          interlace columns every N columns\n");
    printf("  default: rows every 1 (if no -r or -c specified)\n");
    return 1;
  }

  for (i = 3; i < argc; i++) {
    if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
      use_row_mode = 1;
      row_interval = atoi(argv[i + 1]);
      if (row_interval < 1) {
        row_interval = 1;
        use_row_mode = 0;
      }
      i++;
    } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      use_col_mode = 1;
      col_interval = atoi(argv[i + 1]);
      if (col_interval < 1) {
        col_interval = 1;
        use_col_mode = 0;
      }
      i++;
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      outputFilename = argv[i + 1];
      i++;
    }
  }

  tif1 = TIFFOpen(argv[1], "r");
  if (!tif1) {
    printf("error: could not open %s\n", argv[1]);
    return 1;
  }

  TIFFGetField(tif1, TIFFTAG_IMAGEWIDTH, &w1);
  TIFFGetField(tif1, TIFFTAG_IMAGELENGTH, &h1);
  TIFFGetFieldDefaulted(tif1, TIFFTAG_BITSPERSAMPLE, &bps1);
  TIFFGetFieldDefaulted(tif1, TIFFTAG_SAMPLESPERPIXEL, &spp1);
  TIFFGetFieldDefaulted(tif1, TIFFTAG_PHOTOMETRIC, &photo1);
  scanlineSize1 = TIFFScanlineSize(tif1);

  printf("loaded %s: %dx%d %d-bit %d channels\n", argv[1], w1, h1, bps1, spp1);

  raster1 = _TIFFmalloc(scanlineSize1 * h1);
  if (!raster1) {
    printf("error: out of memory\n");
    TIFFClose(tif1);
    return 1;
  }

  for (uint32_t row = 0; row < h1; row++) {
    if (TIFFReadScanline(tif1, (uint8_t *)raster1 + row * scanlineSize1, row,
                         0) < 0) {
      printf("error: failed to read scanline from %s\n", argv[1]);
      _TIFFfree(raster1);
      TIFFClose(tif1);
      return 1;
    }
  }
  TIFFClose(tif1);

  tif2 = TIFFOpen(argv[2], "r");
  if (!tif2) {
    printf("error: could not open %s\n", argv[2]);
    _TIFFfree(raster1);
    return 1;
  }

  TIFFGetField(tif2, TIFFTAG_IMAGEWIDTH, &w2);
  TIFFGetField(tif2, TIFFTAG_IMAGELENGTH, &h2);
  TIFFGetFieldDefaulted(tif2, TIFFTAG_BITSPERSAMPLE, &bps2);
  TIFFGetFieldDefaulted(tif2, TIFFTAG_SAMPLESPERPIXEL, &spp2);
  TIFFGetFieldDefaulted(tif2, TIFFTAG_PHOTOMETRIC, &photo2);
  scanlineSize2 = TIFFScanlineSize(tif2);

  printf("loaded %s: %dx%d %d-bit %d channels\n", argv[2], w2, h2, bps2, spp2);

  raster2 = _TIFFmalloc(scanlineSize2 * h2);
  if (!raster2) {
    printf("error: out of memory\n");
    _TIFFfree(raster1);
    TIFFClose(tif2);
    return 1;
  }

  for (uint32_t row = 0; row < h2; row++) {
    if (TIFFReadScanline(tif2, (uint8_t *)raster2 + row * scanlineSize2, row,
                         0) < 0) {
      printf("error: failed to read scanline from %s\n", argv[2]);
      _TIFFfree(raster1);
      _TIFFfree(raster2);
      TIFFClose(tif2);
      return 1;
    }
  }
  TIFFClose(tif2);

  output_bps = bps1 > bps2 ? bps1 : bps2;
  output_spp = spp1 > spp2 ? spp1 : spp2;
  output_photo = photo1;

  if (bps1 != bps2 || spp1 != spp2) {
    printf("warning: different formats, converting to %d-bit %d channels\n",
           output_bps, output_spp);
  }

  if (w1 != w2 || h1 != h2) {
    printf("warning: different dimensions, using minimum (%dx%d)\n",
           w1 < w2 ? w1 : w2, h1 < h2 ? h1 : h2);
  }

  outputWidth = w1 < w2 ? w1 : w2;
  outputHeight = h1 < h2 ? h1 : h2;

  if (use_row_mode && use_col_mode) {
    printf("mode: grid (rows every %d, cols every %d)\n", row_interval,
           col_interval);
  } else if (use_row_mode) {
    printf("mode: rows every %d\n", row_interval);
  } else if (use_col_mode) {
    printf("mode: columns every %d\n", col_interval);
  } else {
    printf("not using any interlacing as 0 was specified for both values\n");
  }

  bytesPerPixel1 = spp1 * ((bps1 + 7) / 8);
  bytesPerPixel2 = spp2 * ((bps2 + 7) / 8);
  bytesPerPixel3 = output_spp * ((output_bps + 7) / 8);
  scanlineSize3 = outputWidth * bytesPerPixel3;

  raster3 = _TIFFmalloc(scanlineSize3 * outputHeight);
  if (!raster3) {
    printf("error: out of memory\n");
    _TIFFfree(raster1);
    _TIFFfree(raster2);
    return 1;
  }

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

  out = TIFFOpen(outputFilename, "w");
  if (!out) {
    printf("error: could not create %s\n", outputFilename);
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
  printf("wrote %s (%dx%d %d-bit %d channels)\n", outputFilename, outputWidth,
         outputHeight, output_bps, output_spp);

  _TIFFfree(raster1);
  _TIFFfree(raster2);
  _TIFFfree(raster3);

  return 0;
}
