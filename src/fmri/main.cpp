#include <algorithm>
#include <iostream>
#include <glog/logging.h>
#include <GL/glut.h>
#include <map>

#include "LayerData.hpp"
#include "Options.hpp"
#include "Simulator.hpp"
#include "glutils.hpp"
#include "RenderingState.hpp"
#include "LayerVisualisation.hpp"
#include "Range.hpp"
#include "visualisations.hpp"

using namespace std;
using namespace fmri;

static void loadSimulationData(const Options &options)
{
    vector<vector<LayerData>> results;

    auto dumper = options.imageDumper();
    Simulator simulator(options.model(), options.weights(), options.means());

    for (const auto &image : options.inputs()) {
        results.emplace_back(simulator.simulate(image));
    }

    CHECK_GT(results.size(), 0) << "Should have some results" << endl;

    if (dumper) {
        for (auto &layer : *results.begin()) {
            dumper->dump(layer);
        }
    }

    const auto optLabels = options.labels();

    if (optLabels) {
        auto& labels = *optLabels;
        for (const auto& result : results) {
            auto& last = *result.rbegin();
            auto bestIndex = std::distance(last.data(), max_element(last.data(), last.data() + last.numEntries()));
            LOG(INFO) << "Got answer: " << labels[bestIndex] << endl;
        }
    }

    RenderingState::instance().loadSimulationData(simulator.layerInfo(), std::move(results));
}

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);
    glutCreateWindow(argv[0]);

    // Prepare data for simulations
    Options options(argc, argv);
    RenderingState::instance().loadOptions(options);
    loadSimulationData(options);

    // Register callbacks
    glutReshapeFunc(changeWindowSize);

    RenderingState::instance().registerControls();

    // Start visualisation
    glutMainLoop();

    google::ShutdownGoogleLogging();

    return 0;
}
