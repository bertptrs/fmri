#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "utils.hpp"

namespace fmri
{

    using std::ostream;
    using std::string;
    using std::string_view;
    using std::unique_ptr;
    using std::vector;

    class LayerData
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

        LayerData(const string &name, const vector<int> &shape, const DType *data, Type type);
        LayerData(const LayerData &) = delete;

        LayerData(LayerData &&) = default;
        LayerData &operator=(const LayerData &) = delete;
        LayerData &operator=(LayerData &&) = default;

        const string &name() const;
        Type type() const;
        const vector<int> &shape() const;
        DType const *data() const;
        size_t numEntries() const;

        static Type typeFromString(string_view name);

    private:
        string name_;
        vector<int> shape_;
        unique_ptr<DType[]> data_;
        Type type_;
    };
}

std::ostream& operator<<(std::ostream&, const fmri::LayerData&);
