#include <cstring>
#include <functional>
#include <iostream>
#include <numeric>

#include <glog/logging.h>

#include "LayerData.hpp"

using namespace fmri;
using namespace std;

LayerData::LayerData(const string& name, const vector<int>& shape, const DType* data, Type type) :
	name_(name),
	shape_(shape),
	type_(type)
{
	const auto dataSize = numEntries();
	// Compute the dimension of the data area
	data_.reset(new DType[dataSize]);

	// Copy the data over with memcpy because it's just faster that way
	memcpy(data_.get(), data, sizeof(DType) * dataSize);
}

size_t LayerData::numEntries() const
{
	return accumulate(shape_.begin(), shape_.end(), 1, multiplies<>());
}

const vector<int>& LayerData::shape() const
{
	return shape_;
}

typename LayerData::Type LayerData::type() const
{
	return type_;
}

const string& LayerData::name() const
{
	return name_;
}

DType const * LayerData::data() const
{
	return data_.get();
}

LayerData::Type LayerData::typeFromString(string_view typeName)
{
	if (typeName == "Input") {
		return Type::Input;
	} else if (typeName == "Convolution") {
		return Type::Convolutional;
	} else if (typeName == "ReLU") {
		return Type::ReLU;
	} else if (typeName == "Pooling") {
		return Type::Pooling;
	} else {
		LOG(INFO) << "Received unknown layer type: " << typeName << endl;
		return Type::Other;
	}
}

ostream& operator<< (ostream& o, const LayerData& layer)
{
    o << layer.name() << '(';
    bool first = true;

    for (auto d : layer.shape()) {
        if (!first) {
            o << ", ";
        } else {
            first = false;
        }

        o << d;
    }

    o << ')';

    return o;
}
