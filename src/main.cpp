#include <algorithm>
#include <iostream>
#include <glog/logging.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <map>
#include <cmath>

#include "LayerData.hpp"
#include "Options.hpp"
#include "Simulator.hpp"
#include "glutils.hpp"

using namespace std;
using namespace fmri;

struct
{
    optional<vector<string>> labels;
    vector<vector<LayerData>> data;
    float angle = 0;
    map<tuple<string, int, int>, GLuint> textureMap;

    struct
    {
        float pitch = 0;
        float yaw = 0;

        float x = 0, y = 0, z = -10;
    } camera;
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

static void configureCamera()
{
    glLoadIdentity();
    glRotatef(rendererData.camera.yaw, 0, 1, 0);
    glRotatef(rendererData.camera.pitch, 1, 0, 0);
    glTranslatef(rendererData.camera.x, rendererData.camera.y, rendererData.camera.z);
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

static void handleKeys(unsigned char key, int, int)
{
    constexpr float rotationScaling = 0.2f;
    constexpr float movementScaling = 0.2f;
    switch (key) {
        case 'a':
            // TODO: handle rotations
            rendererData.camera.x += movementScaling;
            break;

        case 'd':
            rendererData.camera.x -= rotationScaling;
            break;

        case 'w':
            rendererData.camera.z += movementScaling;
            break;

        case 's':
            rendererData.camera.z -= movementScaling;
            break;

        case 'q':
            // Utility quit function.
            exit(0);

        default:
            LOG(INFO) << "Received key: " << key << endl;
            break;
    }
}

static void handleMouseMove(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    rendererData.camera.yaw = (x - width) / width * 180;
    rendererData.camera.pitch = (y - height) / height * 90;
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
    glutKeyboardFunc(handleKeys);
    glutPassiveMotionFunc(handleMouseMove);

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
