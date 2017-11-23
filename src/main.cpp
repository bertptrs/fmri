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
#include "FlatLayerVisualisation.hpp"
#include "MultiImageVisualisation.hpp"

using namespace std;
using namespace fmri;

struct
{
    optional<vector<string>> labels;
    vector<vector<LayerData>> data;
    float angle = 0;
    vector<LayerData>* currentData = nullptr;
    map<tuple<string, int, int>, GLuint> textureMap;
    vector<unique_ptr<LayerVisualisation>> layerVisualisations;
} rendererData;

static vector<vector<LayerData>> getSimulationData(const Options &options)
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

    return results;
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

    for (unsigned int i = 0; i < dataSet.size(); ++i) {
        glPushMatrix();
        renderLayerName(dataSet[i]);
        rendererData.layerVisualisations[i]->render();

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

static void updateVisualisers(unsigned int dataset)
{
    rendererData.layerVisualisations.clear();

    rendererData.currentData = &rendererData.data[dataset];
    for (auto &layer : *rendererData.currentData) {
        LayerVisualisation* visualisation = nullptr;
        switch (layer.shape().size()) {
            case 2:
                visualisation = new FlatLayerVisualisation(layer);
                break;

            case 4:
                visualisation = new MultiImageVisualisation(layer);
                break;

            default:
                abort();
        }

        rendererData.layerVisualisations.emplace_back(visualisation);
    }
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
    rendererData.data = getSimulationData(options);

    // Register callbacks
    glutDisplayFunc(render);
    glutIdleFunc(glutPostRedisplay);
    glutReshapeFunc(changeWindowSize);

    Camera::instance().registerControls();

    glewInit();
    if (!GLEW_VERSION_2_0) {
        cerr << "OpenGL 2.0 not available" << endl;
        return 2;
    }

    updateVisualisers(0);

    // Enable depth test to fix objects behind you
    glEnable(GL_DEPTH_TEST);

    // Start visualisation
    glutMainLoop();

    google::ShutdownGoogleLogging();

    return 0;
}
