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

	class dropout_op : public operator_base
	{

	public:

		dropout_op(blob_base *&data, args *&args_) : operator_base(data, args_){
			check();

			int input_dim = data->width();
			int channel = data->channel();
			int num = data->num();
#if __USEMBEDDING__ == ON
			o_blob = data;
			_rand_vect = cacu_allocator::create_em_blob(num,channel,input_dim,input_dim, test);
#else
			o_blob = data;
			_rand_vect = cacu_allocator::create_blob(num,channel,input_dim,input_dim, test);
#endif
			echo();
		};

		~dropout_op(){
			delete _rand_vect;
		};

		virtual const void check() override{
			return;
		}

		virtual const void op() override {

			float_t scale_ = 1.0 / (1 - _ratio);

#if __USEMBEDDING__ == ON
			em_blob *o_blob_ = (em_blob*)o_blob;
			em_blob *s_blob_ = (em_blob*)s_blob;
			em_blob *rand_vect_ = (em_blob*)_rand_vect;

			if(train == s_blob_->phrase())
			{
				for(int i = 0; i < s_blob_->num(); ++i)
				{
					rand_vector(rand_vect_->p_data_d(i),rand_vect_->length(), 1 - _ratio);
					cacu_ssx(rand_vect_->p_data_d(i), o_blob_->length(), o_blob_->p_data_d(i));
					cacu_scalex(o_blob_->p_data_d(i), o_blob_->length(), scale_);
					o_blob_->_sync(i);
					rand_vect_->_sync(i);
				}
			}
#else
			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;
			blob *rand_vect_ = (blob*)_rand_vect;

			if(train == s_blob_->phrase())
			{
				rand_vector(rand_vect_->s_data(),rand_vect_->count(), 1 - _ratio);
				cacu_ssx(rand_vect_->s_data(), o_blob_->count(), o_blob_->s_data());
				cacu_scalex(o_blob_->s_data(), o_blob_->count(), scale_);
			}
#endif
		}

		virtual const void grad() override{


			float_t scale_ = 1.0 / (1 - _ratio);
#if __USEMBEDDING__ == ON
			em_blob *o_blob_ = (em_blob*)o_blob;
			em_blob *s_blob_ = (em_blob*)s_blob;
			em_blob *rand_vect_ = (em_blob*)_rand_vect;

			if(train == s_blob_->phrase())
			{
				for(int i = 0; i < s_blob_->num(); ++i){
					//ratio's scale implementation
					cacu_ssx(rand_vect_->p_data_d(i),s_blob_->length(),s_blob_->p_diff_d(i));
					cacu_scalex(s_blob_->p_diff_d(i), o_blob_->length(), scale_);
					s_blob_->_sync(i);
				}
			}
#else
			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;
			blob *rand_vect_ = (blob*)_rand_vect;

			if(train == s_blob_->phrase())
			{
				//ratio's scale implementation
				cacu_ssx(rand_vect_->s_data(),s_blob_->count(),s_blob_->s_diff());
				cacu_scalex(s_blob_->s_diff(), o_blob_->count(), scale_);
			}
#endif
		}

		virtual const void load(std::ifstream& is) override{
			return;
		}

		virtual const void save(std::ostream& os) override{
			return;
		}

		virtual const void echo() override
		{
			LOG_INFO("create dropout op:");
			LOG_INFO("channel: %d, input_dim: %d, output_channel: %d, output_dim: %d",s_blob->channel(),s_blob->height(),o_blob->channel(),o_blob->height());
		}

		inline virtual const void LOOP_INIT_DATA_() override
		{
			_rand_vect->_RESET_DATA();
		}

		inline void set_ratio(float_t ratio_)
		{
			CHECK_GE_OP(0, ratio_, "ratio must be a positive decimal vs %f!", ratio_);
			CHECK_LE_OP(1, ratio_, "ratio must be a positive decimal vs %f!", ratio_);
			_ratio = ratio_;
		}


	private:

		blob_base *_rand_vect;

		float_t _ratio = 0.5f;
	};
};
