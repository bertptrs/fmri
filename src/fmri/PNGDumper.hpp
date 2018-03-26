#pragma once

#include <string>
#include <string_view>

#include "LayerData.hpp"
#include "utils.hpp"

namespace fmri
{
    using std::string;
    using std::string_view;

    class PNGDumper
    {
    public:
        PNGDumper(string_view baseDir);

        void dump(const LayerData& layerData);

    private:
        string baseDir_;

        void dumpImageSeries(const LayerData &data);

        string getFilename(const string &basic_string, int i, int j);
    };
}