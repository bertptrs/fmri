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
    } else {
        LOG(INFO) << "Received unknown layer type: " << name << endl;
        return Type::Other;
    }
}

LayerInfo::LayerInfo(string_view name, string_view type,
                     const vector<boost::shared_ptr<caffe::Blob<DType>>> &parameters)
: parameters_(parameters), name_(name), type_(typeByName(type))
{

}

const std::string &LayerInfo::name() const
{
    return name_;
}
