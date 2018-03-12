#include "LayerInfo.hpp"

using namespace std;
using namespace fmri;


LayerInfo::Type LayerInfo::typeByName(string_view name)
{
    if (name == "Input") {
        return Type::Input;
    } else if (name == "Convolution") {
        return Type::Convolutional;
    } else if (name == "ReLU") {
        return Type::ReLU;
    } else if (name == "Pooling") {
        return Type::Pooling;
    } else if (name == "InnerProduct") {
        return Type::InnerProduct;
    } else if (name == "Dropout") {
        return Type::DropOut;
    } else if (name == "LRN") {
        return Type::LRN;
    } else {
        LOG(INFO) << "Received unknown layer type: " << name << endl;
        return Type::Other;
    }
}

LayerInfo::LayerInfo(string_view name, string_view type,
                     const vector<boost::shared_ptr<caffe::Blob<DType>>> &parameters)
: parameters_(parameters), type_(typeByName(type)), name_(name)
{
}

const std::string &LayerInfo::name() const
{
    return name_;
}

LayerInfo::Type LayerInfo::type() const
{
    return type_;
}

const std::vector<boost::shared_ptr<caffe::Blob<DType>>>& LayerInfo::parameters() const
{
    return parameters_;
}
