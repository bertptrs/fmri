#include <algorithm>
#include <iostream>
#include "Options.hpp"
#include "Simulator.hpp"
#include "utils.hpp"

using namespace std;
using namespace fmri;

int main(int argc, char *const argv[]) {
    google::InitGoogleLogging(argv[0]);

    Options options = Options::parse(argc, argv);
    vector<string> labels;
    if (options.labels() != "") {
        labels = read_vector<string>(options.labels());
    }

    Simulator simulator(options.model(), options.weights(), options.means());

	for (const auto &image : options.inputs()) {
		const auto res = simulator.simulate(image);
		LOG(INFO) << "Result for " << image << ":" << endl;

		const auto& resultRow = res[res.size() - 1];
		if (!labels.empty()) {
			vector<DType> weights(resultRow.data(), resultRow.data() + resultRow.numEntries());
			auto scores = combine(weights, labels);
			sort(scores.begin(), scores.end(), greater<>());
			for (unsigned int i = 0; i < scores.size() && i < 5; ++i) {
				LOG(INFO) << scores[i].first << " " << scores[i].second << endl;
			}
		} else {
			LOG(INFO) << "Best result: " << *(resultRow.data(), resultRow.data() + resultRow.numEntries()) << endl;
		}
	}

    google::ShutdownGoogleLogging();

    return 0;
}
