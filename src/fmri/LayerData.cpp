#include <cstring>
#include <functional>
#include <iostream>
#include <numeric>

#include <glog/logging.h>

#include "LayerData.hpp"

using namespace fmri;
using namespace std;

LayerData::LayerData(const string& name, const vector<int>& shape, const DType* data) :
	name_(name),
	shape_(shape)
{
	const auto dataSize = numEntries();
	// Compute the dimension of the data area
	data_.reset(new DType[dataSize]);

	// Copy the data over with memcpy because it's just faster that way
	memcpy(data_.get(), data, sizeof(DType) * dataSize);
}

size_t LayerData::numEntries() const
{
	return static_cast<size_t>(accumulate(shape_.begin(), shape_.end(), 1, multiplies<>()));
}

const vector<int>& LayerData::shape() const
{
	return shape_;
}

const string& LayerData::name() const
{
	return name_;
}

DType const * LayerData::data() const
{
	return data_.get();
}

const DType &LayerData::operator[](std::size_t i) const
{
    return data_[i];
}

DType const *LayerData::begin() const
{
    return data();
}

DType const *LayerData::end() const
{
    return data() + numEntries();
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
