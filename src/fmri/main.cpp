#include <algorithm>
#include <iostream>
#include <glog/logging.h>
#include <GL/glut.h>
#include <map>
#include <cstdarg>

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

int main(int argc, char *argv[])
{
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    registerErrorCallbacks();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);
    glutCreateWindow(argv[0]);

    // Prepare data for simulations
    Options options(argc, argv);
    RenderingState::instance().loadOptions(options);

    // Register callbacks
    glutReshapeFunc(changeWindowSize);

    RenderingState::instance().registerControls();

    // Start visualisation
    glutMainLoop();

    google::ShutdownGoogleLogging();

    return 0;
}
