#include <algorithm>
#include <iostream>
#include "Options.hpp"
#include "Simulator.hpp"
#include "utils.hpp"

using namespace std;
using namespace fmri;

int main(int argc, char *const argv[]) {
    ::google::InitGoogleLogging(argv[0]);

    Options options = Options::parse(argc, argv);
    vector<string> labels;
    if (options.labels() != "") {
        labels = read_vector<string>(options.labels());
    }

    Simulator simulator(options.model(), options.weights(), options.means());

    for (const auto &image : options.inputs()) {
        cout << "Result for " << image << ":" << endl;
        auto res = simulator.simulate(image);
        if (!labels.empty()) {
            auto scores = combine(res, labels);
            sort(scores.begin(), scores.end(), greater<>());
            for (unsigned int i = 0; i < scores.size() && i < 5; ++i) {
                cout << scores[i].first << " " << scores[i].second << endl;
            }
        } else {
            cout << "Best result: " << *(max_element(res.begin(), res.end())) << endl;
        }

        cout << endl;
    }

    ::google::ShutdownGoogleLogging();

    return 0;
}
