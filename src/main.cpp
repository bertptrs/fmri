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
            for (unsigned int i = 0; i < res.size(); ++i) {
                cout << res[i] << " " << labels[i] << endl;
            }
        } else {
            cout << "Best result: " << *(max_element(res.begin(), res.end())) << endl;
        }

        cout << endl;
    }

    ::google::ShutdownGoogleLogging();

    return 0;
}
