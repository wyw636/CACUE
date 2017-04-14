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

namespace mycnn{

	class max_pooling_op : public operator_base
	{

	public:

		max_pooling_op(blob_base *&data, args *&args_) : operator_base(data, args_){
			check();

			int input_dim = data->width();
			int channel = data->channel();
			int num = data->num();
			int output_dim = (input_dim - _args->kernel_size()) / _args->stride() + 1;
			int pad = abs(input_dim - (output_dim - 1) * _args->stride() - _args->kernel_size());
			if (pad != 0)
				output_dim += 1;

#if __USDYNAMIC__ == ON
			o_blob = create_dy_oblob(num, channel, output_dim, output_dim, _phrase);
			_index = cacu_allocator::create_dy_bin_blob(num, channel, output_dim, output_dim, test);
#else
			o_blob = create_oblob(num, channel, output_dim, output_dim, _phrase);
			_index = cacu_allocator::create_bin_blob(num, channel, output_dim, output_dim, test);
#endif

			echo();
		};

		~max_pooling_op(){
			delete _index;
		};

		virtual const void check() override{
			//kernel_size > 0
			CHECK_GT_OP(_args->kernel_size(), 0,"kernel_size must > 0 vs %d",_args->kernel_size());
		}

		virtual const void op() override {
#if __USDYNAMIC__ == ON
			dy_blob *o_blob_ = (dy_blob*)o_blob;
			dy_blob *s_blob_ = (dy_blob*)s_blob;
			dy_bin_blob *index_ = (dy_bin_blob*)_index;
			for(int i = 0 ; i < s_blob_->num(); ++i){
				cacu_max_pooling(s_blob_->p_data_d(i), _args->kernel_size(), _args->stride(), s_blob_->width(), o_blob_->width(), s_blob_->channel(), o_blob_->p_data_d(i), index_->p_data_d(i));
				o_blob_->switch_dev2host();
				index_->switch_dev2host();
			}
#else
			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;
			bin_blob *index_ = (bin_blob*)_index;
			for(int i = 0 ; i < s_blob_->num(); ++i)
				cacu_max_pooling(s_blob_->p_data(i), _args->kernel_size(), _args->stride(), s_blob_->width(), o_blob_->width(), s_blob_->channel(), o_blob_->p_data(i), index_->p_data(i));
#endif

		}

		virtual const void grad() override{

#if __USDYNAMIC__ == ON
			dy_blob *o_blob_ = (dy_blob*)o_blob;
			dy_blob *s_blob_ = (dy_blob*)s_blob;
			dy_bin_blob *index_ = (dy_bin_blob*)_index;
			for(int i = 0 ; i < s_blob_->num(); ++i){
				cacu_max_pooling_grad(o_blob_->p_diff_d(i), _args->kernel_size(), _args->stride(), s_blob_->width(), o_blob_->width(), s_blob_->channel(), s_blob_->p_diff_d(i), index_->p_data_d(i));
				s_blob_->switch_dev2host();
			}
#else
			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;
			bin_blob *index_ = (bin_blob*)_index;
			for(int i = 0 ; i < s_blob_->num(); ++i)
				cacu_max_pooling_grad(o_blob_->p_diff(i), _args->kernel_size(), _args->stride(), s_blob_->width(), o_blob_->width(), s_blob_->channel(), s_blob_->p_diff(i), index_->p_data(i));
#endif
		}

		virtual const void load(std::ifstream& is) override{
			return;
		}

		virtual const void save(std::ostream& os) override{
			return;
		}

		virtual const void echo() override{
			LOG_INFO("create max_pooling op:");
			LOG_INFO("channel: %d, input_dim: %d, output_channel: %d, output_dim: %d",s_blob->channel(),s_blob->height(),o_blob->channel(),o_blob->height());
		}

		inline virtual const void LOOP_INIT_DATA_() override
		{
			o_blob->_RESET_DATA();
			_index->_RESET_DATA();
		}

	private:

		blob_base *_index;

	};
};
