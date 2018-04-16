#include <GL/glut.h>
#include <cmath>
#include <sstream>
#include <iostream>
#include "RenderingState.hpp"
#include "visualisations.hpp"
#include "Range.hpp"
#include "glutils.hpp"
#include "Simulator.hpp"

using namespace fmri;

static inline void toggle(bool &b)
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

static void updatePointSize(float dir)
{
    float size, granularity;
    glGetFloatv(GL_POINT_SIZE, &size);
    glGetFloatv(GL_POINT_SIZE_GRANULARITY, &granularity);
    granularity = std::max(0.5f, granularity);
    size += dir * granularity;
    glPointSize(std::max(1.f, size));
}

static void renderLoadingScreen()
{
    glLoadIdentity();
    glTranslatef(0, 0, -4);
    glRotatef(360 * getAnimationStep(std::chrono::seconds(4)), 0, 1, 0);
    glColor3f(1, 1, 1);
    glutWireTeapot(1);

    auto pulse = std::cos(2 * M_PI * getAnimationStep(std::chrono::seconds(3)));
    pulse *= pulse;
    glColor3d(pulse, pulse, 0);
    glLoadIdentity();
    setOrthographicProjection();
    renderText("Loading...", 5, 15);
    restorePerspectiveProjection();
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
            reset();
            break;

        case 'p':
            toggle(options.renderInteractionPaths);
            break;

        case 'o':
            toggle(options.activatedOnly);
            break;

        case '+':
            updatePointSize(1);
            break;

        case '-':
            updatePointSize(-1);
            break;

        default:
            // Do nothing.
            break;
    }

    glutPostRedisplay();
}

std::string RenderingState::debugInfo() const
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

void RenderingState::configureRenderingContext() const
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
    auto motionFunc = [](int x, int y) {
        RenderingState::instance().handleMouseAt(x, y);
    };
    glutPassiveMotionFunc(motionFunc);
    glutMotionFunc(motionFunc);

    glutKeyboardFunc([](auto key, auto, auto) {
        RenderingState::instance().handleKey(key);
    });
    glutDisplayFunc([]() {
        float time = getAnimationStep(std::chrono::seconds(5));
        RenderingState::instance().render(time);
    });
    glutIdleFunc([]() {
        RenderingState::instance().idleFunc();
    });
    glutSpecialFunc([](int key, int, int) {
        RenderingState::instance().handleSpecialKey(key);
    });
    glutMouseFunc([](int button, int state, int, int) {
        auto& options = RenderingState::instance().options;
        switch (button) {
            case GLUT_LEFT_BUTTON:
                options.mouse_1_pressed = state == GLUT_DOWN;
                break;
            case GLUT_RIGHT_BUTTON:
                options.mouse_2_pressed = state == GLUT_DOWN;
                break;

            default:
                // Do nothing.
                break;
        }
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

void RenderingState::queueUpdate()
{
    // Make sure that visualisations are cleared in the current thread
    layerVisualisations.clear();
    interactionAnimations.clear();

    visualisationFuture = std::async(std::launch::async, []() {
        RenderingState::instance().updateVisualisers();

        return true;
    });
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

    if (!isLoading()) {
        renderVisualisation(time);
    } else {
        renderLoadingScreen();
    }

    glutSwapBuffers();
}

void RenderingState::renderVisualisation(float time) const
{
    configureRenderingContext();

    glPushMatrix();

    // Ensure we render back-to-front for transparency
    if (angle[0] <= 0) {
            // Render from the first to the last layer.
            glTranslatef(-LAYER_X_OFFSET / 2 * currentData->size(), 0, 0);
            for (auto i : Range(currentData->size())) {
                drawLayer(time, i);
                glTranslatef(LAYER_X_OFFSET, 0, 0);
            }
        } else {
            // Render from the last layer to the first layer.
            glTranslatef(LAYER_X_OFFSET / 2 * (currentData->size() - 2), 0, 0);
            for (auto i = currentData->size(); i--;) {
                drawLayer(time, i);

                glTranslatef(-LAYER_X_OFFSET, 0, 0);
            }
        }

    glPopMatrix();

    renderOverlayText();
}

void RenderingState::drawLayer(float time, unsigned long i) const
{
    glPushMatrix();

    renderLayerName(currentData->at(i).name());
    if (options.renderLayers) {
                layerVisualisations[i]->draw(time);
            }
    if (options.renderInteractions && i < interactionAnimations.size() && interactionAnimations[i]) {
                interactionAnimations[i]->draw(time);
            }

    glPopMatrix();
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
                       "o: toggle activated nodes only\n"
                       "p: toggle interaction paths visible\n"
                       "+/-: increase/decrease particle size\n"
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
            queueUpdate();
            break;

        case GLUT_KEY_RIGHT:
            ++currentData;
            if (currentData == layerData.end()) {
                currentData = layerData.begin();
            }
            queueUpdate();
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

bool RenderingState::renderActivatedOnly() const
{
    return options.activatedOnly;
}

bool RenderingState::renderInteractionPaths() const
{
    return options.renderInteractionPaths;
}

void RenderingState::loadOptions(const Options &programOptions)
{
    options.pathColor = programOptions.pathColor();
    options.layerAlpha = programOptions.layerTransparancy();
    options.interactionAlpha = programOptions.interactionTransparancy();

    simulationFuture = std::async(std::launch::async, Simulator::loadSimulationData, programOptions);
}

const Color &RenderingState::pathColor() const
{
    return options.pathColor;
}

RenderingState::RenderingState() noexcept
{
    // Enable depth test to fix objects behind you
    glEnable(GL_DEPTH_TEST);

    // Nicer rendering
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set initial point size
    glPointSize(3);
}

float RenderingState::interactionAlpha() const
{
    return options.interactionAlpha;
}

float RenderingState::layerAlpha() const
{
    return options.layerAlpha;
}

/**
 * Attempt to wait for completion of a future, for less than a frame.
 *
 * @tparam T
 * @param f The future to wait for
 * @return The result of the computation, or an empty optional if it hasn't finished.
 */
template<class T>
static std::optional<T> awaitCompletion(std::future<T>& f)
{
    constexpr auto waitTime = std::chrono::milliseconds(10);
    switch (f.wait_for(waitTime)) {
        case std::future_status::timeout:
            return std::nullopt;

        case std::future_status::ready:
            return f.get();

        default:
            LOG(ERROR) << "loading status was deferred, invalid state!";
            abort();
    }
}


void RenderingState::idleFunc()
{
    if (isLoading()) {
        if (visualisationFuture.valid()) {
            auto result = awaitCompletion(visualisationFuture);
            if (result) {
                loadGLItems();
                return;
            }
        } else if (simulationFuture.valid()) {
            auto result = awaitCompletion(simulationFuture);
            if (result) {
                tie(layerInfo, layerData) = std::move(*result);
                currentData = layerData.begin();
                queueUpdate();
            }
        }
    } else {
        if (options.mouse_1_pressed) {
            move('w', false);
        }
        if (options.mouse_2_pressed) {
            move('s', false);
        }
        throttleIdleFunc();
    }
    glutPostRedisplay();
}

void RenderingState::loadGLItems()
{
    std::for_each(layerVisualisations.begin(), layerVisualisations.end(), [](auto& x) { x->glLoad(); });
    std::for_each(interactionAnimations.begin(), interactionAnimations.end(), [](auto& x) { if (x) x->glLoad(); });
}

bool RenderingState::isLoading() const
{
    return visualisationFuture.valid() || simulationFuture.valid();
}
