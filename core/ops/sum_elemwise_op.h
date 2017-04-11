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

	class sum_elemwise_op : public operator_base
	{

	public:

		sum_elemwise_op(blobs *&data, args *&args_) : operator_base(data, args_){
			check();

			blob_base *_blob = data->at(0);
			o_blob = create_oblob(_blob->num(), _blob->channel(), _blob->width(), _blob->height(), _phrase);

			echo();
		};

		~sum_elemwise_op(){

		};

		virtual const void check() override{

			CHECK_GE_OP(s_blobs->size(), 2 , "blobs size must >= 2 vs %d",s_blobs->size());

		}

		virtual const void op() override {
			blob *o_blob_ = (blob*)o_blob;

			for (unsigned int j = 0; j < (s_blobs)->size(); ++j){
				blob *s_blob_ = (blob*)(*s_blobs)[j];
				cacu_saxpy(s_blob_->s_data(), (float_t)1, o_blob_->s_data(), o_blob_->count());
			}
		}

		virtual const void grad() override{
			blob *o_blob_ = (blob*)o_blob;

			for (unsigned int j = 0; j < (s_blobs)->size(); ++j){
				blob *s_blob_ = (blob*)(*s_blobs)[j];
				cacu_copy(o_blob_->s_diff(),o_blob_->count(),s_blob_->s_diff());
			}
		}

		virtual const void load(std::ifstream& is) override{
			return;
		}

		virtual const void save(std::ostream& os) override{
			return;
		}

		virtual const void echo() override{
			LOG_INFO("create sum_elemwise op:");
			LOG_INFO("channel: %d, input_dim: %d, output_channel: %d, output_dim: %d",s_blobs->at(0)->channel(),s_blobs->at(0)->height(),o_blob->channel(),o_blob->height());
		}

		inline virtual const void LOOP_INIT_DATA_() override
		{
			o_blob->_RESET_DATA();
		}

	private:

		
	};
};
