#include <algorithm>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <glog/logging.h>
#include "Options.hpp"

using namespace fmri;

[[noreturn]] static void show_help(const char *progname, int exitcode)
{
    std::cerr << "Usage: " << progname << " -m MODEL -w WEIGHTS INPUTS...\n\n"
              << R"END(Simulate the specified network on the specified inputs.

Options:
	-h	show this message
	-n	(required) the model file to simulate
	-w	(required) the trained weights
    -m  means file. Will be substracted from input if available.
    -l  labels file. Will be used to print prediction labels if available.
    -d  Image dump dir. Will be filled with PNG images of intermediate results.
    -p  Image path color in hex format (#RRGGBB or #RRGGBBAA))END" << std::endl;

    std::exit(exitcode);
}

static void check_file(const char *filename)
{
    if (access(filename, R_OK) != 0) {
        perror(filename);
        exit(1);
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
static bool parse_color(const char *input, std::array<float, 4> &targetColor)
{
    if (input[0] == '#') {
        // Attempt to parse #RRGGBBAA
        std::array<unsigned int, 4> colorBuf;
        const int result = std::sscanf(input, "#%02x%02x%02x%02x", &colorBuf[0], &colorBuf[1], &colorBuf[2],
                                       &colorBuf[3]);
        CHECK_GE(result, 3) << "Invalid color HEX format, need at least 3 hex pairs, got " << result << "\n";

        std::transform(colorBuf.begin(), colorBuf.end(), targetColor.begin(), [](auto x) { return x / 255.f; });

        // Optionally, patch the alpha channel if not specified
        if (result == 3) targetColor[3] = 1;
        return true;
    }

    std::cerr << "Unknown color format used (" << input << ")\n";

    return false;
}

Options Options::parse(const int argc, char *const argv[])
{
    Options options;

    char c;

    while ((c = getopt(argc, argv, "hm:w:n:l:d:p:")) != -1) {
        switch (c) {
            case 'h':
                show_help(argv[0], 0);

            case 'w':
                check_file(optarg);
                options.weightsPath = optarg;
                break;

            case 'n':
                check_file(optarg);
                options.modelPath = optarg;
                break;

            case 'm':
                check_file(optarg);
                options.meansPath = optarg;
                break;

            case 'l':
                check_file(optarg);
                options.labelsPath = optarg;
                break;

            case 'd':
                options.dumpPath = optarg;
                break;

            case 'p':
                if (!parse_color(optarg, options.pathColor_)) {
                    show_help(argv[0], 1);
                }
                break;

            case '?':
                show_help(argv[0], 1);

            default:
                std::cerr << "Unhandled option: " << c << std::endl;
                abort();
        }
    }

    if (options.weightsPath.empty()) {
        std::cerr << "Weights file is required!\n";
        show_help(argv[0], 1);
    }

    if (options.modelPath.empty()) {
        std::cerr << "Model file is required!\n";
        show_help(argv[0], 1);
    }

    std::for_each(argv + optind, argv + argc, check_file);
    options.inputPaths.insert(options.inputPaths.end(), argv + optind, argv + argc);
    if (options.inputPaths.empty()) {
        std::cerr << "No inputs specified\n";
        show_help(argv[0], 1);
    }

    return options;
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
    if (!labelsPath) {
        return std::nullopt;
    } else {
        return read_vector<std::string>(labelsPath);
    }
}

std::optional<PNGDumper> Options::imageDumper() const
{
    if (!dumpPath) {
        return std::nullopt;
    } else {
        return PNGDumper(dumpPath);
    }
}

Options::Options() noexcept :
        pathColor_({1, 1, 1, 0.1}),
        labelsPath(nullptr),
        dumpPath(nullptr)
{
}
