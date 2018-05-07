#include <caffe/util/math_functions.hpp>
#include <GL/glu.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include "InputLayerVisualisation.hpp"
#include "Range.hpp"
#include "glutils.hpp"

using namespace fmri;
using namespace std;

/**
 * Combine an arbitrary number of channels into an RGB image.
 *
 * If there are less than 3 channels, the first channel is repeated. This
 * results in greyscale images for single-channel images. Any channels after
 * the third are ignored.
 *
 * @param data Layer data to generate an image for.
 * @return A normalized RGB image, stored in a vector.
 */
static unique_ptr<float[]> getRGBImage(const LayerData &data)
{
    CHECK_EQ(data.shape().size(), 4) << "Should be image-like-layer.";
    CHECK_EQ(data.shape()[0], 1) << "Should be a single image";
    vector<cv::Mat> channels;
    const int numPixels = data.shape()[2] * data.shape()[3];
    for (auto i : Range(3)) {
        if (i >= data.shape()[1]) {
            channels.push_back(channels[0]);
        }

        cv::Mat channel(data.shape()[3], data.shape()[2], CV_32FC1);
        copy_n(data.data() + i * numPixels, numPixels, channel.begin<float>());
        channels.push_back(channel);
    }

    swap(channels[0], channels[2]);

    cv::Mat outImage;
    cv::merge(channels, outImage);

    outImage = outImage.reshape(1);

    auto final = make_unique<float[]>(data.numEntries());
    std::copy_n(outImage.begin<float>(), data.numEntries(), final.get());

    return final;
}

InputLayerVisualisation::InputLayerVisualisation(const LayerData &data) :
        width(data.shape().at(2)),
        height(data.shape().at(3)),
        texture(getRGBImage(data), width, height, GL_RGB)
{
    const auto channels = data.shape()[1];

    if (brainModeEnabled()) {
        targetWidth = targetHeight = BRAIN_SIZE;
    } else {
        targetWidth = width / 5.f;
        targetHeight = height / 5.f;
    }

    nodePositions_ = {0, targetHeight / 2, targetWidth / -2};
    for (auto i : Range(3, 3 * channels)) {
        nodePositions_.push_back(nodePositions_[i % 3]);
    }
}

void InputLayerVisualisation::draw(float)
{
    const float vertices[] = {
            0, 0, 0,
            0, 0, -targetWidth,
            0, targetHeight, -targetWidth,
            0, targetHeight, 0,
    };

    const float texCoords[] = {
            0, 1,
            1, 1,
            1, 0,
            0, 0,
    };

    float alpha = getAlpha();

    drawImageTiles(4, vertices, texCoords, texture, alpha);
}

void InputLayerVisualisation::glLoad()
{
    Drawable::glLoad();

    texture.configure(GL_TEXTURE_2D);
}
