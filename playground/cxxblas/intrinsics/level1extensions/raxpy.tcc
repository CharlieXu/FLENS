/*
 *   Copyright (c) 2012, Klaus Pototzky
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *   1) Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2) Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *   3) Neither the name of the FLENS development group nor the names of
 *      its contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PLAYGROUND_CXXBLAS_INTRINSICS_LEVEL1_RAXPY_TCC
#define PLAYGROUND_CXXBLAS_INTRINSICS_LEVEL1_RAXPY_TCC 1

#include <cxxblas/cxxblas.h>
#include <playground/cxxblas/intrinsics/auxiliary/auxiliary.h>
#include <playground/cxxblas/intrinsics/includes.h>
#include <playground/cxxblas/intrinsics/level1/axpy.h>

namespace cxxblas {

#ifdef USE_INTRINSIC

template <typename IndexType, typename T>
typename flens::RestrictTo<flens::IsReal<T>::value, void>::Type
raxpy(IndexType n, const T &alpha, const T *x,
      IndexType incX, T *y, IndexType incY)
{
    CXXBLAS_DEBUG_OUT("raxpy_intrinsics [real, " INTRINSIC_NAME "]");

    if (incX==1 && incY==1) {
        typedef Intrinsics<T, DEFAULT_INTRINSIC_LEVEL> IntrinsicType;
        const int numElements = IntrinsicType::numElements;

        IndexType i=0;

        IntrinsicType x_, y_;
        IntrinsicType alpha_(alpha);

        for (; i+numElements-1<n; i+=numElements) {
            x_.loadu(x+i);
            y_.loadu(y+i);
            y_ = intrinsic_add_(y_, intrinsic_div_(x_, alpha_));
            y_.storeu(y+i);
        }

        for (; i<n; ++i) {
            y[i] += x[i]/alpha;
        }

    } else {

        cxxblas::raxpy<IndexType, T, T ,T>(n, alpha, x, incX, y, incY);

    }
}

template <typename IndexType, typename T>
typename flens::RestrictTo<flens::IsComplex<T>::value, void>::Type
raxpy(IndexType n, const T &alpha, const T *x,
      IndexType incX, T *y, IndexType incY)
{
    CXXBLAS_DEBUG_OUT("raxpy_intrinsics [complex, " INTRINSIC_NAME "]");

    using std::real;
    using std::imag;

    typedef Intrinsics<T, DEFAULT_INTRINSIC_LEVEL>     IntrinsicType;
    typedef typename IntrinsicType::PrimitiveDataType  PT;
    typedef Intrinsics<PT, DEFAULT_INTRINSIC_LEVEL>    IntrinsicPrimitiveType;

    if (incX==1 && incY==1) {

        if (imag(alpha)==PT(0)) {
            raxpy(2*n, real(alpha), reinterpret_cast<const PT*>(x), 1, reinterpret_cast<PT*>(y), 1);
           return;
        }

        const int numElements = IntrinsicType::numElements;

        IntrinsicType x_, y_, tmp_;

        IndexType i=0;

        if (abs(real(alpha)) < abs(imag(alpha))) {

            PT r   = real(alpha)/imag(alpha);
            PT den = imag(alpha) + r*real(alpha);

            IntrinsicPrimitiveType mr_(-r);
            IntrinsicPrimitiveType den_(den);

           for (; i+numElements-1<n; i+=numElements) {
                x_.loadu(x+i);
                y_.loadu(y+i);

                tmp_ = intrinsic_mul_(mr_, x_);
                x_   = intrinsic_swap_real_imag_(x_);
                tmp_ = intrinsic_addsub_(tmp_, x_);
                y_   = intrinsic_sub_(y_, intrinsic_div_(tmp_, den_));

                y_.storeu(y+i);
            }

        } else {

            PT r   = imag(alpha)/real(alpha);
            PT den = real(alpha) + r*imag(alpha);

            IntrinsicPrimitiveType mr_(-r);
            IntrinsicPrimitiveType den_(den);

           for (; i+numElements-1<n; i+=numElements) {
                x_.loadu(x+i);
                y_.loadu(y+i);

                tmp_ = intrinsic_mul_(mr_,intrinsic_swap_real_imag_(x_));
                x_   = intrinsic_addsub_(x_, tmp_);
                y_   = intrinsic_add_(y_, intrinsic_div_(x_, den_));

                y_.storeu(y+i);
            }



        }

        for (; i<n; ++i) {
            y[i] += x[i]/alpha;
        }

    } else {

        cxxblas::raxpy<IndexType, T, T ,T>(n, alpha, x, incX, y, incY);

    }
}

#endif // USE_INTRINSIC

} // namespace cxxblas

#endif // PLAYGROUND_CXXBLAS_INTRINSICS_LEVEL1_RAXPY_TCC
