#pragma once

#include <string>
#include <memory>
#include <vector>

#include <opencv2/core/types.hpp>
#include <opencv2/core/mat.hpp>

#include "utils.hpp"
#include "LayerData.hpp"

namespace caffe
{
    template<class DType>
    class Net;
}

namespace fmri {
    using std::string;
    using std::vector;

    class Simulator {
    public:
        Simulator(const string &model_file, const string &weights_file, const string &means_file = "");
        ~Simulator();

        vector<LayerData> simulate(const string &input_file);

    private:
        std::unique_ptr<caffe::Net<DType>> net;
        cv::Size input_geometry;
        cv::Mat means;
        unsigned int num_channels;

        vector<cv::Mat> getWrappedInputLayer();
        cv::Mat preprocess(cv::Mat original) const;
        cv::Mat processMeans(const string &means_file) const;
    };
}
