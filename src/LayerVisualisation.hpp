#pragma once

namespace fmri
{
    class LayerVisualisation
    {
    public:
        virtual ~LayerVisualisation() = default;

        virtual void render() = 0;
    };
}
