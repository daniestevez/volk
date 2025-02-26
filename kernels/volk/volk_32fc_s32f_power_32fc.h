/* -*- c++ -*- */
/*
 * Copyright 2012, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*!
 * \page volk_32fc_s32f_power_32fc
 *
 * \b Overview
 *
 * Takes each the input complex vector value to the specified power
 * and stores the results in the return vector. The output is scaled
 * and converted to 16-bit shorts.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32fc_s32f_power_32fc(lv_32fc_t* cVector, const lv_32fc_t* aVector, const
 * float power, unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li aVector: The complex input vector.
 * \li power: The power value to be applied to each data point.
 * \li num_points: The number of samples.
 *
 * \b Outputs
 * \li cVector: The output value as 16-bit shorts.
 *
 * \b Example
 * \code
 * int N = 10000;
 *
 * volk_32fc_s32f_power_32fc();
 *
 * volk_free(x);
 * \endcode
 */

#ifndef INCLUDED_volk_32fc_s32f_power_32fc_a_H
#define INCLUDED_volk_32fc_s32f_power_32fc_a_H

#include <inttypes.h>
#include <math.h>
#include <stdio.h>

//! raise a complex float to a real float power
static inline lv_32fc_t __volk_s32fc_s32f_power_s32fc_a(const lv_32fc_t exp,
                                                        const float power)
{
    const float arg = power * atan2f(lv_creal(exp), lv_cimag(exp));
    const float mag =
        powf(lv_creal(exp) * lv_creal(exp) + lv_cimag(exp) * lv_cimag(exp), power / 2);
    return mag * lv_cmake(-cosf(arg), sinf(arg));
}

#ifdef LV_HAVE_SSE
#include <xmmintrin.h>

#ifdef LV_HAVE_LIB_SIMDMATH
#include <simdmath.h>
#endif /* LV_HAVE_LIB_SIMDMATH */

static inline void volk_32fc_s32f_power_32fc_a_sse(lv_32fc_t* cVector,
                                                   const lv_32fc_t* aVector,
                                                   const float power,
                                                   unsigned int num_points)
{
    unsigned int number = 0;

    lv_32fc_t* cPtr = cVector;
    const lv_32fc_t* aPtr = aVector;

#ifdef LV_HAVE_LIB_SIMDMATH
    const unsigned int quarterPoints = num_points / 4;
    __m128 vPower = _mm_set_ps1(power);

    __m128 cplxValue1, cplxValue2, magnitude, phase, iValue, qValue;
    for (; number < quarterPoints; number++) {

        cplxValue1 = _mm_load_ps((float*)aPtr);
        aPtr += 2;

        cplxValue2 = _mm_load_ps((float*)aPtr);
        aPtr += 2;

        // Convert to polar coordinates

        // Arrange in i1i2i3i4 format
        iValue = _mm_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(2, 0, 2, 0));
        // Arrange in q1q2q3q4 format
        qValue = _mm_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(3, 1, 3, 1));

        phase = atan2f4(qValue, iValue); // Calculate the Phase

        magnitude = _mm_sqrt_ps(
            _mm_add_ps(_mm_mul_ps(iValue, iValue),
                       _mm_mul_ps(qValue, qValue))); // Calculate the magnitude by square
                                                     // rooting the added I2 and Q2 values

        // Now calculate the power of the polar coordinate data
        magnitude = powf4(magnitude, vPower); // Take the magnitude to the specified power

        phase = _mm_mul_ps(phase, vPower); // Multiply the phase by the specified power

        // Convert back to cartesian coordinates
        iValue = _mm_mul_ps(cosf4(phase),
                            magnitude); // Multiply the cos of the phase by the magnitude
        qValue = _mm_mul_ps(sinf4(phase),
                            magnitude); // Multiply the sin of the phase by the magnitude

        cplxValue1 =
            _mm_unpacklo_ps(iValue, qValue); // Interleave the lower two i & q values
        cplxValue2 =
            _mm_unpackhi_ps(iValue, qValue); // Interleave the upper two i & q values

        _mm_store_ps((float*)cPtr,
                     cplxValue1); // Store the results back into the C container

        cPtr += 2;

        _mm_store_ps((float*)cPtr,
                     cplxValue2); // Store the results back into the C container

        cPtr += 2;
    }

    number = quarterPoints * 4;
#endif /* LV_HAVE_LIB_SIMDMATH */

    for (; number < num_points; number++) {
        *cPtr++ = __volk_s32fc_s32f_power_s32fc_a((*aPtr++), power);
    }
}
#endif /* LV_HAVE_SSE */


#ifdef LV_HAVE_GENERIC

static inline void volk_32fc_s32f_power_32fc_generic(lv_32fc_t* cVector,
                                                     const lv_32fc_t* aVector,
                                                     const float power,
                                                     unsigned int num_points)
{
    lv_32fc_t* cPtr = cVector;
    const lv_32fc_t* aPtr = aVector;
    unsigned int number = 0;

    for (number = 0; number < num_points; number++) {
        *cPtr++ = __volk_s32fc_s32f_power_s32fc_a((*aPtr++), power);
    }
}

#endif /* LV_HAVE_GENERIC */


#endif /* INCLUDED_volk_32fc_s32f_power_32fc_a_H */
