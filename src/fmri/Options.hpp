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
        const string modelPath;
        const string weightsPath;
        const string meansPath;
        const string labelsPath;
        const string dumpPath;
        const vector<string> inputPaths;

        Options(string &&, string &&, string&&, string&&, string&&, vector<string> &&) noexcept;
    };
}
