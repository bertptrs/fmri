#include "Animation.hpp"
#include "RenderingState.hpp"

float fmri::Animation::getAlpha()
{
    return fmri::RenderingState::instance().interactionAlpha();
}
