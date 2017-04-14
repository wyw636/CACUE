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

	class batch_normal_op : public operator_base
	{

	public:

		batch_normal_op(blob_base *&data, args *&args_) : operator_base(data, args_){

			check();
			o_blob = create_oblob(data->num(), data->channel(), data->width(), data->height(), _phrase);

			_scale = create_param("scale", data->channel(), 1, 1, 1, _phrase);
			_shift = create_param("shift", data->channel(), 1, 1, 1, _phrase);

			_mean = cacu_allocator::create_blob(data->channel(), 1, 1, 1, _phrase);
			_var = cacu_allocator::create_blob(data->channel(), 1, 1, 1,_phrase);

			_history_mean = cacu_allocator::create_blob(data->channel(), 1, 1, 1,test);
			_history_var = cacu_allocator::create_blob(data->channel(), 1, 1, 1,test);

			_std = cacu_allocator::create_blob(data->channel(), 1, 1, 1,_phrase);

			_dim_sum = cacu_allocator::create_blob(data->num(), data->channel(), 1, 1, test);

			//save for train
			if(train == _phrase)
				_x = cacu_allocator::create_blob(data->num(), data->channel(), data->width(), data->height(), test);
			echo();
		};

		~batch_normal_op(){

			delete _mean;
			delete _var;

			delete _history_mean;
			delete _history_var;

			delete _std;

			delete _dim_sum;

			if(train == _phrase)
				delete _x;
		};

		virtual const void check() override{
			//training for batch_size
			if(train == _phrase)
				CHECK_GT_OP(s_blob->num(), 1 ,"batch size for training must > 1 vs %d",s_blob->num());
		}

		virtual const void op() override {
			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;

			float_t m = (float_t)s_blob_->num()*s_blob_->width()*s_blob_->height();

			if(train == _phrase){

				float_t bias_correction_factor = m > (float_t)1.0 ? (m) / (m - (float_t)1.0) : (float_t)1.0;

				cacu_sumbysize(BYWIDTH, s_blob_->s_data(), s_blob_->count(),1, _dim_sum->s_data(),0, s_blob_->length()/s_blob_->channel());
				cacu_sumbysize(BYHEIGHT, _dim_sum->s_data(), s_blob_->channel() * s_blob_->num(),1, _mean->s_data(),0, s_blob_->channel());

				cacu_scalex(_mean->s_data(), _mean->count(), ((float_t)1.0 / m));

				for (int i = 0; i < s_blob_->num(); ++i)
					cacu_ssxpy(_mean->s_data(),(float_t)(-1),_mean->count(),s_blob_->p_data(i),(float_t)(1),s_blob_->length(),o_blob_->p_data(i));

				//for saving space here we use o_data for container calculate x^2
				cacu_sqr(o_blob_->s_data(), o_blob_->count(), o_blob_->s_data());

				cacu_sumbysize(BYWIDTH, o_blob_->s_data(), o_blob_->count(), 1,_dim_sum->s_data(), 0, o_blob_->length()/o_blob_->channel());
				cacu_sumbysize(BYHEIGHT, _dim_sum->s_data(), o_blob_->channel()*o_blob_->num(),1, _var->s_data(),0, o_blob_->channel());
				cacu_scalex(_var->s_data(), _var->count(), ((float_t)1.0 / m));

				//update history
				cacu_saxpby(_mean->s_data(), (float_t)(1), _history_mean->s_data(), moving_average_fraction, _mean->count());
				cacu_saxpby(_var->s_data(), bias_correction_factor, _history_var->s_data(), moving_average_fraction, _var->count());

				cacu_stdbychannel(_var->s_data(), _std->count(), _std->s_data(), epsilon);

				for (int i = 0; i < s_blob_->num(); ++i){
					cacu_ssxpy(_mean->s_data(), (float_t)(-1), _mean->count(), s_blob_->p_data(i), (float_t)(1), s_blob_->length(), o_blob_->p_data(i));
					cacu_cdxsize(o_blob_->p_data(i), o_blob_->length(), _std->s_data(), _std->count(), o_blob_->p_data(i));
					//save for train
					cacu_copy(o_blob_->p_data(i),o_blob_->length(),_x->p_data(i));
					cacu_cxsize(o_blob_->p_data(i), o_blob_->length(), _scale->s_data(), _scale->count(), o_blob_->p_data(i));
					cacu_ssxpy(_shift->s_data(), (float_t)(1), _shift->count(), o_blob_->p_data(i), (float_t)(1), o_blob_->length(), o_blob_->p_data(i));

				}
			}
			else
			{
				if (!use_global_stats)
				{
					cacu_sumbysize(BYWIDTH, s_blob_->s_data(), s_blob_->count(),1, _dim_sum->s_data(),0, s_blob_->length()/s_blob_->channel());
					cacu_sumbysize(BYHEIGHT, _dim_sum->s_data(), s_blob_->channel() * s_blob_->num(),1, _mean->s_data(),0, s_blob_->channel());

					cacu_scalex(_mean->s_data(), _mean->count(), ((float_t)1.0 / m));

					for (int i = 0; i < s_blob_->num(); ++i)
						cacu_ssxpy(_mean->s_data(),(float_t)(-1),_mean->count(),s_blob_->p_data(i),(float_t)(1),s_blob_->length(),o_blob_->p_data(i));

					//for saving space here we use o_data for container calculate x^2
					cacu_sqr(o_blob_->s_data(), o_blob_->count(), o_blob_->s_data());

					cacu_sumbysize(BYWIDTH, o_blob_->s_data(), o_blob_->count(), 1,_dim_sum->s_data(), 0, o_blob_->length()/o_blob_->channel());
					cacu_sumbysize(BYHEIGHT, _dim_sum->s_data(), o_blob_->channel()*o_blob_->num(),1, _var->s_data(),0, o_blob_->channel());
					cacu_scalex(_var->s_data(), _var->count(), ((float_t)1.0 / m));

					cacu_stdbychannel(_var->s_data(), _std->count(), _std->s_data(), epsilon);
					for (int i = 0; i < s_blob_->num(); ++i){
						cacu_ssxpy(_mean->s_data(), (float_t)(-1), _mean->count(), s_blob_->p_data(i), (float_t)(1), s_blob_->length(), o_blob_->p_data(i));
						cacu_cdxsize(o_blob_->p_data(i), o_blob_->length(), _std->s_data(), _std->count(), o_blob_->p_data(i));
						cacu_cxsize(o_blob_->p_data(i), o_blob_->length(), _scale->s_data(), _scale->count(), o_blob_->p_data(i));
						cacu_ssxpy(_shift->s_data(), (float_t)(1), _shift->count(), o_blob_->p_data(i), (float_t)(1), o_blob_->length(), o_blob_->p_data(i));
					}
				}
				else{
					cacu_stdbychannel(_history_var->s_data(), _std->count(), _std->s_data(), epsilon);

					for (int i = 0; i < s_blob_->num(); ++i){
						cacu_ssxpy(_history_mean->s_data(), (float_t)(-1), _mean->count(), s_blob_->p_data(i), (float_t)(1), s_blob_->length(), o_blob_->p_data(i));
						cacu_cdxsize(o_blob_->p_data(i), o_blob_->length(), _std->s_data(), _std->count(), o_blob_->p_data(i));
						cacu_cxsize(o_blob_->p_data(i), o_blob_->length(), _scale->s_data(), _scale->count(), o_blob_->p_data(i));
						cacu_ssxpy(_shift->s_data(), (float_t)(1), _shift->count(), o_blob_->p_data(i), (float_t)(1), o_blob_->length(), o_blob_->p_data(i));
					}
				}
			}

			//cout << "means:";
			//cuda_print(_history_mean->s_data(),3);
			//cout << "vars:";
			//cuda_print(_history_var->s_data(),3);
			//cout << "o_blob:";
			//cuda_print(o_blob_->s_data(),1000);
			//cout << endl << endl << endl;


		}

		virtual const void grad() override{

			blob *o_blob_ = (blob*)o_blob;
			blob *s_blob_ = (blob*)s_blob;

			float_t *mean_data_,*mean_diff_;

			for(int i = 0 ; i < s_blob_->num(); ++i){
				//calculate dl/x_
				cacu_cxsize(o_blob_->p_diff(i), o_blob_->length(), _scale->s_data(), _scale->count(), s_blob_->p_diff(i));
			}
			mean_data_ = _mean->s_data();
			mean_diff_ = _mean->s_diff();
			//calculate dl/std^2
			cacu_bn_rou_grad(s_blob_->s_data(), s_blob_->s_diff(), mean_data_, _std->s_data(), s_blob_->num(),s_blob_->length(),s_blob_->channel(), _std->s_diff());
			//calculate dl/mu
			cacu_bn_mu_grad(s_blob_->s_data(), s_blob_->s_diff(), mean_data_, _std->s_data(), _std->s_diff(), s_blob_->num(), s_blob_->length(), s_blob_->channel(), mean_diff_);
			//calculate dl/x
			cacu_bn_dx_grad(s_blob_->s_data(), s_blob_->s_diff(), mean_data_, _std->s_data(), _std->s_diff(), mean_diff_, s_blob_->num(), s_blob_->length(), s_blob_->channel(), s_blob_->s_diff());
			//gradient of scale
			cacu_bn_gamma_grad(_x->s_data(), o_blob_->s_diff(),o_blob_->num(), o_blob_->length(), o_blob_->channel(),_scale->s_diff());
			//gradient of shift
			cacu_sumbysize(BYWIDTH, o_blob_->s_diff(), o_blob_->count(), 1, _dim_sum->s_data(), 0, o_blob_->length() / o_blob_->channel());
			cacu_sumbysize(BYHEIGHT, _dim_sum->s_data(), s_blob_->channel() * s_blob_->num(), 1, _shift->s_diff(), 0, s_blob_->channel());

			//cout << "s_blob:";
			//cuda_print(s_blob_->s_diff(),1000);
			//cout << endl;
		}

		virtual const void load(std::ifstream& is) override {

			_history_mean->load(is);
			_history_var->load(is);
			_scale->load(is);
			_shift->load(is);
		}

		virtual const void save(std::ostream& os) override {

			_history_mean->serializa(os);
			_history_var->serializa(os);
			_scale->serializa(os);
			_shift->serializa(os);
		}

		virtual const void echo() override
		{
			LOG_INFO("create batch_normalize op:");
			LOG_INFO("channel: %d, input_dim: %d, output_channel: %d, output_dim: %d",s_blob->channel(),s_blob->height(),o_blob->channel(),o_blob->height());
		}

		inline virtual const void LOOP_INIT_DATA_() override
		{
			o_blob->_RESET_DATA();

			_scale->_RESET_DIFF();
			_shift->_RESET_DIFF();

			_mean->_RESET_DATA();
			_var->_RESET_DATA();

			_std->_RESET_DATA();

			if(train == _phrase)
				_x->_RESET_DATA();

		}

		inline weight* scale(){ return _scale; }

		inline weight* shift(){ return _shift; }

		bool use_global_stats = true;

		float_t moving_average_fraction = 0.999f;

		float_t epsilon = 0.00001f;

	private:

		weight *_scale;

		weight *_shift;

		blob *_mean;
		
		blob *_var;

		blob *_std;

		blob *_history_mean;

		blob *_history_var;

		blob *_dim_sum;

		blob *_x;

	};
};
