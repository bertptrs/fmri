#pragma once

#include <string>

namespace fmri
{
    struct Camera {
        float pos[3];
        float angle[2];

        void reset();
        void configureRenderingContext();
        void registerControls();
        std::string infoLine();

        static Camera& instance();

    private:
        Camera() noexcept = default;
    };
}
