#pragma once

#include <vector>
#include "utils.hpp"

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

    private:
        static float getAlpha();

    protected:
        std::vector<float> nodePositions_;

        template<Ordering Order>
        void initNodePositions(size_t n, float spacing);

        template<typename It>
        void patchTransparancy(It begin, It end)
        {
            if constexpr (std::tuple_size<Color>::value >= 4) {
                const auto alpha = getAlpha();
                for (; begin != end; ++begin) {
                    Color &color = *begin;
                    color[3] = alpha;
                }
            }
        }
    };
}
