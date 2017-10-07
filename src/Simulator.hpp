#pragma once

#include <string>
#include <memory>

#include <caffe/caffe.hpp>

namespace fmri {
    using std::string;

    class Simulator {
    public:
        typedef float DType;

        Simulator(const string &model_file, const string &weights_file);

        void simulate(const string &input_file);

    private:
        caffe::Net<DType> net;
    };
}
