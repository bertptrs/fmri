#include "LayerInfo.hpp"

using namespace std;
using namespace fmri;

const unordered_map<string_view, LayerInfo::Type> LayerInfo::NAME_TYPE_MAP = {
        {"Input", Type::Input},
        {"Convolution", Type::Convolutional},
        {"ReLU", Type::ReLU},
        {"Pooling", Type::Pooling},
        {"InnerProduct", Type::InnerProduct},
        {"DropOut", Type::DropOut},
        {"LRN", Type::LRN},
        {"Split", Type::Split},
        {"Softmax", Type::Softmax}
};


LayerInfo::Type LayerInfo::typeByName(string_view name)
{
    try {
        return NAME_TYPE_MAP.at(name);
    } catch (std::out_of_range &e) {
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

std::ostream &fmri::operator<<(std::ostream &out, LayerInfo::Type type)
{
    for (auto i : LayerInfo::NAME_TYPE_MAP) {
        if (i.second == type) {
            out << i.first;
            return out;
        }
    }

    out << "ERROR! UNSUPPORTED TYPE";
    return out;
}

