#pragma once

#include <fstream>

namespace fmri {

    /** Name of the main config file */
    extern const char* MAIN_CONFIG_FILE;
    /** Name of the config file loaded for brain-mode only */
    extern const char* BRAIN_CONFIG_FILE;

    /**
     * Get a config file according to the XDG_CONFIG_HOME.
     *
     * Note that these config files will always include the "fmri"
     * subdirectory for simplicity.
     *
     * @param filename
     * @return A (possibly invalid) file stream for the config file.
     */
    std::ifstream get_xdg_config(const char *filename) noexcept;
}
