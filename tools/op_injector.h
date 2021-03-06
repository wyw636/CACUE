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

#include <map>
#include "../mycnn.h"

using namespace std;
using namespace mycnn;

class op_injector{

public:

	op_injector(operator_base *&op_){

		_neural_count = new map<unsigned int,unsigned int>();
		_op = op_;
	};

	~op_injector(){

		delete _neural_count;
	};

	inline void add(unsigned int index_)
	{
		map<unsigned int,unsigned int>::iterator it = _neural_count->find(index_);
		if(it != _neural_count->end()){
			(*_neural_count)[index_] = (*_neural_count)[index_] + 1;
		}
		else
			(*_neural_count)[index_] = 1;
	}

	inline map<unsigned int,unsigned int> *distribute()
	{
		return _neural_count;
	}

	void get_outblob_count()
	{
		blob *o_blob_ = _op->out_data<blob>();
		for(int n = 0 ; n < o_blob_->num(); ++n)
		{
			for(unsigned int i = 0 ; i < o_blob_->length(); ++i)
			{
				if(o_blob_->p_data(n)[i] > 0)
				{
					add(i);
				}
			}
		}
	}

	void serializa(string output_path)
	{
		std::ofstream os(output_path, ios::binary);
		os.precision(std::numeric_limits<float_t>::digits10);

		map<unsigned int,unsigned int>::iterator it = _neural_count->begin();

		while(it != _neural_count->end())
		{
			os << it->first << " " << it->second << endl;
		    it ++;
		}
		os.close();
	}

private:


	map<unsigned int,unsigned int> *_neural_count;

	operator_base *_op;

};
