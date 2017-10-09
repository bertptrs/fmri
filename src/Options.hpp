#pragma once

#include <vector>
#include <string>

namespace fmri {

    using std::vector;
    using std::string;

    class Options {
    public:
        static Options parse(const int argc, char *const argv[]);

        const string& model() const;
        const string& weights() const;
        const string& means() const;
        const string& labels() const;

        const vector<string>& inputs() const;

    private:
        const string modelPath;
        const string weightsPath;
        const string meansPath;
        const string labelsPath;
        const vector<string> inputPaths;

        Options(string &&, string &&, string&&, string&&, vector<string> &&) noexcept;
    };
}
