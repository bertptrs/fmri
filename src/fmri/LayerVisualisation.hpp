#pragma once

#include <vector>
#include "utils.hpp"
#include "Drawable.hpp"
#include "LayerInfo.hpp"

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
        void drawLayerName() const;
        void setupLayerName(std::string_view name, LayerInfo::Type type);

    protected:
        std::vector<float> nodePositions_;
        std::string displayName;

        template<Ordering Order>
        void initNodePositions(size_t n, float spacing);
        float getAlpha() override;
    };
}
