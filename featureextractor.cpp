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


#include "featureextractor.h"
#include "immintrin.h"
#include "math.h"

typedef __m256i int32x8;

/**
 * @brief G1x1 used to calculate features F1, F4
 */
static inline void G1x1(const int32x8 *Y, unsigned int *absSum, unsigned int *sqrSum)
{
    // Initialize accumulators

    int32x8 absSum8 = _mm256_setzero_si256();
    int32x8 sqrSum8 = _mm256_setzero_si256();

    // Loop for differences in rows and columns simultaneously

    for (int i = 0;  i < 8;  )  // fragmentSize = 8
    {
        const int32x8 row_hi = Y[i++];
        const int32x8 row_lo = Y[i++];

        const int32x8 dh = _mm256_hsub_epi32(row_hi, row_lo);
        const int32x8 dv = _mm256_sub_epi32(row_hi, row_lo);

        absSum8 += _mm256_add_epi32(_mm256_abs_epi32(dh), _mm256_abs_epi32(dv));
        sqrSum8 += _mm256_add_epi32(_mm256_mullo_epi32(dh, dh), _mm256_mullo_epi32(dv, dv));
    }

    // Sum the vector components

    const int32x8 sum_aassaass = _mm256_hadd_epi32(absSum8, sqrSum8);
    const int32x8 sum_as__as__ = _mm256_hadd_epi32(sum_aassaass, sum_aassaass);

    *absSum = _mm256_extract_epi32(sum_as__as__, 0) + _mm256_extract_epi32(sum_as__as__, 4);
    *sqrSum = _mm256_extract_epi32(sum_as__as__, 1) + _mm256_extract_epi32(sum_as__as__, 5);
}


/**
 * @brief G2x2 used to calculate features F2, F5
 */
static inline void G2x2(const int32x8 *Y, unsigned int *absSum, unsigned int *sqrSum)
{
    const int32x8 row0 = _mm256_add_epi32(Y[0], Y[1]);
    const int32x8 row1 = _mm256_add_epi32(Y[2], Y[3]);
    const int32x8 row2 = _mm256_add_epi32(Y[4], Y[5]);
    const int32x8 row3 = _mm256_add_epi32(Y[6], Y[7]);

    // Horisontal differences

    const int32x8 row0_hadd_row1 = _mm256_hadd_epi32(row0, row1);
    const int32x8 row2_hadd_row3 = _mm256_hadd_epi32(row2, row3);

    const int32x8 hor_differences = _mm256_hsub_epi32(row0_hadd_row1, row2_hadd_row3);

    // Vertical differences

    const int32x8 row0_sub_row1 = _mm256_sub_epi32(row0, row1);
    const int32x8 row2_sub_row3 = _mm256_sub_epi32(row2, row3);

    const int32x8 ver_differences = _mm256_hadd_epi32(row0_sub_row1, row2_sub_row3);

    // Absolute and squared differences

    const int32x8 absSum8 = _mm256_add_epi32(_mm256_abs_epi32(hor_differences), _mm256_abs_epi32(ver_differences));
    const int32x8 sqrSum8 = _mm256_add_epi32(_mm256_mullo_epi32(hor_differences, hor_differences), _mm256_mullo_epi32(ver_differences, ver_differences));

    // Sum the vector components

    const int32x8 sum_aassaass = _mm256_hadd_epi32(absSum8, sqrSum8);
    const int32x8 sum_as__as__ = _mm256_hadd_epi32(sum_aassaass, sum_aassaass);

    *absSum = _mm256_extract_epi32(sum_as__as__, 0) + _mm256_extract_epi32(sum_as__as__, 4);
    *sqrSum = _mm256_extract_epi32(sum_as__as__, 1) + _mm256_extract_epi32(sum_as__as__, 5);
}


/**
 * @brief G4x4 used to calculate features F3, F6
 */
static inline void G4x4(const int32x8 *Y, unsigned int *absSum, unsigned int *sqrSum)
{
    const int32x8 row0 = Y[0] + Y[1] + Y[2] + Y[3];  // sum_ab_128
    const int32x8 row1 = Y[4] + Y[5] + Y[6] + Y[7];  // sum_cd_128

    const int32x8 sum_aaccbbdd = _mm256_hadd_epi32(row0, row1);
    const int32x8 sum_aabbccdd = _mm256_permute4x64_epi64(sum_aaccbbdd, 216);  // acbd -> abcd

    const int32x8 sum_acabbdcd_32 = _mm256_hadd_epi32(sum_aaccbbdd, sum_aabbccdd);

    const int32x8 differences = _mm256_permute4x64_epi64(_mm256_hsub_epi32(sum_acabbdcd_32, sum_acabbdcd_32), 216);  // only lower 128 bit store data
    const int32x8 abs_differences = _mm256_abs_epi32(differences);
    const int32x8 sqr_differences = _mm256_mullo_epi32(differences, differences);

    const int32x8 sum_aass____ = _mm256_hadd_epi32(abs_differences, sqr_differences);
    const int32x8 sum_as______ = _mm256_hadd_epi32(sum_aass____, sum_aass____);

    *absSum = _mm256_extract_epi32(sum_as______, 0);
    *sqrSum = _mm256_extract_epi32(sum_as______, 1);
}


/**
 * @brief D2x2 used to calculate feature F7
 */
inline unsigned int D2x2(const int32x8 *Y)
{
    // Process 4 rows in one operation

    const int32x8 sum = _mm256_abs_epi32(_mm256_sub_epi32(_mm256_hsub_epi32(Y[0], Y[2]), _mm256_hsub_epi32(Y[1], Y[3]))) +
                        _mm256_abs_epi32(_mm256_sub_epi32(_mm256_hsub_epi32(Y[4], Y[6]), _mm256_hsub_epi32(Y[5], Y[7])));

    // Sum all components of the vector

    const int32x8 sum_xx__xx__ = _mm256_hadd_epi32(sum, sum);
    const int32x8 sum_x___x___ = _mm256_hadd_epi32(sum_xx__xx__, sum_xx__xx__);

    return _mm256_extract_epi32(sum_x___x___, 0) + _mm256_extract_epi32(sum_x___x___, 4);
}


/**
 * @brief D4x4 used to calculate feature F8
 */
static inline unsigned int D4x4(const int32x8 *Y)
{
    const int32x8 r0 = _mm256_add_epi32(Y[0], Y[1]);
    const int32x8 r1 = _mm256_add_epi32(Y[2], Y[3]);
    const int32x8 r2 = _mm256_add_epi32(Y[4], Y[5]);
    const int32x8 r3 = _mm256_add_epi32(Y[6], Y[7]);

    const int32x8 r0_hadd_r2 = _mm256_hadd_epi32(r0, r2);
    const int32x8 r1_hadd_r3 = _mm256_hadd_epi32(r1, r3);

    const int32x8 partial_diff = _mm256_sub_epi32(r0_hadd_r2, r1_hadd_r3);

    const int32x8 diff_12__34__ = _mm256_hsub_epi32(partial_diff, partial_diff);

    const int32x8 abs_diff_12__34__ = _mm256_abs_epi32(diff_12__34__);

    const int32x8 sum_1___2___ = _mm256_hadd_epi32(abs_diff_12__34__, abs_diff_12__34__);

    return _mm256_extract_epi32(sum_1___2___, 0) + _mm256_extract_epi32(sum_1___2___, 4);
}


/**
 * @brief G2x2_UV used to calculate feature F10
 */
static inline unsigned int G2x2_UV(const int32x8 *U, const int32x8 *V)
{
    // Part 1: process U

    const int32x8 row0 = _mm256_add_epi32(U[0], U[1]);
    const int32x8 row1 = _mm256_add_epi32(U[2], U[3]);
    const int32x8 row2 = _mm256_add_epi32(U[4], U[5]);
    const int32x8 row3 = _mm256_add_epi32(U[6], U[7]);

    // Horisontal differences

    const int32x8 row0_hadd_row1 = _mm256_hadd_epi32(row0, row1);
    const int32x8 row2_hadd_row3 = _mm256_hadd_epi32(row2, row3);
    const int32x8 hor_differences_u = _mm256_hsub_epi32(row0_hadd_row1, row2_hadd_row3);

    // Vertical differences

    const int32x8 row0_sub_row1 = _mm256_sub_epi32(row0, row1);
    const int32x8 row2_sub_row3 = _mm256_sub_epi32(row2, row3);
    const int32x8 ver_differences_u = _mm256_hadd_epi32(row0_sub_row1, row2_sub_row3);

    // Absolute differences only

    const int32x8 absSum8u = _mm256_add_epi32(_mm256_abs_epi32(hor_differences_u), _mm256_abs_epi32(ver_differences_u));

    // Part 2: process V

    const int32x8 row4 = _mm256_add_epi32(V[0], V[1]);
    const int32x8 row5 = _mm256_add_epi32(V[2], V[3]);
    const int32x8 row6 = _mm256_add_epi32(V[4], V[5]);
    const int32x8 row7 = _mm256_add_epi32(V[6], V[7]);

    // Horisontal differences

    const int32x8 row4_hadd_row5 = _mm256_hadd_epi32(row4, row5);
    const int32x8 row6_hadd_row7 = _mm256_hadd_epi32(row6, row7);
    const int32x8 hor_differences_v = _mm256_hsub_epi32(row4_hadd_row5, row6_hadd_row7);

    // Vertical differences

    const int32x8 row4_sub_row5 = _mm256_sub_epi32(row4, row5);
    const int32x8 row6_sub_row7 = _mm256_sub_epi32(row6, row7);
    const int32x8 ver_differences_v = _mm256_hadd_epi32(row4_sub_row5, row6_sub_row7);

    // Absolute differences only

    const int32x8 absSum8v = _mm256_add_epi32(_mm256_abs_epi32(hor_differences_v), _mm256_abs_epi32(ver_differences_v));

    // Sum components from part 1 and 2

    const int32x8 sum_xxxxxxxx = _mm256_add_epi32(absSum8u, absSum8v);  // 8 integers
    const int32x8 sum_xx__xx__ = _mm256_hadd_epi32(sum_xxxxxxxx, sum_xxxxxxxx);  // 4 integers
    const int32x8 sum_x___x___ = _mm256_hadd_epi32(sum_xx__xx__, sum_xx__xx__);  // 2 integers

    return _mm256_extract_epi32(sum_x___x___, 0) + _mm256_extract_epi32(sum_x___x___, 4);
}


/**
 * @brief absCheckboardConvolution used to calculate feature F9
 */
static inline unsigned int absCheckboardConvolution(const int32x8 *Y)
{
    const int32x8 d0 = _mm256_sub_epi32(Y[0], Y[1]);
    const int32x8 d1 = _mm256_sub_epi32(Y[2], Y[3]);
    const int32x8 d2 = _mm256_sub_epi32(Y[4], Y[5]);
    const int32x8 d3 = _mm256_sub_epi32(Y[6], Y[7]);

    const int32x8 fourRows0 = _mm256_hsub_epi32(d0, d1);
    const int32x8 fourRows1 = _mm256_hsub_epi32(d2, d3);

    const int32x8 sum_xxxxxxxx = _mm256_add_epi32(fourRows0, fourRows1);

    const int32x8 sum_xx__xx__ = _mm256_hadd_epi32(sum_xxxxxxxx, sum_xxxxxxxx);

    const int32x8 sum_x___x___ = _mm256_hadd_epi32(sum_xx__xx__, sum_xx__xx__);

    const int sum = _mm256_extract_epi32(sum_x___x___, 0) + _mm256_extract_epi32(sum_x___x___, 4);

    return sum >= 0 ? sum : -sum;
}


/**
 * @brief calculateFeatures - this function calculates all necessaary features for entire image fragment by fragment
 * Note: when training regression models features F9 and F10 were mixed up, so this function was corrected to reflect the changes.
 */
void FeatureExtractor::calculateFeatures(const unsigned int *imageData, int imageWidth, int imageHeight, double *features)
{
    // Here we consider only 8x8 fragments

    const int fragmentSize = 8;

    const int numFragmentsInRow = imageWidth / fragmentSize;
    const int numFragmentsInCol = imageHeight / fragmentSize;
    const int numFragments = numFragmentsInRow * numFragmentsInCol;

    // In a loop we load one fragment from the image and
    // calculate its features accumulating results among all fragments.

    // Initialize accumulators that will be used for different features across all fragments.
    // All necessary features are calculated simultaneously.

    unsigned long long int absSumG1x1 = 0;
    unsigned long long int sqrSumG1x1 = 0;

    unsigned long long int absSumG2x2 = 0;
    unsigned long long int sqrSumG2x2 = 0;

    unsigned long long int absSumG4x4 = 0;
    unsigned long long int sqrSumG4x4 = 0;

    unsigned long long int absSumD2x2 = 0;
    unsigned long long int absSumD4x4 = 0;

    unsigned long long int absSumG2UV = 0;

    unsigned long long int absSumCheckboard = 0;

    // Main loop across fragments 8x8

    for (int frow = 0;  frow < numFragmentsInCol;  frow++)
    {
        const int fy = frow * fragmentSize;
        unsigned int *currentFragmentPointer = (unsigned int *) imageData + (fy * imageWidth);

        for (int fcol = 0;  fcol < numFragmentsInRow;  fcol++)
        {
            // Extract current fragment

            int32x8 Y [fragmentSize];
            int32x8 U [fragmentSize];
            int32x8 V [fragmentSize];

            unsigned int *linePointer = currentFragmentPointer;

            for (int line = 0;  line < fragmentSize;  line++)
            {
                // Load a line of xRGB into int8 register

                const int32x8 pixel = _mm256_loadu_si256((const int32x8 *) linePointer);

                // Some constants

                const int32x8 _0xff  = _mm256_set1_epi32(0xff);
                const int32x8 _32768 = _mm256_set1_epi32(32768);
                const int32x8 _8388608 = _mm256_set1_epi32(8388608);  // 128 * 65536

                // Get R, G and B components

                const int32x8 r = _mm256_and_si256(_mm256_bsrli_epi128(pixel, 2), _0xff);  // shift in 2 bytes from the right and crop result to a byte
                const int32x8 g = _mm256_and_si256(_mm256_bsrli_epi128(pixel, 1), _0xff);
                const int32x8 b = _mm256_and_si256(pixel, _0xff);

                // Calculate Y
                // const unsigned int y = (19595 * r + 38470 * g + 7471 * b + _32768) >> 16;

                const int32x8 y_t0 = _mm256_mullo_epi32(_mm256_set1_epi32(19595), r);
                const int32x8 y_t1 = _mm256_mullo_epi32(_mm256_set1_epi32(38470), g);
                const int32x8 y_t2 = _mm256_mullo_epi32(_mm256_set1_epi32(7471), b);
                const int32x8 y =    _mm256_srli_epi32(y_t0 + y_t1 + y_t2 + _32768, 16);
                Y[line] = y;

                // Calculate U
                // const unsigned int u = (-11058 * r  - 21710 * g + _32768 * b + _8388608) >> 16;

                const int32x8 u_t0 = _mm256_mullo_epi32(_mm256_set1_epi32(-11058), r);
                const int32x8 u_t1 = _mm256_mullo_epi32(_mm256_set1_epi32(-21710), g);
                const int32x8 u_t2 = _mm256_mullo_epi32(_32768, b);
                //const int32x8 u = _mm256_srli_epi32(u_t0 + u_t1 + u_t2 + _8388608, 16);  // CAUTION: int64 addition possible
                const int32x8 u = _mm256_srli_epi32(_mm256_add_epi32(_mm256_add_epi32(u_t0, u_t1), _mm256_add_epi32(u_t2, _8388608)), 16);
                U[line] = u;

                // Calculate V
                // const unsigned int v = (_32768 * r - 27439 * g - 5329 * b + _8388608) >> 16;

                const int32x8 v_t0 = _mm256_mullo_epi32(_32768, r);
                const int32x8 v_t1 = _mm256_mullo_epi32(_mm256_set1_epi32(-27439), g);
                const int32x8 v_t2 = _mm256_mullo_epi32(_mm256_set1_epi32(-5329), b);
                //const int32x8 v = _mm256_srli_epi32(v_t0 + v_t1 + v_t2 + _8388608, 16);  // CAUTION: int64 addition possible
                const int32x8 v = _mm256_srli_epi32(_mm256_add_epi32(_mm256_add_epi32(v_t0, v_t1), _mm256_add_epi32(v_t2, _8388608)), 16);
                V[line] = v;

                // Move line pointer to the next line in fragment

                linePointer += imageWidth;
            }

            // Current fragment is in local array
            // Process current fragment

            // -----
            // F1, F4
            // Calculating gradient 1x1
            // -----

            {
                unsigned int absSum, sqrSum;
                G1x1(Y, &absSum, &sqrSum);
                absSumG1x1 += absSum;
                sqrSumG1x1 += sqrSum;
            }

            // -----
            // F2, F5
            // Calculating gradient 2x2
            // -----

            {
                unsigned int absSum, sqrSum;
                G2x2(Y, &absSum, &sqrSum);
                absSumG2x2 += absSum;
                sqrSumG2x2 += sqrSum;
            }

            // -----
            // F3, F6
            // Calculating gradient 4x4
            // -----

            {
                unsigned int absSum, sqrSum;
                G4x4(Y, &absSum, &sqrSum);
                absSumG4x4 += absSum;
                sqrSumG4x4 += sqrSum;
            }

            // -----
            // F7
            // Calculating diagonal differences in 2x2 blocks
            // -----

            absSumD2x2 += D2x2(Y);

            // -----
            // F8
            // Calculating diagonal differences in 4x4 blocks
            // -----

            absSumD4x4 += D4x4(Y);

            // -----
            // F9
            // Calculating checkboard convolution for Y component
            // -----

            absSumCheckboard += absCheckboardConvolution(Y);

            // -----
            // F10
            // Calculating gradient 2x2 for UV components
            // -----

            absSumG2UV += G2x2_UV(U, V);

            // Current fragment is processed

            // Move pointer to the neighbour fragment in current row

            currentFragmentPointer += fragmentSize;
        }
    }

    // Calculate main values and logarithmize features

    // -----
    // Finishing gradient 1x1 (F1, F4)
    // -----

    {
        const int numDifferences = fragmentSize * fragmentSize;
        const int totalDifferences = numDifferences * numFragments;

        const double meanAbsValue = (double) absSumG1x1 / totalDifferences;
        const double meanSqrValue = (double) sqrSumG1x1 / totalDifferences;

        features[0] = log(meanAbsValue + 1);
        features[3] = log(meanSqrValue + 1);
    }

    // -----
    // Finishing gradient 2x2 (F2, F5)
    // -----

    {
        const int blockSize = 2;
        const int normalizationCoef = blockSize * blockSize;  // Number of pixels in a block

        const int numDifferences = (fragmentSize / blockSize) * (fragmentSize / blockSize);
        const int totalDifferences = numDifferences * numFragments;

        const double meanAbsValue = (double) absSumG2x2 / (totalDifferences * normalizationCoef);
        const double meanSqrValue = (double) sqrSumG2x2 / (totalDifferences * normalizationCoef * normalizationCoef);

        features[1] = log(meanAbsValue + 1);
        features[4] = log(meanSqrValue + 1);
    }

    // -----
    // Finishing gradient 4x4 (F3, F6)
    // -----

    {
        const int blockSize = 4;
        const int normalizationCoef = blockSize * blockSize;  // Number of pixels in a block

        const int numDifferences = (fragmentSize / blockSize) * (fragmentSize / blockSize);
        const int totalDifferences = numDifferences * numFragments;

        const double meanAbsValue = (double) absSumG4x4 / (totalDifferences * normalizationCoef);
        const double meanSqrValue = (double) sqrSumG4x4 / (totalDifferences * normalizationCoef * normalizationCoef);

        features[2] = log(meanAbsValue + 1);
        features[5] = log(meanSqrValue + 1);
    }

    // -----
    // Finishing diagonal differences in blocks 2x2 (F7)
    // -----

    {
        const int blockSize = 2;
        const int normalizationCoef = 2;
        const int numDifferences = (fragmentSize / blockSize) * (fragmentSize / blockSize);
        const int totalDifferences = numDifferences * numFragments;

        const double meanAbsValue = (double) absSumD2x2 / (totalDifferences * normalizationCoef);
        features[6] = log(meanAbsValue + 1);
    }

    // -----
    // Finishing diagonal differences in blocks 4x4 (F8)
    // -----

    {
        const int blockSize = 4;
        const int normalizationCoef = 8;
        const int numDifferences = (fragmentSize / blockSize) * (fragmentSize / blockSize);
        const int totalDifferences = numDifferences * numFragments;

        const double meanAbsValue = (double) absSumD4x4 / (totalDifferences * normalizationCoef);
        features[7] = log(meanAbsValue + 1);
    }

    // -----
    // Finishing gradient 2x2 for UV components (F10)
    // -----

    {
        const int blockSize = 2;
        const int normalizationCoef = blockSize * blockSize * 2;  // Number of pixels in a block times the number of blocks in 2 colour channels

        const int numDifferences = (fragmentSize / blockSize) * (fragmentSize / blockSize);  // Number of differences in 1 fragment per colour channel
        const int totalDifferences = numDifferences * numFragments;

        const double meanAbsValue = (double) absSumG2UV / (totalDifferences * normalizationCoef);
        features[8] = log(meanAbsValue + 1);
    }

    // -----
    // Finishing chechboard convolution for Y components (F9)
    // -----

    {
        const int normalizationCoef = fragmentSize * fragmentSize / 2;  // Half positive cells and half negative
        const double meanAbsValue = (double) absSumCheckboard / (numFragments * normalizationCoef);
        features[9] = log(meanAbsValue + 1);
    }

    // -----

    // End of the feature extraction function
}
