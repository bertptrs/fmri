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
        PNGDumper(PNGDumper&&) = default;
        PNGDumper(const PNGDumper&) = default;

        void dump(const LayerData& layerData);

    private:
        const string baseDir_;

        void dumpImageSeries(const LayerData &data);

        string getFilename(const string &basic_string, int i, int j);
    };
}