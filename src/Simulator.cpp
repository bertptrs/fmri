#include <cassert>
#include <iostream>
#include <vector>

#include "Simulator.hpp"
#include <caffe/caffe.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace caffe;
using namespace std;
using namespace fmri;

Simulator::Simulator(const string& model_file, const string& weights_file, const string& means_file) :
	net(new Net<DType>(model_file, TEST))
{
	net->CopyTrainedLayersFrom(weights_file);

	Blob<DType>* input_layer = net->input_blobs()[0];
	input_geometry = cv::Size(input_layer->width(), input_layer->height());
	num_channels = input_layer->channels();

	input_layer->Reshape(1, num_channels,
			input_geometry.height, input_geometry.width);
	/* Forward dimension change to all layers. */
	net->Reshape();

    if (means_file != "")  {
        means = processMeans(means_file);
    }

}

vector<LayerData> Simulator::simulate(const string& image_file)
{
	typedef LayerData::Type LType;

	cv::Mat im = cv::imread(image_file, -1);

    assert(!im.empty());

    auto input = preprocess(im);
    auto channels = getWrappedInputLayer();

    cv::split(input, channels);

    net->Forward();

	vector<LayerData> result;

    Blob<DType>* input_layer = net->input_blobs()[0];

	const auto& names = net->layer_names();
	const auto& results = net->top_vecs();
	const auto& layers = net->layers();

	for (unsigned int i = 0; i < names.size(); ++i) {
		CHECK_EQ(results[i].size(), 1) << "Multiple outputs per layer are not supported!" << endl;
		const auto blob = results[i][0];

		result.emplace_back(names[i], blob->shape(), blob->cpu_data(), LayerData::typeFromString(layers[i]->type()));
	}

    return result;
}

vector<cv::Mat> Simulator::getWrappedInputLayer()
{
    vector<cv::Mat> channels;
    Blob<DType>* input_layer = net->input_blobs()[0];

    const int width = input_geometry.width;
    const int height = input_geometry.height;

    DType* input_data = input_layer->mutable_cpu_data();
    for (unsigned int i = 0; i < num_channels; i++) {
        channels.emplace_back(height, width, CV_32FC1, input_data);
        input_data += width * height;
    }

    return channels;
}

static cv::Mat fix_channels(const int num_channels, cv::Mat original) {
    cv::Mat converted;

    if (num_channels == 1 && original.channels() == 3) {
        cv::cvtColor(original, converted, cv::COLOR_BGR2GRAY);
    } else if (num_channels == 1 && original.channels() == 4) {
        cv::cvtColor(original, converted, cv::COLOR_BGRA2GRAY);
    } else if (num_channels == 3 && original.channels() == 1) {
        cv::cvtColor(original, converted, cv::COLOR_GRAY2BGR);
    } else if (num_channels == 3 && original.channels() == 4) {
        cv::cvtColor(original, converted, cv::COLOR_BGRA2BGR);
    } else {
		CHECK(num_channels == original.channels()) << "Cannot convert between channel types. ";
        return original;
    }

    return converted;
}

static cv::Mat resize(const cv::Size& targetSize, cv::Mat original)
{
    if (targetSize != original.size()) {
        cv::Mat resized;
        cv::resize(original, resized, targetSize);

        return resized;
    }

    return original;
}

cv::Mat Simulator::preprocess(cv::Mat original) const
{
    auto converted = fix_channels(num_channels, original);

    auto resized = resize(input_geometry, converted);

    cv::Mat sample_float;
    resized.convertTo(sample_float, num_channels == 3 ? CV_32FC3 : CV_32FC1);

    if (means.empty()) {
        return sample_float;
    }

    cv::Mat normalized;
    cv::subtract(sample_float, means, normalized);

    return normalized;

}

cv::Mat Simulator::processMeans(const string &means_file) const
{
    BlobProto proto;
    ReadProtoFromBinaryFileOrDie(means_file, &proto);

    Blob<DType> mean_blob;
    mean_blob.FromProto(proto);

    assert(mean_blob.channels() == num_channels);

    vector<cv::Mat> channels;
    float* data = mean_blob.mutable_cpu_data();
    for (unsigned int i = 0; i < num_channels; ++i) {
        channels.emplace_back(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
        data += mean_blob.height() * mean_blob.width();
    }

    cv::Mat mean;
    cv::merge(channels, mean);

    return cv::Mat(input_geometry, mean.type(), cv::mean(mean));
}

Simulator::~Simulator()
{
    // Empty but defined constructor.
}
