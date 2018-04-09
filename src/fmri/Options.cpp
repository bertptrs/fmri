#include <algorithm>
#include <iostream>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include "Options.hpp"
#include "visualisations.hpp"

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
    if (input[0] == '#') {
        // Attempt to parse #RRGGBBAA
        std::array<unsigned int, 4> colorBuf;
        const int result = std::sscanf(input, "#%02x%02x%02x%02x", &colorBuf[0], &colorBuf[1], &colorBuf[2],
                                       &colorBuf[3]);
        if (result < 3) {
            throw std::invalid_argument("Invalid color HEX format, need at least 3 hex pairs");
        }

        std::transform(colorBuf.begin(), colorBuf.end(), targetColor.begin(), [](auto x) { return x / 255.f; });

        // Optionally, patch the alpha channel if not specified
        if (result == 3) targetColor[3] = 1;
        return;
    }

    char errorBuf[1024];
    std::snprintf(errorBuf, sizeof(errorBuf), "Unknown color value: %s", input);
    throw std::invalid_argument(errorBuf);
}

Options::Options(int argc, char * const argv[]):
        layerTransparancy_(1),
        interactionTransparancy_(1),
        pathColor_({1, 1, 1, 0.1})
{
    using namespace boost::program_options;

    try {
        options_description desc("Options");
        positional_options_description positionals;
        positionals.add("input", -1);

        options_description hidden;
        hidden.add_options()
                ("input", value<std::vector<std::string>>(&inputPaths)->required()->composing());

        desc.add_options()
                ("help,h", "Show this help message")
                ("weights,w", value<std::string>(&weightsPath)->required(), "weights file for the network")
                ("network,n", value<std::string>(&modelPath)->required(), "caffe model file for the network")
                ("labels,l", value<std::string>(&labelsPath), "labels file")
                ("means,m", value<std::string>(&meansPath), "means file")
                ("path-color,p", value<std::string>()->default_value("#ffffff19"), "color for paths")
                ("layer-opacity", value_for(layerTransparancy_), "Opacity for layers")
                ("interaction-opacity", value_for(interactionTransparancy_), "Opacity for interactions")
                ("layer-distance", value_for(LAYER_X_OFFSET), "Distance between layers")
                ("interaction-limit", value_for(INTERACTION_LIMIT), "Maximum number of interactions per layer")
                ("dump,d", value<std::string>(&dumpPath), "dump convolutional images in this directory");

        options_description composed = desc;
        composed.add(hidden);

        variables_map vm;
        store(command_line_parser(argc, argv).options(composed).positional(positionals).run(), vm);

        if (vm.count("help")) {
            std::cout << "Usage: " << argv[0] << " [OPTIONS] [INPUTS]\n\n" << desc << '\n';
            std::exit(0);
        }

        notify(vm);

        if (vm.count("path-color")) {
            parse_color(vm["path-color"].as<std::string>().c_str(), pathColor_);
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

float Options::layerTransparancy() const
{
    return layerTransparancy_;
}

float Options::interactionTransparancy() const
{
    return interactionTransparancy_;
}
