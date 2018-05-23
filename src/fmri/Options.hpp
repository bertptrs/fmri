#pragma once

#include <optional>
#include <string>
#include <vector>

#include "utils.hpp"
#include "PNGDumper.hpp"

namespace fmri {

    using std::vector;
    using std::string;

    class Options {
    public:
        Options(const int argc, char *const argv[]);

        const string& model() const;
        const string& weights() const;
        const string& means() const;
        const Color& pathColor() const;
        std::optional<vector<string>> labels() const;
        std::optional<fmri::PNGDumper> imageDumper() const;
        float layerTransparency() const;
        float interactionTransparency() const;

        const vector<string>& inputs() const;
        bool brainMode() const;
        int inputMillis() const;

    private:
        float layerTransparency_;
        float interactionTransparency_;
        Color pathColor_;
        string modelPath;
        string weightsPath;
        string meansPath;
        string labelsPath;
        string dumpPath;
        vector<string> inputPaths;
        bool brainMode_;
        int inputMillis_;
    };
}
