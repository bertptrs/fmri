#include <algorithm>
#include <iostream>
#include <boost/program_options.hpp>
#include <fstream>
#include <GL/gl.h>
#include "Options.hpp"
#include "visualisations.hpp"
#include "../common/config_files.hpp"

using namespace fmri;

static void check_file(const std::string& filename)
{
    if (access(filename.c_str(), R_OK) != 0) {
        char errorBuf[1024];
        std::snprintf(errorBuf, sizeof(errorBuf), "%s: %s", filename.c_str(), std::strerror(errno));

        throw std::invalid_argument(errorBuf);
    }
}

/**
 * Create a boost::program_options::value for some variable.
 *
 * Utility method to make generating value wrappers less repetitive. This
 * method automatically sets the default value to whatever the variable
 * currently holds.
 *
 * @tparam T
 * @param val Variable to be set/use as default.
 * @return A value wrapper
 */
template<typename T>
inline static auto value_for(T& val)
{
    return boost::program_options::value<T>(&val)
            ->default_value(val);
}

/**
 * Parse a color string into a color array.
 *
 * This function may terminate the program on a partial match.
 *
 * @param input
 * @param targetColor
 * @return true if the read was successful.
 */
static void parse_color(const char *input, Color &targetColor)
{
    if (isxdigit(input[0])) {
        // Attempt to parse #RRGGBBAA
        std::array<unsigned int, 4> colorBuf;
        const int result = std::sscanf(input, "%02x%02x%02x%02x", &colorBuf[0], &colorBuf[1], &colorBuf[2],
                                       &colorBuf[3]);
        if (result < 3) {
            throw std::invalid_argument("Invalid color HEX format, need at least 3 hex pairs");
        }

        std::transform(colorBuf.begin(), colorBuf.end(), targetColor.begin(), [](auto x) { return x / 255.f; });

        // Optionally, patch the alpha channel if not specified
        if (result == 3 && alphaEnabled()) targetColor[3] = 1;
        return;
    } else if (input[0] == '#') {
        parse_color(input + 1, targetColor);
        return;
    }

    char errorBuf[1024];
    std::snprintf(errorBuf, sizeof(errorBuf), "Unknown color value: %s", input);
    throw std::invalid_argument(errorBuf);
}

static inline void parse_color(const std::string& input, Color& targetColor)
{
    parse_color(input.c_str(), targetColor);
}

static void use_color(const boost::program_options::variables_map& vm, const char* key, Color& target) {
    if (vm.count(key)) {
        parse_color(vm[key].as<std::string>(), target);
    }
}

Options::Options(int argc, char * const argv[]):
        layerTransparency_(1),
        interactionTransparency_(1),
        pathColor_({1, 1, 1, 0.1}),
        brainMode_(false),
        inputMillis_(1000)
{
    using namespace boost::program_options;

    try {
        const bool brainMode = std::any_of(argv, argv + argc, [](auto x) {return !std::strcmp("-b", x) || !std::strcmp("--brainmode", x);});
        if (brainMode) {
            // Alternative defaults for brain mode,
            // As suggested by Michael Lew
            NEUTRAL_COLOR = {0.5, 0.5, 0.5, 1};
            NEGATIVE_COLOR = {0, 0, 0, 1};
            POSITIVE_COLOR = {1, 1, 1, 1};
            LAYER_X_OFFSET = 0.8;
            interactionTransparency_ = 1;
            layerTransparency_ = 0.2;
        }
        bool show_help = false;
        options_description desc;
        options_description cli("Options");
        positional_options_description positionals;
        positionals.add("input", -1);

        options_description hidden;
        hidden.add_options()
                ("input", value<std::vector<std::string>>(&inputPaths)->required()->composing());

        cli.add_options()
                ("brain-mode,b", bool_switch(&brainMode_), "Enable brain mode")
                ("help,h", bool_switch(&show_help), "Show this help message");

        desc.add_options()
                ("weights,w", value<std::string>(&weightsPath)->required(), "weights file for the network")
                ("network,n", value<std::string>(&modelPath)->required(), "caffe model file for the network")
                ("labels,l", value<std::string>(&labelsPath), "labels file")
                ("means,m", value<std::string>(&meansPath), "means file")
                ("path-color,p", value<std::string>()->default_value("#ffffff19"), "color for paths")
                ("layer-opacity", value_for(layerTransparency_), "Opacity for layers")
                ("interaction-opacity", value_for(interactionTransparency_), "Opacity for interactions")
                ("layer-distance", value_for(LAYER_X_OFFSET), "Distance between layers")
                ("interaction-limit", value_for(INTERACTION_LIMIT), "Maximum number of interactions per layer")
                ("neutral-color", value<std::string>(), "Color for showing neutral states")
                ("positive-color", value<std::string>(), "Color for showing positive states")
                ("negative-color", value<std::string>(), "Color for showing negative states")
                ("background-color", value<std::string>()->default_value("#00000000"), "Color for showing neutral states")
                ("input-millis", value_for(inputMillis_), "Milliseconds for which an input is shown in movie mode")
                ("dump,d", value<std::string>(&dumpPath), "dump convolutional images in this directory");

        cli.add(desc);
        options_description composed = cli;
        composed.add(hidden);

        variables_map vm;

        // Boost handles priority as: first defined wins. So, first CLI, then brain mode, then config.
        store(command_line_parser(argc, argv).options(composed).positional(positionals).run(), vm);
        if (brainMode) {
            if (auto config = get_xdg_config(BRAIN_CONFIG_FILE); config.good()) {
                store(parse_config_file(config, desc, true), vm);
            }
        }
        if (auto config = get_xdg_config(MAIN_CONFIG_FILE); config.good()) {
            store(parse_config_file(config, desc, true), vm);
        }

        if (show_help) {
            std::cout << "Usage: " << argv[0] << " [OPTIONS] [INPUTS]\n\n" << cli << '\n';
            std::exit(0);
        }

        notify(vm);

        use_color(vm, "path-color", pathColor_);
        use_color(vm, "neutral-color", NEUTRAL_COLOR);
        use_color(vm, "positive-color", POSITIVE_COLOR);
        use_color(vm, "negative-color", NEGATIVE_COLOR);

        if (vm.count("background-color")) {
            Color bg;
            parse_color(vm["background-color"].as<std::string>(), bg);
            glClearColor(bg[0], bg[1], bg[2], bg[3]);
        }

        // Sanity checks
        check_file(modelPath);
        check_file(weightsPath);
        if (!meansPath.empty()) check_file(meansPath);
        if (!labelsPath.empty()) check_file(labelsPath);
        std::for_each(inputPaths.begin(), inputPaths.end(), check_file);
        return;
    } catch (required_option& e) {
        if (e.get_option_name() == "--input") {
            std::cerr << "No input files specified" << std::endl;
        } else {
            std::cerr << e.what() << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    std::exit(1);
}

const string &Options::model() const
{
    return modelPath;
}

const string &Options::weights() const
{
    return weightsPath;
}

const vector<string> &Options::inputs() const
{
    return inputPaths;
}

const string &Options::means() const
{
    return meansPath;
}

std::optional<vector<string>> Options::labels() const
{
    if (labelsPath.empty()) {
        return std::nullopt;
    } else {
        return read_vector<std::string>(labelsPath);
    }
}

std::optional<PNGDumper> Options::imageDumper() const
{
    if (dumpPath.empty()) {
        return std::nullopt;
    } else {
        return PNGDumper(dumpPath);
    }
}

const Color &Options::pathColor() const
{
    return pathColor_;
}

float Options::layerTransparency() const
{
    return layerTransparency_;
}

float Options::interactionTransparency() const
{
    return interactionTransparency_;
}

bool Options::brainMode() const
{
    return brainMode_;
}

int Options::inputMillis() const
{
    return inputMillis_;
}
