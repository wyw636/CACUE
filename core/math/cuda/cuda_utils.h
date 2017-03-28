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

#if __PARALLELTYPE__ == __GPU__

#include <cublas.h>
#include <cublas_api.h>

#include "cuda_log.h"


cudaError_t res;
cublasHandle_t handle;
cublasStatus_t status;


bool cuda_initial()
{
	int device_count;
	if(cudaGetDeviceCount(&device_count))
	{
		LOG_FATAL("There is no device!");
		return false;
	}
	return true;
}

void cuda_set_device(int device_id)
{
	struct cudaDeviceProp device_prop;
	if(cudaGetDeviceProperties(&device_prop, device_id) == cudaSuccess){
		printf("=======================================================\n");
		printf("device %i: %s\n",device_id,device_prop.name);
		printf("-------------------------------------------------------\n");
		printf("   totalGlobalMem   |	%lu \n", device_prop.totalGlobalMem);
		printf("      warpSize      |   %d \n", device_prop.warpSize);
		printf(" maxThreadsPerBlock |   %d \n", device_prop.maxThreadsPerBlock);
		printf("  sharedMemPerBlock |   %lu \n", device_prop.totalConstMem);
		printf("    totalConstMem   |   %lu \n", device_prop.totalConstMem);
		printf("=======================================================\n");
	}
}

template<typename DTYPE>
inline DTYPE* cuda_malloc(int num,int length)
{
	DTYPE* data_;
	res = cudaMalloc((void**) (&data_), num * length * sizeof(float_t));
	CUDA_CHECK(res);
	return data_;
}

template<typename DTYPE>
inline DTYPE* cuda_malloc_v(int num,int length,DTYPE value)
{
	DTYPE* data_;
	res = cudaMalloc((void**) (&data_), num * length * sizeof(float_t));
	vector<DTYPE> d(num * length, value);
	res = cudaMemcpy((void*) (data_), (void*) (&d[0]), num * length * sizeof(DTYPE), cudaMemcpyHostToDevice);
	CUDA_CHECK(res);
	vector<DTYPE>().swap(d);
	return data_;
}

template<typename DTYPE>
inline void cuda_setvalue(DTYPE *data_,DTYPE value, int length)
{
	vector<DTYPE> v(length,value);
	res = cudaMemcpy((void*) (data_), (void*) (&v[0]),	length * sizeof(DTYPE), cudaMemcpyHostToDevice);
	CUDA_CHECK(res);
	vector<DTYPE>().swap(v);
}

template<typename DTYPE>
inline void cuda_refresh(DTYPE *data_, int length)
{
	res = cudaMemset((void*) (data_), (DTYPE)0 ,length * sizeof(DTYPE));
	CUDA_CHECK(res);
}

template<typename DTYPE>
inline void cuda_copy2dev(DTYPE *d_data_, DTYPE* s_values, int length)
{
	res = cudaMemcpy((void*) (d_data_), (void*) (s_values),	length * sizeof(DTYPE), cudaMemcpyHostToDevice);
	CUDA_CHECK(res);
}

template<typename DTYPE>
inline void cuda_copy2host(DTYPE *d_data_,DTYPE* s_values, int length)
{
	res = cudaMemcpy((void*) (d_data_), (void*) (s_values),	length * sizeof(DTYPE), cudaMemcpyDeviceToHost);
	CUDA_CHECK(res);
}

template<typename DTYPE>
inline void cuda_free(DTYPE* data_)
{
	cudaFree(data_);
}

template<typename DTYPE>
inline void cuda_print(DTYPE* data_,int length)
{
	vector<DTYPE> v(length);
	cuda_copy2host(&v[0],data_,length);
	for(int i = 0; i < length ;++i)
		cout<< v[i] << ",";
	cout<<endl;
	vector<DTYPE>().swap(v);
}



#endif
