#pragma once

#include <optional>
#include <string>
#include <vector>

#include "PNGDumper.hpp"

namespace fmri {

    using std::vector;
    using std::string;

    class Options {
    public:
        static Options parse(const int argc, char *const argv[]);

        const string& model() const;
        const string& weights() const;
        const string& means() const;
        std::optional<vector<string>> labels() const;
        std::optional<fmri::PNGDumper> imageDumper() const;

        const vector<string>& inputs() const;

    private:
        std::array<float, 4> pathColor_;
        string modelPath;
        string weightsPath;
        string meansPath;
        char const * labelsPath;
        char const * dumpPath;
        vector<string> inputPaths;

        Options() noexcept;
    };
}
