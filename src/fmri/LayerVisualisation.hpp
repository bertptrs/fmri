#pragma once

#include <vector>
#include "utils.hpp"
#include "Drawable.hpp"

namespace fmri
{
    class LayerVisualisation
            : public Drawable
    {
    public:
        enum class Ordering {
            LINE,
            SQUARE,
        };

        LayerVisualisation() = default;
        explicit LayerVisualisation(size_t numNodes);
        virtual ~LayerVisualisation() = default;

        virtual const std::vector<float>& nodePositions() const;

    protected:
        float getAlpha() override;

    protected:
        std::vector<float> nodePositions_;

        template<Ordering Order>
        void initNodePositions(size_t n, float spacing);
    };
}
