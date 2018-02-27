#pragma once

#include <vector>

namespace fmri
{
    class LayerVisualisation
    {
    public:
        enum class Ordering {
            LINE,
            SQUARE,
        };

        LayerVisualisation() = default;
        explicit LayerVisualisation(size_t numNodes);
        virtual ~LayerVisualisation() = default;

        virtual void render() = 0;
        virtual const std::vector<float>& nodePositions() const;

    protected:
        std::vector<float> nodePositions_;

        template<Ordering Order>
        void initNodePositions(size_t n, float spacing);
    };
}
