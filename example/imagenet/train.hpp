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

#include "alex_net.h"
#include "vgg_net.h"
#include "data_proc.h"
#include "resnet_18.h"

void train_net()
{
	int batch_size = 2;

	int max_iter = 2;


	//set gpu device if training by gpu
#if __PARALLELTYPE__ == __CUDA__
	cuda_set_device(0);
#endif

	network *net = create_res18net(batch_size,train);//create_vgg_16_net(batch_size,train);//create_alexnet(batch_size,train);

	net->load_weights("/home/seal/4T/cacue/imagenet/res18net_30000.model");	//net->load_weights("/home/seal/4T/cacue/imagenet/alex_net_20000.model");

	sgd_solver *sgd = new sgd_solver(net);

	sgd->set_lr(0.00001f);
	sgd->set_weight_decay(0.0005f);

	string datapath = "/home/seal/4T/imagenet/224X224_train/";
	string trainlist = "/home/seal/4T/imagenet/train_list.txt";
	string meanfile = "/home/seal/4T/imagenet/224X224_mean.binproto";

	vector<string> full_data;
	vector<vec_i> full_label;

	/**
	 * load mean data
	 */
	blob *mean_ = cacu_allocator::create_blob(1,3,224,224,test);
#if __PARALLELTYPE__ == __CUDA__
	imageio_utils::load_mean_file_gpu(mean_->s_data(),meanfile);
#else
	imageio_utils::load_mean_file(mean_->s_data(),meanfile);
#endif
	/**
	 * read train list data into local memory
	 */
	ifstream is(trainlist);
	is.precision(numeric_limits<float>::digits10);
	if(!is)
		LOG_FATAL("file %s cannot be opened!",trainlist.c_str());
	string file_;
	while(getline(is,file_)){
		vector<string> vec = split(file_," ");
		full_data.push_back(datapath + vec[0]);
		vec_i label(1);
		label[0] = strtoul(vec[1].c_str(), NULL, 10);
		full_label.push_back(label);
	}

	int ALL_DATA_SIZE = full_data.size();

	/**
	 * read data for training
	 */
	blob *input_data = (blob*)net->input_blobs()->at(0);
	bin_blob *input_label = (bin_blob*)net->input_blobs()->at(1);

	int step_index = 0;
	clock_t start,end;
	for (int i = 1 ; i <= max_iter; ++i)
	{
		start = clock();
		for (int j = 0 ; j < batch_size ; ++j)
		{
			if (step_index == ALL_DATA_SIZE)
				step_index = 0;
			//load image data
			readdata(full_data[step_index],input_data->p_data(j));//,mean_->s_data());
			input_label->copy_data_io(full_label[step_index],j);
			step_index += 1;
		}
		sgd->train_iter();
		//net->predict();
		end = clock();

		if(i % 1 == 0){
			LOG_INFO("iter_%d, lr: %f, %ld ms/iter", i,sgd->lr(),end - start);
			((softmax_with_loss_op*)net->get_op(net->op_count()-1))->echo();
		}

		if(i % 50000 == 0)
			sgd->set_lr_iter(0.1f);
		if(i % 20000 == 0){
			ostringstream oss;
			oss << "/home/seal/4T/cacue/imagenet/res_net_" << i << ".model";
			net->save_weights(oss.str());
		}
	}

	ostringstream oss;
	oss << "/home/seal/4T/cacue/imagenet/res_net_" << max_iter << ".model";
	net->save_weights(oss.str());

	for(int i = 0; i < full_label.size(); ++i)
	{
		vec_i().swap(full_label[i]);
	}
	vector<string>().swap(full_data);
#if __PARALLELTYPE__ == __CUDA__
	cuda_release();
#endif
}
