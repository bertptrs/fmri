#pragma once

#include <string>
#include <memory>
#include <vector>

#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace fmri {
    using std::string;
    using std::vector;

    class Simulator {
    public:
        typedef float DType;

        Simulator(const string &model_file, const string &weights_file, const string &means_file = "");

        vector<DType> simulate(const string &input_file);

    private:
        caffe::Net<DType> net;
        cv::Size input_geometry;
        cv::Mat means;
        unsigned int num_channels;

        vector<cv::Mat> getWrappedInputLayer();
        cv::Mat preprocess(cv::Mat original) const;
        cv::Mat processMeans(const string &means_file) const;
    };
}
