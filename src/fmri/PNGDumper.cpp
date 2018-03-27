#include <cstring>

#include <glog/logging.h>
#include <sys/stat.h>
#include <png++/png.hpp>

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

    // Buffer for storing the current image data.
    vector<DType> buffer(imagePixels);

    auto data = layer.data();

    png::image<png::gray_pixel> image(width, height);

    for (int i = 0; i < images; ++i) {
        for (int j = 0; j < channels; ++j) {
            memcpy(buffer.data(), data, imagePixels * sizeof(DType));

            // advance the buffer;
            data += imagePixels;

            rescale(buffer.begin(), buffer.end(), 0.0, 255.0);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    image[y][x] = png::gray_pixel((int) buffer[x + y * width]);
                }
            }

            image.write(getFilename(layer.name(), i, j));
        }
    }
}

string PNGDumper::getFilename(const string &layerName, int i, int j)
{
    stringstream nameBuilder;

    nameBuilder << baseDir_
                << "/" << layerName
                << "-" << i
                << "-" << j << ".png";

    return nameBuilder.str();
}