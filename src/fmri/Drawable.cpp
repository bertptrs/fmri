#include "Drawable.hpp"
#include "RenderingState.hpp"

void fmri::Drawable::patchTransparency()
{
    if constexpr (!alphaEnabled()) {
        // Not compiling with alpha support
        return;
    }

    const auto alpha = getAlpha();
    const auto end = colorBuffer.end();

    for (auto it = colorBuffer.begin(); it != end; ++it) {
        (*it)[3] = alpha;
    }
}

void fmri::Drawable::glLoad()
{
    // Do nothing
}

void fmri::Drawable::handleBrainMode(std::vector<float> &vertices)
{
    if (!fmri::RenderingState::instance().brainMode()) {
        return;
    }

    constexpr auto BRAIN_SIZE = 15;

    std::array<float, 3> maxVals = {0, 0, 0};

    const auto limit = vertices.size();

    for (auto i = 0u; i < limit; ++i) {
        maxVals[i % 3] = std::max(maxVals[i % 3], std::abs(vertices[i]));
    }

    std::array<float, 3> scaling = {
            1,
            BRAIN_SIZE / maxVals[1],
            BRAIN_SIZE / maxVals[2],
    };

    for (auto i = 0u; i < limit; ++i) {
        vertices[i] *= scaling[i % 3];
    }
}
