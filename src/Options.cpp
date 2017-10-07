#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include "Options.hpp"

using namespace fmri;
using namespace std;

static void show_help(const char* progname, int exitcode)
{
	cerr << "Usage: " << progname << " -m MODEL -w WEIGHTS INPUTS..." << endl
		<< endl
		<< R"END(
Simulate the specified network on the specified inputs.

Options:
	-h	show this message
	-m	(required) the model file to simulate
	-w	(required) the trained weights
)END" << endl;

	exit(exitcode);
}

static void check_file(const char * filename)
{
	if (access(filename, R_OK) != 0) {
		perror(filename);
		exit(1);
	}
}

Options Options::parse(const int argc, char * const argv[])
{
	string model;
	string weights;

	char c;

	while ((c = getopt(argc, argv, "hm:w:")) != -1) {
		switch (c) {
			case 'h':
				show_help(argv[0], 0);
				break;

			case 'w':
				check_file(optarg);
				weights = optarg;
				break;

			case 'm':
				check_file(optarg);
				model = optarg;
				break;

			case '?':
				cerr << "Unknown option character: " << char(optopt) << endl;
				show_help(argv[0], 1);
				break;

			default:
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

	return Options(move(model), move(weights), move(inputs));
}

Options::Options(string&& model, string&& weights, vector<string>&& inputs) noexcept:
	modelPath(move(model)),
	weightsPath(move(weights)),
	inputPaths(move(inputs))
{
}

const string& Options::model() const
{
	return modelPath;
}

const string& Options::weights() const
{
	return weightsPath;
}

const vector<string>& Options::inputs() const
{
	return inputPaths;
}
