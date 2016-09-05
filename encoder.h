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


#ifndef ENCODER_H
#define ENCODER_H

class Encoder
{
public:
    static unsigned char *compressToJpeg(const unsigned char *bgrx_image_data, int width, int height, int quality, unsigned long long int *out_buffer_size);
    static unsigned char *compressToWebp(const unsigned char *bgrx_image_data, int width, int height, int quality, unsigned long long int *out_buffer_size);
};

#endif // ENCODER_H
