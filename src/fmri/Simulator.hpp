#pragma once

#include <string>
#include <memory>
#include <vector>
#include "LayerData.hpp"
#include "LayerInfo.hpp"
#include "Options.hpp"

namespace fmri {
    using std::string;
    using std::vector;

    class Simulator {
    public:
        Simulator(const string &model_file, const string &weights_file, const string &means_file = "");
        ~Simulator();

        vector<LayerData> simulate(const string &input_file);
		const std::map<std::string, LayerInfo>& layerInfo() const;

		static std::pair<std::map<std::string, LayerInfo>, std::vector<std::vector<LayerData>>> loadSimulationData(const Options &options);

    private:
		struct Impl;
		std::unique_ptr<Impl> pImpl;
    };
}
