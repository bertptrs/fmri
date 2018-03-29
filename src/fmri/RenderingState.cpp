#include <GL/glut.h>
#include <cmath>
#include <sstream>
#include <iostream>
#include "RenderingState.hpp"
#include "visualisations.hpp"
#include "Range.hpp"
#include "glutils.hpp"

using namespace fmri;

static inline void toggle(bool& b)
{
    b = !b;
}

static float getFPS()
{
    static int frames = 0;
    static float fps = 0;
    static auto timeBase = glutGet(GLUT_ELAPSED_TIME);

    ++frames;
    const auto time = glutGet(GLUT_ELAPSED_TIME);
    if (time - timeBase > 2000) {
        fps = frames * 1000.0f / (time - timeBase);
        frames = 0;
        timeBase = time;
    }


    return fps;
}

void RenderingState::move(unsigned char key, bool sprint)
{
    float speed = 0.5f;
    std::array<float, 3> dir;

    const auto yaw = deg2rad(angle[0]);
    const auto pitch = deg2rad(angle[1]);

    if (key == 'w' || key == 's') {
        dir[0] = std::sin(yaw) * std::cos(pitch);
        dir[1] = -std::sin(pitch);
        dir[2] = -std::cos(yaw) * std::cos(pitch);
    } else {
        dir[0] = -std::cos(yaw);
        dir[1] = 0;
        dir[2] = -std::sin(yaw);
    }

    if (key == 's' || key == 'd') {
        speed *= -1;
    }

    if (sprint) {
        speed *= 2;
    }

    for (auto i = 0u; i < dir.size(); ++i) {
        pos[i] += speed * dir[i];
    }
}

void RenderingState::handleKey(unsigned char x)
{
    switch (x) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            move(x, false);
            break;
        case 'W':
        case 'A':
        case 'S':
        case 'D':
            move(static_cast<unsigned char>(std::tolower(x)), true);
            break;

        case 'l':
            toggle(options.renderLayers);
            break;

        case 'i':
            toggle(options.renderInteractions);
            break;

        case 'q':
            exit(0);

        case 'h':
            RenderingState::instance().reset();
            break;

        default:
            // Do nothing.
            break;
    }

    glutPostRedisplay();
}

std::string RenderingState::debugInfo()const
{
    std::stringstream buffer;
    buffer << "Pos(x,y,z) = (" << pos[0] << ", " << pos[1] << ", " << pos[2] << ")\n";
    buffer << "Angle(p,y) = (" << angle[0] << ", " << angle[1] << ")\n";
    buffer << "FPS = " << getFPS() << "\n";
    return buffer.str();
}

void RenderingState::reset()
{
    pos[0] = 0;
    pos[1] = 0;
    pos[2] = 3;

    angle[0] = 0;
    angle[1] = 0;
}

void RenderingState::configureRenderingContext()const
{
    glLoadIdentity();
    glRotatef(angle[1], 1, 0, 0);
    glRotatef(angle[0], 0, 1, 0);
    glTranslatef(-pos[0], -pos[1], -pos[2]);
}

RenderingState &RenderingState::instance()
{
    static RenderingState state;
    return state;
}

void RenderingState::registerControls()
{
    reset();
    glutPassiveMotionFunc([](int x, int y) {
        RenderingState::instance().handleMouseAt(x, y);
    });
    glutKeyboardFunc([](auto key, auto, auto) {
        RenderingState::instance().handleKey(key);
    });
    glutDisplayFunc([]() {
        float time = getAnimationStep(std::chrono::seconds(5));
        RenderingState::instance().render(time);
    });
    glutIdleFunc([]() {
        checkGLErrors();
        throttleIdleFunc();
        glutPostRedisplay();
    });
    glutSpecialFunc([](int key, int, int) {
        RenderingState::instance().handleSpecialKey(key);
    });
}

void RenderingState::handleMouseAt(int x, int y)
{
    const float width = glutGet(GLUT_WINDOW_WIDTH) / 2.f;
    const float height = glutGet(GLUT_WINDOW_HEIGHT) / 2.f;

    angle[0] = (x - width) / width * 180;
    angle[1] = (y - height) / height * 90;

    glutPostRedisplay();
}

void RenderingState::loadSimulationData(const std::map<string, LayerInfo> &info, vector<vector<LayerData>> &&data)
{
    layerInfo = std::move(info);
    layerData = std::move(data);
    currentData = layerData.begin();

    updateVisualisers();
}

void RenderingState::updateVisualisers()
{
    layerVisualisations.clear();
    interactionAnimations.clear();
    LayerData *prevState = nullptr;
    LayerVisualisation *prevVisualisation = nullptr;

    for (LayerData &layer : *currentData) {
        LayerVisualisation *visualisation = getVisualisationForLayer(layer, layerInfo.at(layer.name()));
        if (prevState && prevVisualisation && visualisation) {
            auto interaction = getActivityAnimation(*prevState, layer, layerInfo.at(layer.name()),
                                                    prevVisualisation->nodePositions(), visualisation->nodePositions());
            interactionAnimations.emplace_back(interaction);
        }

        layerVisualisations.emplace_back(visualisation);

        prevVisualisation = visualisation;
        prevState = &layer;
    }

    glutPostRedisplay();
}

void RenderingState::render(float time) const
{
    // Clear Color and Depth Buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    configureRenderingContext();


    glPushMatrix();
    glTranslatef(5 * currentData->size(), 0, 0);

    for (auto i : Range(currentData->size())) {
        glPushMatrix();
        renderLayerName(currentData->at(i).name());
        if (options.renderLayers) {
            layerVisualisations[i]->render();
        }
        if (options.renderInteractions && i < interactionAnimations.size() && interactionAnimations[i]) {
            interactionAnimations[i]->draw(time);
        }

        glPopMatrix();
        glTranslatef(LAYER_X_OFFSET, 0, 0);
    }

    glPopMatrix();

    renderOverlayText();

    glutSwapBuffers();
}

void RenderingState::renderOverlayText() const
{
    std::stringstream overlayText;
    if (options.showDebug) {
        overlayText << debugInfo() << "\n";
    }

    if (options.showHelp) {
        overlayText << "Controls:\n"
                       "wasd: move\n"
                       "shift: move faster\n"
                       "F1: toggle this help message\n"
                       "F2: toggle debug info\n"
                       "l: toggle layers visible\n"
                       "i: toggle interactions visible\n"
                       "q: quit\n";
    }

    glLoadIdentity();
    setOrthographicProjection();
    glColor3f(1, 1, 0);
    renderText(overlayText.str(), 2, 10);
    restorePerspectiveProjection();
}

void RenderingState::renderLayerName(const std::string &name) const
{
    glColor3f(0.5, 0.5, 0.5);
    auto layerName = name;
    layerName += ": ";
    layerName += LayerInfo::nameByType(layerInfo.at(name).type());
    renderText(layerName);

    glTranslatef(0, 0, -10);
}

void RenderingState::handleSpecialKey(int key)
{
    switch (key) {
        case GLUT_KEY_LEFT:
            if (currentData == layerData.begin()) {
                currentData = layerData.end();
            }
            --currentData;
            updateVisualisers();
            break;

        case GLUT_KEY_RIGHT:
            ++currentData;
            if (currentData == layerData.end()) {
                currentData = layerData.begin();
            }
            updateVisualisers();
            break;

        case GLUT_KEY_F1:
            toggle(options.showHelp);
            glutPostRedisplay();
            break;

        case GLUT_KEY_F2:
            toggle(options.showDebug);
            break;

        default:
            LOG(INFO) << "Received keystroke " << key;
    }
}
