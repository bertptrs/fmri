#include <cstring>

#include <glog/logging.h>
#include <sys/stat.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>

#include "PNGDumper.hpp"

using namespace fmri;
using namespace std;

static void ensureDir(const string& dir)
{
    struct stat s;
    if (stat(dir.c_str(), &s) == 0) {
        CHECK(S_ISDIR(s.st_mode)) << dir << " already exists and is not a directory." << endl;
        return;
    }

    switch (errno) {
        case ENOENT:
            PCHECK(mkdir(dir.c_str(), 0777) == 0) << "Couldn't create directory";
            return;

        default:
            perror("Unusable dump dir");
            break;
    }
}

PNGDumper::PNGDumper(string_view baseDir) :
        baseDir_(baseDir)
{
    ensureDir(baseDir_);
}

void PNGDumper::dump(const LayerData &layerData)
{
    if (layerData.shape().size() == 4) {
        // We have a series of images.
        dumpImageSeries(layerData);
    } else {
        LOG(INFO) << "Unable to dump this type of layer to png.";
    }
}

void PNGDumper::dumpImageSeries(const LayerData &layer)
{
    const auto& shape = layer.shape();
    const auto images = shape[0], channels = shape[1], height = shape[2], width = shape[3];
    const auto imagePixels = width * height;

    cv::Mat image(width, height, CV_32FC1);

    auto data = layer.data();

    for (int i = 0; i < images; ++i) {
        for (int j = 0; j < channels; ++j) {
            char pathBuf[PATH_MAX];
            std::copy_n(data, imagePixels, image.begin<float>());
            rescale(image.begin<float>(), image.end<float>(), 0, 255);
            std::snprintf(pathBuf, sizeof(pathBuf), "%s/%s-%d-%d.png", baseDir_.c_str(), layer.name().c_str(), i, j);

            cv::imwrite(pathBuf, image);

            data += imagePixels;
        }
    }
}

