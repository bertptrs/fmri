#include <GL/gl.h>
#include "LabelVisualisation.hpp"
#include "glutils.hpp"

using namespace fmri;

void LabelVisualisation::draw(float)
{
    glPushMatrix();
    glTranslatef(LAYER_X_OFFSET, 0, 0);

    for (auto i = 0u; i < nodeLabels.size(); ++i) {
        glPushMatrix();
        glTranslatef(nodePositions_[3 * i], nodePositions_[3 * i + 1], nodePositions_[3 * i + 2]);
        if constexpr (alphaEnabled()) {
            glColor4fv(colorBuffer[i].data());
        } else {
            glColor3fv(colorBuffer[i].data());
        }
        renderText(nodeLabels[i]);
        glPopMatrix();
    }

    glPopMatrix();
}

LabelVisualisation::LabelVisualisation(const std::vector<float> &positions, const LayerData &prevData,
                                       const std::vector<std::string> &labels)
{
    const auto limit = std::min(prevData.numEntries(), labels.size());
    const auto maxVal = *std::max_element(prevData.data(), prevData.data() + prevData.numEntries());

    auto nodeInserter = std::back_inserter(nodePositions_);

    for (auto i = 0u; i < limit; ++i) {
        if (prevData[i] < DISPLAY_LIMIT) {
            continue;
        }

        colorBuffer.emplace_back(Color{1 - prevData[i] / maxVal, 1 - prevData[i] / maxVal, 1});
        std::copy_n(positions.begin() + 3 * i, 3, nodeInserter);
        nodeLabels.emplace_back(labels[i]);
    }

    patchTransparancy();
}
