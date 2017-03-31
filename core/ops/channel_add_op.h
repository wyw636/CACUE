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

	class channel_add_op : public operator_base
	{

	public:

		channel_add_op(blobs *&data, args *&args_) : operator_base(data, args_){

			blob_base* data_ = data->at(0);

			int input_dim = data_->width();
			int channel = data_->channel();
			int num = data_->num();

			int output_dim = (input_dim - _args->kernel_size()) / _args->stride() + 1;
			int pad = abs(input_dim - (output_dim - 1) * _args->stride() - _args->kernel_size());
			if (pad != 0)
				output_dim += 1;
			o_blob = cacu_allocator::create_blob(num, channel, output_dim, output_dim, _phrase);

		};

		~channel_add_op(){

		};

		virtual const void check() override{
			//kernel_size > 0
			CHECK_GT_OP(_args->kernel_size(), 0,"kernel_size must > 0 vs %d",_args->kernel_size());
		}

		virtual const void op() override {

			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;

			for(int i = 0 ; i < s_blob_->num(); ++i)
				cacu_average_pooling(s_blob_->p_data(i), _args->kernel_size(), _args->stride(), s_blob_->width(), o_blob_->width(), s_blob_->channel(), o_blob_->p_data(i));

		}


		virtual const void grad() override {
			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;

			for(int i = 0 ; i < s_blob_->num(); ++i)
				cacu_average_pooling_grad(o_blob_->p_diff(i), _args->kernel_size(), _args->stride(), s_blob_->width(), o_blob_->width(), s_blob_->channel(), s_blob_->p_diff(i));
		}

		virtual const void load(std::ifstream& is) override {
			return;
		}

		virtual const void save(std::ostream& os) override {
			return;
		}

		virtual const void echo() override {
			return;
		}

		inline virtual const void LOOP_INIT_DATA_() override {

			o_blob->_RESET_DATA();
		}

	private:


	};
};