#include "Drawable.hpp"

void fmri::Drawable::patchTransparancy()
{
    if constexpr (std::tuple_size<Color>::value < 4) {
        // Not compiling with alpha support
        return;
    }

    const auto alpha = getAlpha();
    const auto end = colorBuffer.end();

    for (auto it = colorBuffer.begin(); it != end; ++it) {
        (*it)[3] = alpha;
    }
}
