#include <iostream>
#include "Options.hpp"

using namespace std;
using namespace fmri;

int main(int argc, char * const argv[])
{
	Options options = Options::parse(argc, argv);

	return 0;
}
