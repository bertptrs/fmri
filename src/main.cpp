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
    vector<LayerData>* currentData = nullptr;
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

static void renderLayer(const LayerData& data);

static void render()
{
    // Clear Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    auto& camera = Camera::instance();

    camera.configureRenderingContext();

    glPushMatrix();
    for (auto& layer : *rendererData.currentData) {
        glPushMatrix();
        renderLayer(layer);
        glPopMatrix();
        glTranslatef(-5, 0, 0);
    }
    glPopMatrix();

    glutSwapBuffers();
}

static void drawOneParticle()
{
    // Code taken from CG workshop 1. Should probably replace with something nicer.
    glBegin(GL_TRIANGLE_STRIP);
    // triangle 1
    glVertex3f(-0.5, 0.0, 0.5); // A
    glVertex3f(0.0, 0.0, -0.5); // B
    glVertex3f(0.0, 1.0, 0.0); // top
    // triangle 2
    glVertex3f(0.5, 0.0, 0.5); // C
    // triangle 3
    glVertex3f(-0.5, 0.0, 0.5); // A again
    // triangle 4 (bottom)
    glVertex3f(0.0, 0.0, -0.5); // B again
    glEnd();
}

static void renderFlatLayer(const LayerData& data)
{
    auto& shape = data.shape();
    CHECK_EQ(shape[0], 1) << "Should have only one instance per layer." << endl;
    // Draw one triangle for every point in the layer
    // Color depends on current value.
    vector<float> intensities(data.data(), data.data() + data.numEntries());
    transform(intensities.begin(), intensities.end(), intensities.begin(), [](auto x) { return clamp(x, -1.0f, 1.0f);});

    glPushMatrix();
    for (auto i : intensities) {
        auto intensity = min(-log(abs(i)) / 20.0f, 1.0f);
        if (i > 0) {
            glColor3f(intensity, intensity, 1);
        } else {
            glColor3f(1, intensity, intensity);
        }
        drawOneParticle();
        glTranslatef(0, 0, -2);
    }

    glPopMatrix();


}

static void renderText(string_view text)
{
    constexpr auto font = GLUT_BITMAP_HELVETICA_10;
    glRasterPos2i(0, 0);
    for (char c : text) {
        glutBitmapCharacter(font, c);
    }
}

static void renderLayer(const LayerData& data)
{
    auto& shape = data.shape();
    // Draw the name of the layer for reference.
    glColor3f(0.5, 0.5, 0.5);
    renderText(data.name());

    glTranslatef(0, 0, -2);
    switch (shape.size()) {
        case 4:
            // TODO: implement this.
            return;

        case 2:
            renderFlatLayer(data);
            return;

        default:
            cerr << "What is this even: " << data << endl;
    }
}

static void reloadTextures(unsigned dataIndex)
{
    // First, release any existing textures
    for (auto &entry : rendererData.textureMap) {
        glDeleteTextures(0, &entry.second);
    }

    rendererData.textureMap.clear();
    rendererData.currentData = &rendererData.data[dataIndex];

    for (auto &layer : *rendererData.currentData) {
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

    Camera::instance().registerControls();

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
