#include <GL/gl.h>
#include "LabelVisualisation.hpp"
#include "glutils.hpp"
#include "RenderingState.hpp"

using namespace fmri;

void LabelVisualisation::draw(float)
{
    if (RenderingState::instance().renderInteractionPaths()) {
        setGlColor(RenderingState::instance().pathColor());
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, nodePositions_.data());
        glDrawElements(GL_LINES, nodeIndices.size(), GL_UNSIGNED_INT, nodeIndices.data());
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    glPushMatrix();
    glTranslatef(LAYER_X_OFFSET, 0, 0);

    for (auto i = 0u; i < nodeLabels.size(); ++i) {
        glPushMatrix();
        glTranslatef(nodePositions_[3 * i], nodePositions_[3 * i + 1], nodePositions_[3 * i + 2]);
        setGlColor(colorBuffer[i]);
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

    // First, determine all nodes elegible to be rendered.
    for (auto i = 0u; i < limit; ++i) {
        if (prevData[i] < DISPLAY_LIMIT) {
            continue;
        }

        colorBuffer.emplace_back(Color{1 - prevData[i] / maxVal, 1 - prevData[i] / maxVal, 1});
        std::copy_n(positions.begin() + 3 * i, 3, nodeInserter);
        nodeLabels.emplace_back(labels[i]);
    }

    // Now fix the points for the interaction paths.
    nodeIndices.reserve(2 * nodeLabels.size());
    for (auto i = 0u; i < nodeLabels.size(); ++i) {
        nodeIndices.push_back(i);
        nodeIndices.push_back(i + nodeLabels.size());
    }

    // Make sure the end positions exist.
    std::copy_n(nodePositions_.begin(), nodePositions_.size(), nodeInserter);
    for (auto i = nodePositions_.size() / 2; i < nodePositions_.size(); i += 3) {
        nodePositions_[i] = LAYER_X_OFFSET;
    }


    patchTransparancy();
}
