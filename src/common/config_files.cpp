#include <climits>
#include <cstring>
#include "config_files.hpp"

const char* fmri::BRAIN_CONFIG_FILE = "brain.ini";
const char* fmri::MAIN_CONFIG_FILE = "main.ini";

std::ifstream fmri::get_xdg_config(const char *filename) noexcept
{
    std::ifstream configFile(filename);
    if (configFile) {
        return configFile;
    }


    // Determine the XDG_CONFIG_HOME
    char configBuf[PATH_MAX];
    if (char* configHome = std::getenv("XDG_CONFIG_HOME"); configHome != nullptr) {
        std::strncpy(configBuf, configHome, sizeof(configBuf) - 1);
    } else {
        std::snprintf(configBuf, sizeof(configBuf), "%s/.config", getenv("HOME"));
    }

    // Work around an unfair warning. If this exceeds my array limit, I have other problems.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
    char fileBuf[PATH_MAX];
    std::snprintf(fileBuf, sizeof(fileBuf), "%s/fmri/%s", configBuf, filename);
#pragma GCC diagnostic pop

    configFile.open(fileBuf);
    return configFile;
}
