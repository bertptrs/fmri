#include <algorithm>
#include <iostream>
#include <boost/program_options.hpp>
#include <glog/logging.h>
#include "Options.hpp"

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

Options Options::parse(const int argc, char *const argv[])
{
    using namespace boost::program_options;

    try {
        Options options;

        options_description desc("Options");
        positional_options_description positionals;
        positionals.add("input", -1);

        options_description hidden;
        hidden.add_options()
                ("input", value<std::vector<std::string>>(&options.inputPaths)->required()->composing());

        desc.add_options()
                ("help,h", "Show this help message")
                ("weights,w", value<std::string>(&options.weightsPath)->required(), "weights file for the network")
                ("network,n", value<std::string>(&options.modelPath)->required(), "caffe model file for the network")
                ("labels,l", value<std::string>(&options.labelsPath), "labels file")
                ("means,m", value<std::string>(&options.meansPath), "means file")
                ("path-color,p", value<std::string>(), "color for paths")
                ("dump,d", value<std::string>(&options.dumpPath), "dump convolutional images in this directory");

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
            parse_color(vm["path-color"].as<std::string>().c_str(), options.pathColor_);
        }

        // Sanity checks
        check_file(options.modelPath);
        check_file(options.weightsPath);
        check_file(options.meansPath);
        std::for_each(options.inputPaths.begin(), options.inputPaths.end(), check_file);

        return options;
    } catch (required_option& e) {
        std::cerr << e.get_option_name() << std::endl;
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

Options::Options() noexcept :
        pathColor_({1, 1, 1, 0.1})
{
}

const Color &Options::pathColor() const
{
    return pathColor_;
}
