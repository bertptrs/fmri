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

using namespace std;
using namespace fmri;

struct
{
    optional<vector<string>> labels;
    vector<vector<LayerData>> data;
    float angle = 0;
    map<tuple<string, int, int>, GLuint> textureMap;
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

static void render()
{
    // Clear Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    configureCamera();

    glRotatef(rendererData.angle, 0.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLES);
    glVertex3f(-2.0f, -2.0f, 0.0f);
    glVertex3f(2.0f, 0.0f, 0.0);
    glVertex3f(0.0f, 2.0f, 0.0);
    glEnd();

    rendererData.angle += 0.1f;

    glutSwapBuffers();
}

static void reloadTextures(unsigned dataIndex)
{
    // First, release any existing textures
    for (auto &entry : rendererData.textureMap) {
        glDeleteTextures(0, &entry.second);
    }

    rendererData.textureMap.clear();

    const auto &dataSet = rendererData.data[dataIndex];

    for (auto &layer : dataSet) {
        auto dimensions = layer.shape();
        if (dimensions.size() != 4) {
            continue;
        }

        const auto images = dimensions[0],
                channels = dimensions[1],
                width = dimensions[2],
                height = dimensions[3];

        auto dataPtr = layer.data();
        for (auto i = 0; i < images; ++i) {
            for (auto j = 0; j < channels; ++j) {
                rendererData.textureMap[make_tuple(layer.name(), i, j)] = loadTexture(dataPtr, width, height);
                dataPtr += width * height;
            }
        }
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
    glutIdleFunc(render);
    glutReshapeFunc(changeWindowSize);
    registerCameraControls();

    glewInit();
    if (!GLEW_VERSION_2_0) {
        cerr << "OpenGL 2.0 not available" << endl;
        return 2;
    }

    reloadTextures(0);

    // Enable depth test to fix objects behind you
    glEnable(GL_DEPTH_TEST);

    // Start visualisation
    glutMainLoop();

    google::ShutdownGoogleLogging();

    return 0;
}
