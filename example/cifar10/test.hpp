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


#include <time.h>

#include "../../mycnn.h"

#include "../../tools/imageio_utils.h"
#include "../../tools/op_injector.h"

#include "cifar_quick_net.h"
#include "data_proc.h"
#include "cifar_test_net.h"



void test_net()
{
	int batch_size = 100;

	int max_iter = 5000;

#if __PARALLELTYPE__ == __CUDA__
	cuda_set_device(0);
#endif

	network *net = create_cifar_quick_net(batch_size,test);//create_cifar_test_net(batch_size,test);

	op_injector *injector = new op_injector(net->get_op(4));

	string datapath = "/home/seal/4T/cacue/cifar10/data/";
	string meanfile = "/home/seal/4T/cacue/cifar10/data/mean.binproto";

	vector<vec_t> full_data;
	vector<vec_i> full_label;
	load_test_data_bymean(datapath, meanfile, full_data, full_label);

	vec_i _full_label;
	for(int i = 0; i < full_label.size(); ++i)
		_full_label.push_back(full_label[i][0]);

	blob *input_data = (blob*)net->input_blobs()->at(0);

	blob *output_data = net->output_blob();

	net->load_weights("/home/seal/4T/cacue/cifar10/data/cifar10_quick.model");

	unsigned int max_index;
	float_t count = 0;

	int step_index = 0;

	clock_t start,end;

	for (int i = 0 ; i < max_iter; ++i)
	{
		start = clock();

		for (int j = 0 ; j < batch_size ; ++j)
		{
			if (step_index == kCIFARBatchSize)
				break;
			input_data->copy_data_io(full_data[step_index], j);
			step_index += 1;
		}
		net->predict();
		//injector->get_outblob_count();
		for(int j = 0 ; j < batch_size ; ++j)
		{
			max_index = argmax(output_data->p_data(j),output_data->length());
			if(max_index == _full_label[i * batch_size + j]){
				count += 1.0;
			}
		}

		end = clock();

		if(i % 1 == 0){
			LOG_INFO("iter_%d , %ld ms/iter", i, end - start);
		}
		if (step_index == kCIFARBatchSize)
			break;
	}

	LOG_INFO("precious: %f,%f", count / kCIFARBatchSize,count);

	//injector->serializa("/home/seal/4T/cacue/cifar10/relu3.txt");

	delete injector;
#if __PARALLELTYPE__ == __CUDA__
	cuda_release();
#endif
}
