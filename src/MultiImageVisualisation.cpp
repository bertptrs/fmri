#include <glog/logging.h>
#include "MultiImageVisualisation.hpp"
#include "glutils.hpp"

using namespace fmri;
using namespace std;

MultiImageVisualisation::MultiImageVisualisation(const fmri::LayerData &layer)
{
    auto dimensions = layer.shape();
    CHECK_EQ(4, dimensions.size()) << "Should be image-like layer";

    const auto images = dimensions[0],
            channels = dimensions[1],
            width = dimensions[2],
            height = dimensions[3];

    auto dataPtr = layer.data();
    for (auto i = 0; i < images; ++i) {
        for (auto j = 0; j < channels; ++j) {
            textureReferences[make_pair(i, j)] = loadTexture(dataPtr, width, height);
            dataPtr += width * height;
        }
    }
}

MultiImageVisualisation::~MultiImageVisualisation()
{
    for (auto entry : textureReferences) {
        glDeleteTextures(0, &entry.second);
    }
}

void MultiImageVisualisation::render()
{
    // TODO: do something.
}
