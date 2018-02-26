#include <algorithm>
#include <iostream>
#include <glog/logging.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <map>

#include "LayerData.hpp"
#include "Options.hpp"
#include "Simulator.hpp"
#include "glutils.hpp"
#include "camera.hpp"
#include "LayerVisualisation.hpp"
#include "Range.hpp"
#include "visualisations.hpp"

using namespace std;
using namespace fmri;

struct
{
    optional<vector<string>> labels;
    map<string, LayerInfo> layerInfo;
    vector<vector<LayerData>> data;
    vector<vector<LayerData>>::iterator currentData;
    vector<unique_ptr<LayerVisualisation>> layerVisualisations;
    vector<unique_ptr<Animation>> animations;
    float animationStep = 0;
} rendererData;

static void loadSimulationData(const Options &options)
{
    vector<vector<LayerData>> &results = rendererData.data;
    results.clear();

    auto dumper = options.imageDumper();
    Simulator simulator(options.model(), options.weights(), options.means());
    rendererData.layerInfo = simulator.layerInfo();

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
}

static void renderLayerName(const LayerData &data);

static void render()
{
    // Clear Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto& camera = Camera::instance();

    camera.configureRenderingContext();

    const auto& dataSet = *rendererData.currentData;

    glPushMatrix();
    glTranslatef(5 * dataSet.size(), 0, 0);

    for (auto i : Range(dataSet.size())) {
        glPushMatrix();
        renderLayerName(dataSet[i]);
        rendererData.layerVisualisations[i]->render();
        if (i < rendererData.animations.size() && rendererData.animations[i]) {
            rendererData.animations[i]->draw(rendererData.animationStep);
        }

        glPopMatrix();
        glTranslatef(-10, 0, 0);
    }


    glPopMatrix();

    glutSwapBuffers();
}

static void renderLayerName(const LayerData &data)
{
    // Draw the name of the layer for reference.
    glColor3f(0.5, 0.5, 0.5);
    renderText(data.name());

    glTranslatef(0, 0, -10);
}

static void updateVisualisers()
{
    rendererData.layerVisualisations.clear();
    rendererData.animations.clear();
    LayerData* prevState = nullptr;
    LayerVisualisation* prevVisualisation = nullptr;

    for (LayerData &layer : *rendererData.currentData) {
        LayerVisualisation* visualisation = getVisualisationForLayer(layer, rendererData.layerInfo.at(layer.name()));
        if (prevState && prevVisualisation && visualisation) {
            auto interaction = getActivityAnimation(*prevState, layer, rendererData.layerInfo.at(layer.name()), prevVisualisation->nodePositions(), visualisation->nodePositions());
            rendererData.animations.emplace_back(interaction);
        }

        rendererData.layerVisualisations.emplace_back(visualisation);

        prevVisualisation = visualisation;
        prevState = &layer;
    }

    glutPostRedisplay();
}

static void specialKeyFunc(int key, int, int)
{
    switch (key) {
        case GLUT_KEY_LEFT:
            if (rendererData.currentData == rendererData.data.begin()) {
                rendererData.currentData = rendererData.data.end();
            }
            --rendererData.currentData;
            updateVisualisers();
            break;

        case GLUT_KEY_RIGHT:
            ++rendererData.currentData;
            if (rendererData.currentData == rendererData.data.end()) {
                rendererData.currentData = rendererData.data.begin();
            }
            updateVisualisers();
            break;

        default:
            LOG(INFO) << "Received keystroke " << key;
    }
}

static void idleFunc()
{
    checkGLErrors();
    glutPostRedisplay();
    throttleIdleFunc();
    rendererData.animationStep = (1 - cos(M_PI * getAnimationStep(std::chrono::seconds(5)))) / 2;
}

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutCreateWindow(argv[0]);

    // Prepare data for simulations
    Options options = Options::parse(argc, argv);
    rendererData.labels = options.labels();
    loadSimulationData(options);

    // Register callbacks
    glutDisplayFunc(render);
    glutIdleFunc(idleFunc);
    glutReshapeFunc(changeWindowSize);
    glutSpecialFunc(specialKeyFunc);

    Camera::instance().registerControls();

    glewInit();
    if (!GLEW_VERSION_2_0) {
        cerr << "OpenGL 2.0 not available" << endl;
        return 2;
    }

    rendererData.currentData = rendererData.data.begin();
    updateVisualisers();

    // Enable depth test to fix objects behind you
    glEnable(GL_DEPTH_TEST);

    // Start visualisation
    glutMainLoop();

    google::ShutdownGoogleLogging();

    return 0;
}
