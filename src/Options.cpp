#include <algorithm>
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "Options.hpp"

using namespace fmri;
using namespace std;

static void show_help(const char *progname, int exitcode) {
    cerr << "Usage: " << progname << " -m MODEL -w WEIGHTS INPUTS..." << endl
         << endl
         << R"END(Simulate the specified network on the specified inputs.

Options:
	-h	show this message
	-n	(required) the model file to simulate
	-w	(required) the trained weights
    -m  means file. Will be substracted from input if available.
    -l  labels file. Will be used to print prediction labels if available.)END" << endl;

    exit(exitcode);
}

static void check_file(const char *filename) {
    if (access(filename, R_OK) != 0) {
        perror(filename);
        exit(1);
    }
}

Options Options::parse(const int argc, char *const argv[]) {
    string model;
    string weights;
    string means;
    string labels;

    char c;

    while ((c = getopt(argc, argv, "hm:w:n:l:")) != -1) {
        switch (c) {
            case 'h':
                show_help(argv[0], 0);
                break;

            case 'w':
                check_file(optarg);
                weights = optarg;
                break;

            case 'n':
                check_file(optarg);
                model = optarg;
                break;

            case 'm':
                check_file(optarg);
                means = optarg;
                break;

            case 'l':
                check_file(optarg);
                labels = optarg;
                break;

            case '?':
                show_help(argv[0], 1);
                break;

            default:
                cerr << "Unhandled option: " << c << endl;
                abort();
        }
    }

    if (weights.empty()) {
        cerr << "Weights file is required!" << endl;
        show_help(argv[0], 1);
    }

    if (model.empty()) {
        cerr << "Model file is required!" << endl;
        show_help(argv[0], 1);
    }

    for_each(argv + optind, argv + argc, check_file);

    vector<string> inputs(argv + optind, argv + argc);
    if (inputs.empty()) {
        cerr << "No inputs specified" << endl;
        show_help(argv[0], 1);
    }

    return Options(move(model), move(weights), move(means), move(labels), move(inputs));
}

Options::Options(string &&model, string &&weights, string&& means, string&& labels, vector<string> &&inputs) noexcept:
        modelPath(move(model)),
        weightsPath(move(weights)),
        meansPath(means),
        labelsPath(labels),
        inputPaths(move(inputs))
{
}

const string& Options::model() const {
    return modelPath;
}

const string& Options::weights() const {
    return weightsPath;
}

const vector<string>& Options::inputs() const {
    return inputPaths;
}

const string& Options::means() const
{
    return meansPath;
}

const string& Options::labels() const
{
    return labelsPath;
}
