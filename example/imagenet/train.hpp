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

#include "./alex_net.h"
#include "./data_proc.hpp"


void train_net()
{
	int batch_size = 100;

	int max_iter = 5000;


	//set gpu device if training by gpu
#if __PARALLELTYPE__ == __GPU__
	cuda_set_device(1);
#endif

	network *net = create_cifar_quick_net(batch_size,train);

	sgd_solver *sgd = new sgd_solver(net);

	sgd->set_lr(0.001f);
	sgd->set_weight_decay(0.0001f);

	string datapath = "/home/seal/4T/imagenet/227X227_train/";
	string trainlist = "/home/seal/4T/imagenet/train_list.txt";
	string meanfile = "/home/seal/4T/imagenet/227X227_mean.binproto";

	vector<string> full_data;
	vector<vec_i> full_label;

	/**
	 * load mean data
	 */
	blob *mean_ = cacu_allocator::create_blob(1,3,227,227);
#if __PARALLELTYPE__ == __GPU__
	imageio_utils::load_mean_file_gpu(mean_->s_data(),meanfile);
#else
	imageio_utils::load_mean_file(mean_->s_data(),meanfile);
#endif
	/*
	 * read train list data into local memory
	 */
	ifstream is(trainlist);
	is.precision(numeric_limits<float>::digits10);
	if(!is)
		LOG_FATAL("file %s cannot be opened!",trainlist.c_str());
	string file_;
	while(getline(is,file_)){
		vector<string> vec = split(file_);
		full_data.push_back(vec[0]);
		full_label.push_back(vec_i(strtoul(vec[1].c_str(), NULL, 10)));
	}

	int ALL_DATA_SIZE = full_data.size();

	/**
	 * read data for training
	 */
	blob *input_data = (blob*)net->input_blobs()->at(0);
	bin_blob *input_label = (bin_blob*)net->input_blobs()->at(1);

	int step_index = 0;
	clock_t start,end;
	for (int i = 0 ; i < max_iter; ++i)
	{
		start = clock();
		for (int j = 0 ; j < batch_size ; ++j)
		{
			if (step_index == ALL_DATA_SIZE)
				step_index = 0;
			//load image data
			readdata(full_data[step_index],input_data->p_data(j),mean_->s_data());
			input_label->copy_data_io(full_label[step_index],j);
			step_index += 1;
		}
		sgd->train_iter();
		end = clock();

		if(i % 1 == 0){
			LOG_INFO("iter_%d, lr: %f, %ld ms/iter", i,sgd->lr(),end - start);
			((softmax_with_loss_op*)net->get_op(net->op_count()-1))->echo();
		}

		if(i == 50000)
			sgd->set_lr_iter(0.1f);

	}

	net->save_weights("/home/seal/4T/cacue/cifar10/data/cifar10_quick.model");
}
