#pragma once


namespace fmri
{
    class Animation
    {
    public:
        virtual ~Animation() = default;

        virtual void draw(float step) = 0;

    };
}
