#pragma once

#include "LayerData.hpp"
#include "Animation.hpp"

namespace fmri
{
    class LabelVisualisation : public Animation
    {
    public:
        LabelVisualisation(const std::vector<float>& positions, const LayerData& prevData, const std::vector<std::string>& labels);
        void draw(float time) override;

    private:
        static constexpr float DISPLAY_LIMIT = 0.01;

        std::vector<std::string> nodeLabels;
        std::vector<float> nodePositions_;
    };
}
