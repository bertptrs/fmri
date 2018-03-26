#pragma once

#include <string_view>
#include <caffe/blob.hpp>
#include <string>
#include "utils.hpp"

namespace fmri
{
    class LayerInfo
    {
    public:
        enum class Type
        {
            Input,
            Convolutional,
            ReLU,
            Pooling,
            InnerProduct,
            DropOut,
            LRN,
            Split,
            Softmax,
            Other
        };

        LayerInfo(std::string_view name, std::string_view type,
                  const std::vector<boost::shared_ptr<caffe::Blob<DType>>> &parameters);

        const std::string& name() const;
        Type type() const;
        const std::vector<boost::shared_ptr<caffe::Blob<DType>>>& parameters() const;

        static Type typeByName(std::string_view name);

        friend std::ostream& operator<<(std::ostream& out, Type type);

    private:
        std::vector<boost::shared_ptr<caffe::Blob<DType>>> parameters_;
        Type type_;
        std::string name_;

        const static std::unordered_map<std::string_view, Type> NAME_TYPE_MAP;
    };

    std::ostream& operator<<(std::ostream& out, LayerInfo::Type type);
}
