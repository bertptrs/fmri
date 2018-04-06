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
static vector<float> getRGBImage(const LayerData &data)
{
    vector<cv::Mat> channels;
    const int numPixels = data.shape()[2] * data.shape()[3];
    for (auto i : Range(3)) {
        if (i >= data.shape()[1]) {
            channels.push_back(channels[0]);
        }

        cv::Mat channel(data.shape()[3], data.shape()[2], CV_32FC1);
        copy(data.data() + i * numPixels, data.data() + (i + 1) * numPixels, channel.begin<float>());
        channels.push_back(channel);
    }

    swap(channels[0], channels[2]);

    cv::Mat outImage;
    cv::merge(channels, outImage);

    outImage = outImage.reshape(1);

    vector<float> final(outImage.begin<float>(), outImage.end<float>());
    rescale(final.begin(), final.end(), 0.f, 1.f);

    return final;
}

InputLayerVisualisation::InputLayerVisualisation(const LayerData &data)
{
    CHECK_EQ(data.shape().size(), 4) << "Should be image-like-layer." << endl;
    auto imageData = getRGBImage(data);

    const auto images = data.shape()[0], channels = data.shape()[1], width = data.shape()[2], height = data.shape()[3];
    CHECK_EQ(images, 1) << "Should be single image" << endl;

    targetWidth = width / 5.f;
    targetHeight = width / 5.f;

    nodePositions_ = {0, targetHeight / 2, targetWidth / -2};
    for (auto i : Range(3, 3 * channels)) {
        nodePositions_.push_back(nodePositions_[i % 3]);
    }

    texture.configure(GL_TEXTURE_2D);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_FLOAT, imageData.data());
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
