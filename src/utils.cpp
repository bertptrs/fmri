#include "utils.hpp"

std::default_random_engine &fmri::rng()
{
    static std::default_random_engine rng;
    static std::default_random_engine::result_type seed = 0;

    if (seed == 0) {
        std::random_device dev;
        rng.seed(seed = dev());
    }

    return rng;
}
