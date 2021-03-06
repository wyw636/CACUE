/*
 Copyright (c) 2016, David lu
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once


#include "cuda_log.h"
#include "cuda_utils.h"
#include "../../utils/data_defination.h"
#if __PARALLELTYPE__ == __CUDA__


inline void cacu_saxpy_gpu(mycnn::float_t *x, mycnn::float_t a, mycnn::float_t *y, int length) {
	mycnn::float_t a_ = a;
	status = cublasSaxpy_v2(handle, length, &a_, x, 1, y, 1);
	CUBLAS_CHECK(status);
}

inline void cacu_saxpby_gpu(mycnn::float_t *x, mycnn::float_t a, mycnn::float_t *y, mycnn::float_t b, int length)
{
	mycnn::float_t a_ = a;
	mycnn::float_t b_ = b;
	status = cublasSscal_v2(handle, length, &b_, y, 1);
	CUBLAS_CHECK(status);
	status = cublasSaxpy_v2(handle, length, &a_, x, 1, y, 1);
	CUBLAS_CHECK(status);
}

inline void cacu_scalex_gpu(mycnn::float_t *x, mycnn::float_t a, int length)
{
	mycnn::float_t a_ = a;
	status = cublasSscal_v2(handle, length, &a, x, 1);
	CUBLAS_CHECK(status);
}

inline void cacu_sgemv_gpu(cublasOperation_t trans, mycnn::float_t *x, int x_height, mycnn::float_t *y, int x_width, mycnn::float_t alpha, mycnn::float_t *z , mycnn::float_t beta)
{
	int m = x_height,n = x_width;
	mycnn::float_t _alpha = alpha;
	mycnn::float_t _beta = beta;
	status = cublasSgemv_v2(handle, trans, m, n, &alpha, x, m, y, 1, &beta, z, 1);
	CUBLAS_CHECK(status);
}

inline void cacu_sgemm_gpu(cublasOperation_t transx, cublasOperation_t transy, mycnn::float_t *x, int x_height, int x_width, mycnn::float_t *y, int y_width, mycnn::float_t alpha, mycnn::float_t *z, mycnn::float_t beta)
{
	int m = x_height,n = y_width,k = x_width;
	int lda = (transx == CUBLAS_OP_N) ? m : k;
	int ldb = (transy == CUBLAS_OP_N) ? k : n;
	mycnn::float_t _alpha = alpha;
	mycnn::float_t _beta = beta;
	status = cublasSgemm_v2(handle, transx, transy, m, n, k, &_alpha, x, lda, y, ldb, &_beta, z, m);
	CUBLAS_CHECK(status);
}

inline void cacu_copy_gpu(mycnn::float_t *x, int x_length,mycnn::float_t *y)
{
	status = cublasScopy_v2(handle, x_length, x, 1, y, 1);
	CUBLAS_CHECK(status);
}

/**
 * @cacu_isaxdb_gpu
 * y[index] = x[index]*a + b
 */
extern "C" void cacu_isaxb_gpu(mycnn::float_t *x, int length, mycnn::float_t a ,unsigned int *index_, mycnn::float_t b, mycnn::float_t *y);

extern "C" void cacu_argmax_gpu(mycnn::float_t *x,int length, unsigned int *index_);


#endif
