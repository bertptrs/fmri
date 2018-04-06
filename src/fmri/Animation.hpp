#pragma once


#include "utils.hpp"
#include "Drawable.hpp"

namespace fmri
{
    class Animation : public Drawable
    {
    public:
        virtual ~Animation() = default;

    protected:
        float getAlpha() override;
    };
}
