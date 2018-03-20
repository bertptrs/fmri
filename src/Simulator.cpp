#include <cassert>
#include <iostream>
#include <optional>
#include <vector>

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "Simulator.hpp"
#include "Range.hpp"

using namespace caffe;
using namespace std;
using namespace fmri;

struct Simulator::Impl
{
    caffe::Net<DType> net;
    cv::Size input_geometry;
    optional<cv::Mat> means;
    unsigned int num_channels;
    map<string, LayerInfo> layerInfo_;

    Impl(const string& model_file, const string& weights_file, const string& means_file);

    vector<cv::Mat> getWrappedInputLayer();
    cv::Mat preprocess(cv::Mat original) const;
    vector<LayerData> simulate(const string &input_file);
    const map<string, LayerInfo>& layerInfo() const;

    void computeLayerInfo();

    void loadMeans(const string &means_file);

    void ensureNoInPlaceLayers();
};

// Create simple forwarding functions.
Simulator::Simulator(const string& model_file, const string& weights_file, const string& means_file) :
	pImpl(new Impl(model_file, weights_file, means_file))
{
}

vector<LayerData> Simulator::simulate(const string& image_file)
{
    return pImpl->simulate(image_file);
}

Simulator::Impl::Impl(const string& model_file, const string& weights_file, const string& means_file) :
	net(model_file, TEST)
{
	net.CopyTrainedLayersFrom(weights_file);
    ensureNoInPlaceLayers();

	auto input_layer = net.input_blobs()[0];
	input_geometry = cv::Size(input_layer->width(), input_layer->height());
	num_channels = input_layer->channels();

	input_layer->Reshape(1, num_channels,
			input_geometry.height, input_geometry.width);
	/* Forward dimension change to all layers. */
	net.Reshape();

    if (!means_file.empty()) {
        loadMeans(means_file);
    }

    computeLayerInfo();
}

void Simulator::Impl::loadMeans(const string &means_file)
{// Read in the means file
    BlobProto proto;
    ReadProtoFromBinaryFileOrDie(means_file, &proto);

    Blob<DType> mean_blob;
    mean_blob.FromProto(proto);

    CHECK_EQ(mean_blob.channels(), num_channels) << "Number of channels should match!" << endl;

    vector<cv::Mat> channels;
    float *data = mean_blob.mutable_cpu_data();
    for (auto i : Range(num_channels)) {
        (void)i;// Suppress unused warning
        channels.emplace_back(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
        data += mean_blob.height() * mean_blob.width();
    }

    cv::Mat mean;
    merge(channels, mean);

    this->means = cv::Mat(input_geometry, mean.type(), cv::mean(mean));
}

vector<LayerData> Simulator::Impl::simulate(const string& image_file)
{
	cv::Mat im = cv::imread(image_file, -1);

    assert(!im.empty());

    auto input = preprocess(im);
    auto channels = getWrappedInputLayer();

    cv::split(input, channels);

    net.Forward();

	vector<LayerData> result;

	const auto& names = net.layer_names();
	const auto& results = net.top_vecs();

	for (auto i : Range(names.size())) {
		CHECK_EQ(results[i].size(), 1) << "Multiple outputs per layer are not supported!" << endl;
		const auto blob = results[i][0];

		result.emplace_back(names[i], blob->shape(), blob->cpu_data());
	}

    return result;
}

vector<cv::Mat> Simulator::Impl::getWrappedInputLayer()
{
    vector<cv::Mat> channels;
    auto input_layer = net.input_blobs()[0];

    const int width = input_geometry.width;
    const int height = input_geometry.height;

    DType* input_data = input_layer->mutable_cpu_data();
    for (auto i : Range(num_channels)) {
        (void)i;// Suppress unused warning
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

cv::Mat Simulator::Impl::preprocess(cv::Mat original) const
{
    auto converted = fix_channels(num_channels, std::move(original));

    auto resized = resize(input_geometry, converted);

    cv::Mat sample_float;
    resized.convertTo(sample_float, num_channels == 3 ? CV_32FC3 : CV_32FC1);

    if (!means) {
        return sample_float;
    }

    cv::Mat normalized;
    cv::subtract(sample_float, *means, normalized);

    return normalized;

}

const map<string, LayerInfo> &Simulator::Impl::layerInfo() const
{
    return layerInfo_;
}

void Simulator::Impl::computeLayerInfo()
{
    const auto& names = net.layer_names();
    const auto& layers = net.layers();

    CHECK_EQ(names.size(), layers.size()) << "Size mismatch";

    for (auto i : Range(names.size())) {
        auto& layer = layers[i];
        LayerInfo layerInfo(names[i], layer->type(), layer->blobs());
        CHECK_NE(layerInfo.type(), LayerInfo::Type::Split) << "Split layers are not supported!";
        layerInfo_.emplace(names[i], std::move(layerInfo));
    }
}

void Simulator::Impl::ensureNoInPlaceLayers()
{
    auto blobList = net.top_vecs();
    typeof(blobList) uniqueVecs;
    unique_copy(blobList.begin(), blobList.end(), back_inserter(uniqueVecs));

    LOG_IF(ERROR, blobList.size() != uniqueVecs.size())
        << "Network file contains in-place layers, layer-state will not be accurate\n"
        << "If accurate results are desired, see the deinplace script in tools." << endl;
}

Simulator::~Simulator()
{
    // Empty but defined constructor.
}

const map<string, LayerInfo> & Simulator::layerInfo() const
{
    return pImpl->layerInfo();
}
