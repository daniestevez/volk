/* -*- c++ -*- */
/*
 * Copyright 2012, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*!
 * \page volk_32f_s32f_convert_8i
 *
 * \b Overview
 *
 * Converts a floating point number to a 8-bit int after applying a
 * scaling factor.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32f_s32f_convert_8i(int8_t* outputVector, const float* inputVector, const
 float scalar, unsigned int num_points)
 * \endcode
 *
 * \b Inputs
 * \li inputVector: the input vector of floats.
 * \li scalar: The value multiplied against each point in the input buffer.
 * \li num_points: The number of data points.
 *
 * \b Outputs
 * \li outputVector: The output vector.
 *
 * \b Example
 * Convert floats from [-1,1] to 16-bit integers with a scale of 5 to maintain smallest
 delta
 *  int N = 10;
 *   unsigned int alignment = volk_get_alignment();
 *   float* increasing = (float*)volk_malloc(sizeof(float)*N, alignment);
 *   int16_t* out = (int16_t*)volk_malloc(sizeof(int16_t)*N, alignment);
 *
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       increasing[ii] = 2.f * ((float)ii / (float)N) - 1.f;
 *   }
 *
 *   // Normalize by the smallest delta (0.2 in this example)
 *   // With float -> 8 bit ints be careful of scaling

 *   float scale = 5.1f;
 *
 *   volk_32f_s32f_convert_32i(out, increasing, scale, N);
 *
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       printf("out[%u] = %i\n", ii, out[ii]);
 *   }
 *
 *   volk_free(increasing);
 *   volk_free(out);
 * \endcode
 */

#ifndef INCLUDED_volk_32f_s32f_convert_8i_u_H
#define INCLUDED_volk_32f_s32f_convert_8i_u_H

#include <inttypes.h>
#include <stdio.h>

static inline void volk_32f_s32f_convert_8i_single(int8_t* out, const float in)
{
    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    if (in > max_val) {
        *out = (int8_t)(max_val);
    } else if (in < min_val) {
        *out = (int8_t)(min_val);
    } else {
        *out = (int8_t)(rintf(in));
    }
}

#ifdef LV_HAVE_AVX2
#include <immintrin.h>

static inline void volk_32f_s32f_convert_8i_u_avx2(int8_t* outputVector,
                                                   const float* inputVector,
                                                   const float scalar,
                                                   unsigned int num_points)
{
    unsigned int number = 0;

    const unsigned int thirtysecondPoints = num_points / 32;

    const float* inputVectorPtr = (const float*)inputVector;
    int8_t* outputVectorPtr = outputVector;

    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    float r;

    __m256 vScalar = _mm256_set1_ps(scalar);
    __m256 inputVal1, inputVal2, inputVal3, inputVal4;
    __m256i intInputVal1, intInputVal2, intInputVal3, intInputVal4;
    __m256 vmin_val = _mm256_set1_ps(min_val);
    __m256 vmax_val = _mm256_set1_ps(max_val);
    __m256i intInputVal;

    for (; number < thirtysecondPoints; number++) {
        inputVal1 = _mm256_loadu_ps(inputVectorPtr);
        inputVectorPtr += 8;
        inputVal2 = _mm256_loadu_ps(inputVectorPtr);
        inputVectorPtr += 8;
        inputVal3 = _mm256_loadu_ps(inputVectorPtr);
        inputVectorPtr += 8;
        inputVal4 = _mm256_loadu_ps(inputVectorPtr);
        inputVectorPtr += 8;

        inputVal1 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal1, vScalar), vmax_val), vmin_val);
        inputVal2 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal2, vScalar), vmax_val), vmin_val);
        inputVal3 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal3, vScalar), vmax_val), vmin_val);
        inputVal4 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal4, vScalar), vmax_val), vmin_val);

        intInputVal1 = _mm256_cvtps_epi32(inputVal1);
        intInputVal2 = _mm256_cvtps_epi32(inputVal2);
        intInputVal3 = _mm256_cvtps_epi32(inputVal3);
        intInputVal4 = _mm256_cvtps_epi32(inputVal4);

        intInputVal1 = _mm256_packs_epi32(intInputVal1, intInputVal2);
        intInputVal1 = _mm256_permute4x64_epi64(intInputVal1, 0b11011000);
        intInputVal3 = _mm256_packs_epi32(intInputVal3, intInputVal4);
        intInputVal3 = _mm256_permute4x64_epi64(intInputVal3, 0b11011000);

        intInputVal1 = _mm256_packs_epi16(intInputVal1, intInputVal3);
        intInputVal = _mm256_permute4x64_epi64(intInputVal1, 0b11011000);

        _mm256_storeu_si256((__m256i*)outputVectorPtr, intInputVal);
        outputVectorPtr += 32;
    }

    number = thirtysecondPoints * 32;
    for (; number < num_points; number++) {
        r = inputVector[number] * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_AVX2 */


#ifdef LV_HAVE_SSE2
#include <emmintrin.h>

static inline void volk_32f_s32f_convert_8i_u_sse2(int8_t* outputVector,
                                                   const float* inputVector,
                                                   const float scalar,
                                                   unsigned int num_points)
{
    unsigned int number = 0;

    const unsigned int sixteenthPoints = num_points / 16;

    const float* inputVectorPtr = (const float*)inputVector;
    int8_t* outputVectorPtr = outputVector;

    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    float r;

    __m128 vScalar = _mm_set_ps1(scalar);
    __m128 inputVal1, inputVal2, inputVal3, inputVal4;
    __m128i intInputVal1, intInputVal2, intInputVal3, intInputVal4;
    __m128 vmin_val = _mm_set_ps1(min_val);
    __m128 vmax_val = _mm_set_ps1(max_val);

    for (; number < sixteenthPoints; number++) {
        inputVal1 = _mm_loadu_ps(inputVectorPtr);
        inputVectorPtr += 4;
        inputVal2 = _mm_loadu_ps(inputVectorPtr);
        inputVectorPtr += 4;
        inputVal3 = _mm_loadu_ps(inputVectorPtr);
        inputVectorPtr += 4;
        inputVal4 = _mm_loadu_ps(inputVectorPtr);
        inputVectorPtr += 4;

        inputVal1 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal1, vScalar), vmax_val), vmin_val);
        inputVal2 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal2, vScalar), vmax_val), vmin_val);
        inputVal3 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal3, vScalar), vmax_val), vmin_val);
        inputVal4 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal4, vScalar), vmax_val), vmin_val);

        intInputVal1 = _mm_cvtps_epi32(inputVal1);
        intInputVal2 = _mm_cvtps_epi32(inputVal2);
        intInputVal3 = _mm_cvtps_epi32(inputVal3);
        intInputVal4 = _mm_cvtps_epi32(inputVal4);

        intInputVal1 = _mm_packs_epi32(intInputVal1, intInputVal2);
        intInputVal3 = _mm_packs_epi32(intInputVal3, intInputVal4);

        intInputVal1 = _mm_packs_epi16(intInputVal1, intInputVal3);

        _mm_storeu_si128((__m128i*)outputVectorPtr, intInputVal1);
        outputVectorPtr += 16;
    }

    number = sixteenthPoints * 16;
    for (; number < num_points; number++) {
        r = inputVector[number] * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_SSE2 */


#ifdef LV_HAVE_SSE
#include <xmmintrin.h>

static inline void volk_32f_s32f_convert_8i_u_sse(int8_t* outputVector,
                                                  const float* inputVector,
                                                  const float scalar,
                                                  unsigned int num_points)
{
    unsigned int number = 0;
    size_t inner_loop;

    const unsigned int quarterPoints = num_points / 4;

    const float* inputVectorPtr = (const float*)inputVector;
    int8_t* outputVectorPtr = outputVector;

    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    float r;

    __m128 vScalar = _mm_set_ps1(scalar);
    __m128 ret;
    __m128 vmin_val = _mm_set_ps1(min_val);
    __m128 vmax_val = _mm_set_ps1(max_val);

    __VOLK_ATTR_ALIGNED(16) float outputFloatBuffer[4];

    for (; number < quarterPoints; number++) {
        ret = _mm_loadu_ps(inputVectorPtr);
        inputVectorPtr += 4;

        ret = _mm_max_ps(_mm_min_ps(_mm_mul_ps(ret, vScalar), vmax_val), vmin_val);

        _mm_store_ps(outputFloatBuffer, ret);
        for (inner_loop = 0; inner_loop < 4; inner_loop++) {
            *outputVectorPtr++ = (int8_t)(rintf(outputFloatBuffer[inner_loop]));
        }
    }

    number = quarterPoints * 4;
    for (; number < num_points; number++) {
        r = inputVector[number] * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_SSE */


#ifdef LV_HAVE_GENERIC

static inline void volk_32f_s32f_convert_8i_generic(int8_t* outputVector,
                                                    const float* inputVector,
                                                    const float scalar,
                                                    unsigned int num_points)
{
    const float* inputVectorPtr = inputVector;
    unsigned int number = 0;
    float r;

    for (number = 0; number < num_points; number++) {
        r = *inputVectorPtr++ * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_GENERIC */


#endif /* INCLUDED_volk_32f_s32f_convert_8i_u_H */
#ifndef INCLUDED_volk_32f_s32f_convert_8i_a_H
#define INCLUDED_volk_32f_s32f_convert_8i_a_H

#include <inttypes.h>
#include <stdio.h>
#include <volk/volk_common.h>

#ifdef LV_HAVE_AVX2
#include <immintrin.h>

static inline void volk_32f_s32f_convert_8i_a_avx2(int8_t* outputVector,
                                                   const float* inputVector,
                                                   const float scalar,
                                                   unsigned int num_points)
{
    unsigned int number = 0;

    const unsigned int thirtysecondPoints = num_points / 32;

    const float* inputVectorPtr = (const float*)inputVector;
    int8_t* outputVectorPtr = outputVector;

    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    float r;

    __m256 vScalar = _mm256_set1_ps(scalar);
    __m256 inputVal1, inputVal2, inputVal3, inputVal4;
    __m256i intInputVal1, intInputVal2, intInputVal3, intInputVal4;
    __m256 vmin_val = _mm256_set1_ps(min_val);
    __m256 vmax_val = _mm256_set1_ps(max_val);
    __m256i intInputVal;

    for (; number < thirtysecondPoints; number++) {
        inputVal1 = _mm256_load_ps(inputVectorPtr);
        inputVectorPtr += 8;
        inputVal2 = _mm256_load_ps(inputVectorPtr);
        inputVectorPtr += 8;
        inputVal3 = _mm256_load_ps(inputVectorPtr);
        inputVectorPtr += 8;
        inputVal4 = _mm256_load_ps(inputVectorPtr);
        inputVectorPtr += 8;

        inputVal1 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal1, vScalar), vmax_val), vmin_val);
        inputVal2 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal2, vScalar), vmax_val), vmin_val);
        inputVal3 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal3, vScalar), vmax_val), vmin_val);
        inputVal4 = _mm256_max_ps(
            _mm256_min_ps(_mm256_mul_ps(inputVal4, vScalar), vmax_val), vmin_val);

        intInputVal1 = _mm256_cvtps_epi32(inputVal1);
        intInputVal2 = _mm256_cvtps_epi32(inputVal2);
        intInputVal3 = _mm256_cvtps_epi32(inputVal3);
        intInputVal4 = _mm256_cvtps_epi32(inputVal4);

        intInputVal1 = _mm256_packs_epi32(intInputVal1, intInputVal2);
        intInputVal1 = _mm256_permute4x64_epi64(intInputVal1, 0b11011000);
        intInputVal3 = _mm256_packs_epi32(intInputVal3, intInputVal4);
        intInputVal3 = _mm256_permute4x64_epi64(intInputVal3, 0b11011000);

        intInputVal1 = _mm256_packs_epi16(intInputVal1, intInputVal3);
        intInputVal = _mm256_permute4x64_epi64(intInputVal1, 0b11011000);

        _mm256_store_si256((__m256i*)outputVectorPtr, intInputVal);
        outputVectorPtr += 32;
    }

    number = thirtysecondPoints * 32;
    for (; number < num_points; number++) {
        r = inputVector[number] * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_AVX2 */


#ifdef LV_HAVE_SSE2
#include <emmintrin.h>

static inline void volk_32f_s32f_convert_8i_a_sse2(int8_t* outputVector,
                                                   const float* inputVector,
                                                   const float scalar,
                                                   unsigned int num_points)
{
    unsigned int number = 0;

    const unsigned int sixteenthPoints = num_points / 16;

    const float* inputVectorPtr = (const float*)inputVector;
    int8_t* outputVectorPtr = outputVector;

    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    float r;

    __m128 vScalar = _mm_set_ps1(scalar);
    __m128 inputVal1, inputVal2, inputVal3, inputVal4;
    __m128i intInputVal1, intInputVal2, intInputVal3, intInputVal4;
    __m128 vmin_val = _mm_set_ps1(min_val);
    __m128 vmax_val = _mm_set_ps1(max_val);

    for (; number < sixteenthPoints; number++) {
        inputVal1 = _mm_load_ps(inputVectorPtr);
        inputVectorPtr += 4;
        inputVal2 = _mm_load_ps(inputVectorPtr);
        inputVectorPtr += 4;
        inputVal3 = _mm_load_ps(inputVectorPtr);
        inputVectorPtr += 4;
        inputVal4 = _mm_load_ps(inputVectorPtr);
        inputVectorPtr += 4;

        inputVal1 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal1, vScalar), vmax_val), vmin_val);
        inputVal2 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal2, vScalar), vmax_val), vmin_val);
        inputVal3 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal3, vScalar), vmax_val), vmin_val);
        inputVal4 =
            _mm_max_ps(_mm_min_ps(_mm_mul_ps(inputVal4, vScalar), vmax_val), vmin_val);

        intInputVal1 = _mm_cvtps_epi32(inputVal1);
        intInputVal2 = _mm_cvtps_epi32(inputVal2);
        intInputVal3 = _mm_cvtps_epi32(inputVal3);
        intInputVal4 = _mm_cvtps_epi32(inputVal4);

        intInputVal1 = _mm_packs_epi32(intInputVal1, intInputVal2);
        intInputVal3 = _mm_packs_epi32(intInputVal3, intInputVal4);

        intInputVal1 = _mm_packs_epi16(intInputVal1, intInputVal3);

        _mm_store_si128((__m128i*)outputVectorPtr, intInputVal1);
        outputVectorPtr += 16;
    }

    number = sixteenthPoints * 16;
    for (; number < num_points; number++) {
        r = inputVector[number] * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}
#endif /* LV_HAVE_SSE2 */


#ifdef LV_HAVE_SSE
#include <xmmintrin.h>

static inline void volk_32f_s32f_convert_8i_a_sse(int8_t* outputVector,
                                                  const float* inputVector,
                                                  const float scalar,
                                                  unsigned int num_points)
{
    unsigned int number = 0;
    size_t inner_loop;

    const unsigned int quarterPoints = num_points / 4;

    const float* inputVectorPtr = (const float*)inputVector;

    float min_val = INT8_MIN;
    float max_val = INT8_MAX;
    float r;

    int8_t* outputVectorPtr = outputVector;
    __m128 vScalar = _mm_set_ps1(scalar);
    __m128 ret;
    __m128 vmin_val = _mm_set_ps1(min_val);
    __m128 vmax_val = _mm_set_ps1(max_val);

    __VOLK_ATTR_ALIGNED(16) float outputFloatBuffer[4];

    for (; number < quarterPoints; number++) {
        ret = _mm_load_ps(inputVectorPtr);
        inputVectorPtr += 4;

        ret = _mm_max_ps(_mm_min_ps(_mm_mul_ps(ret, vScalar), vmax_val), vmin_val);

        _mm_store_ps(outputFloatBuffer, ret);
        for (inner_loop = 0; inner_loop < 4; inner_loop++) {
            *outputVectorPtr++ = (int8_t)(rintf(outputFloatBuffer[inner_loop]));
        }
    }

    number = quarterPoints * 4;
    for (; number < num_points; number++) {
        r = inputVector[number] * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_SSE */


#ifdef LV_HAVE_GENERIC

static inline void volk_32f_s32f_convert_8i_a_generic(int8_t* outputVector,
                                                      const float* inputVector,
                                                      const float scalar,
                                                      unsigned int num_points)
{
    const float* inputVectorPtr = inputVector;
    unsigned int number = 0;
    float r;

    for (number = 0; number < num_points; number++) {
        r = *inputVectorPtr++ * scalar;
        volk_32f_s32f_convert_8i_single(&outputVector[number], r);
    }
}

#endif /* LV_HAVE_GENERIC */


#endif /* INCLUDED_volk_32f_s32f_convert_8i_a_H */
