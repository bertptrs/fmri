#include <iostream>
#include <string>
#include "Simulator.hpp"

using namespace caffe;
using namespace std;
using namespace fmri;

Simulator::Simulator(const string& model_file, const string& weights_file) :
	net(model_file, TEST)
{
	net.CopyTrainedLayersFrom(weights_file);
}

void Simulator::simulate(const string& image_file)
{
	cerr << "This is not implemented yet." << endl;
}
