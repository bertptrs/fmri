#include <GL/glut.h>
#include <cmath>
#include <sstream>
#include <iostream>
#include "RenderingState.hpp"
#include "visualisations.hpp"
#include "Range.hpp"
#include "glutils.hpp"
#include "Simulator.hpp"
#include "LabelVisualisation.hpp"

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

static unsigned int loadingPct;

static void renderLoadingScreen()
{
    glLoadIdentity();
    glTranslatef(0, 0, -4);
    glRotatef(360 * getAnimationStep(std::chrono::seconds(4)), 0, 1, 0);
    glColor3f(1, 1, 1);
    glutWireTeapot(1);

    char state[1024];
    std::snprintf(state, sizeof(state), "Loading... %u%%", loadingPct);

    auto pulse = std::cos(2 * M_PI * getAnimationStep(std::chrono::seconds(3)));
    pulse *= pulse;
    glColor3d(pulse, pulse, 0);
    glLoadIdentity();
    setOrthographicProjection();
    renderText(state, 5, 15);
    restorePerspectiveProjection();
}

typedef std::vector<std::vector<std::pair<std::unique_ptr<LayerVisualisation>, std::unique_ptr<Animation>>>> VisualisationList;

static VisualisationList loadVisualisations(const Options& options)
{
    using namespace std;

    Simulator simulator(options.model(), options.weights(), options.means());

    const auto layerInfo = simulator.layerInfo();
    auto labels = options.labels();

    VisualisationList result;

    auto dumper = options.imageDumper();

    for (auto& input : options.inputs()) {
        loadingPct = 100 * result.size() / options.inputs().size();
        LOG(INFO) << "Simulating " << input;
        auto item = simulator.simulate(input);

        vector<unique_ptr<LayerVisualisation>> layers;
        vector<unique_ptr<Animation>> animations;
        LayerData* prevData = nullptr;

        for (auto &layer : item) {
            unique_ptr<LayerVisualisation> layerVisualisation(getVisualisationForLayer(layer, layerInfo.at(layer.name())));

            if (prevData != nullptr) {
                auto animation = getActivityAnimation(*prevData, layer, layerInfo.at(layer.name()), (*layers.rbegin())->nodePositions(), layerVisualisation->nodePositions());
                animations.emplace_back(animation);
            }

            layers.emplace_back(move(layerVisualisation));
            prevData = &layer;

        }

        VisualisationList::value_type dataSet;

        if (labels) {
            auto &last = *item.rbegin();
            auto bestIndex = std::distance(last.data(), max_element(last.data(), last.data() + last.numEntries()));
            LOG(INFO) << "Got answer: " << labels->at(bestIndex) << endl;
            animations.emplace_back(new LabelVisualisation(layers.rbegin()->get()->nodePositions(), *prevData, labels.value()));
        }

        if (dumper) {
            for (auto &layer : item) {
                dumper->dump(layer);
            }
        }

        for (auto i = 0u; i < layers.size(); ++i) {
            auto interaction = i < animations.size() ? move(animations[i]) : nullptr;
            dataSet.emplace_back(move(layers[i]), move(interaction));
        }

        result.push_back(move(dataSet));
    }

    return result;
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
    if (isLoading()) {
        // Don't handle user input while loading.
        return;
    }
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

    auto& layer = currentData->at(i);

    layer.first->drawLayerName();
    if (options.renderLayers) {
        layer.first->draw(time);
    }
    if (layer.second) {
        if (options.renderInteractions) {
            layer.second->draw(time);
        }
        if (options.renderInteractionPaths) {
            layer.second->drawPaths();
        }
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

void RenderingState::handleSpecialKey(int key)
{
    if (isLoading()) {
        // Don't handle user input while loading.
        return;
    }
    switch (key) {
        case GLUT_KEY_LEFT:
            if (currentData == visualisations.begin()) {
                currentData = visualisations.end();
            }
            --currentData;
            break;

        case GLUT_KEY_RIGHT:
            ++currentData;
            if (currentData == visualisations.end()) {
                currentData = visualisations.begin();
            }
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

    loadingFuture = std::async(std::launch::async, loadVisualisations, programOptions);
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
        if (auto result = awaitCompletion(loadingFuture); result) {
            visualisations = std::move(*result);
            loadGLItems();
            currentData = visualisations.begin();
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
    for (auto &item : visualisations) {
        for (auto &item2 : item) {
            item2.first->glLoad();
            if (item2.second) {
                item2.second->glLoad();
            }
        }
    }
}

bool RenderingState::isLoading() const
{
    return loadingFuture.valid();
}
