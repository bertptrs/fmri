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
        LayerData(const string &name, const vector<int> &shape, const DType *data);
        LayerData(const LayerData &) = delete;

        LayerData(LayerData &&) = default;
        LayerData &operator=(const LayerData &) = delete;
        LayerData &operator=(LayerData &&) = default;

        const string &name() const;
        const vector<int> &shape() const;
        DType const *data() const;
        std::size_t numEntries() const;

        const DType& operator[] (std::size_t i) const;
    private:
        string name_;
        vector<int> shape_;
        unique_ptr<DType[]> data_;
    };
}

std::ostream& operator<<(std::ostream&, const fmri::LayerData&);
