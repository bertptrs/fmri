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
            Output,
            Other
        };

        LayerInfo(std::string_view name, std::string_view type,
                  const std::vector<boost::shared_ptr<caffe::Blob<DType>>> &parameters);

        const std::string& name() const;
        Type type() const;

        static Type typeByName(std::string_view name);

    private:
        std::vector<boost::shared_ptr<caffe::Blob<DType>>> parameters_;
        Type type_;
        std::string name_;
    };
}
