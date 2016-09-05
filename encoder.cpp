/*
    Copyright 2016 Oleksandr Murashko, John Thomson.
    This file is part of ACACIA Image Processing Tool.

    ACACIA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ACACIA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ACACIA. If not, see <http://www.gnu.org/licenses/>.
*/


#include "encoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#include "webp/encode.h"


#define USE_LIBJPEG_TURBO

#ifndef USE_LIBJPEG_TURBO
    #define USE_LIBJPEG
#endif


unsigned char *Encoder::compressToJpeg(const unsigned char *bgrx_image_data, int width, int height, int quality, unsigned long long *out_buffer_size)
{
    // Input parameters

    const bool optimize_coding = true;    // Huffman code optimization option - very important

#ifdef USE_LIBJPEG_TURBO
    const int num_input_components = 4;
    const J_COLOR_SPACE input_color_space = JCS_EXT_BGRX;
    const unsigned char *image_data = bgrx_image_data;
#endif

#ifdef USE_LIBJPEG
    const int num_input_components = 3;
    const J_COLOR_SPACE input_color_space = JCS_RGB;
    unsigned char *image_data = new unsigned char [width * height * num_input_components];
    int counter = 0;
    for (int pixel_num = 0;  pixel_num < width * height;  pixel_num++)
    {
        const unsigned int pixel = ((unsigned int *) bgrx_image_data) [pixel_num];
        image_data[counter++] = (unsigned char) (pixel >> 16);
        image_data[counter++] = (unsigned char) (pixel >> 8);
        image_data[counter++] = (unsigned char) (pixel);
    }
#endif

    // Initialize compression structure

    struct jpeg_error_mgr jerr;
    struct jpeg_compress_struct cinfo;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    // Specify destination in memory

    unsigned char *out_buffer = NULL;
    jpeg_mem_dest(&cinfo, &out_buffer, (long unsigned int*) out_buffer_size);

    // Set main parameters for compression

    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = num_input_components;
    cinfo.in_color_space = input_color_space;
    jpeg_set_defaults(&cinfo);

    // Set optional parameters

    cinfo.optimize_coding = optimize_coding;
    jpeg_set_quality(&cinfo, quality, true);

    // Start compression

    jpeg_start_compress(&cinfo, true);

    JSAMPROW row_pointer [1];

    const int row_stride = cinfo.image_width * num_input_components;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = (JSAMPLE *) (image_data + cinfo.next_scanline * row_stride);
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    // Image is compressed into memory!

#ifdef USE_LIBJPEG
    delete [] image_data;
#endif

    return out_buffer;
}

unsigned char *Encoder::compressToWebp(const unsigned char *bgrx_image_data, int width, int height, int quality, unsigned long long *out_buffer_size)
{
    unsigned char *compressedImageBuffer = NULL;
    *out_buffer_size = WebPEncodeBGRA((const uint8_t*) bgrx_image_data, width, height, width * 4, (float) quality, (uint8_t **) &compressedImageBuffer);
    return compressedImageBuffer;
}
