#include <iostream>
#include "Options.hpp"
#include "Simulator.hpp"

using namespace std;
using namespace fmri;

int main(int argc, char * const argv[])
{
	::google::InitGoogleLogging(argv[0]);

	Options options = Options::parse(argc, argv);

	Simulator simulator(options.model(), options.weights());

	for (const auto& image : options.inputs()) {
		simulator.simulate(image);
	}

	return 0;
}
