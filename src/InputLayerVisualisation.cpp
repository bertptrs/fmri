#include <caffe/util/math_functions.hpp>
#include <GL/glu.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core.hpp>
#include "InputLayerVisualisation.hpp"
#include "Range.hpp"

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

    texture.bind(GL_TEXTURE_2D);
    // Set up (lack of) repetition
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);

    // Set up texture scaling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // Use mipmapping for scaling down
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // Use nearest pixel when scaling up.
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_FLOAT, imageData.data());
}

void InputLayerVisualisation::render()
{
    const float vertices[] = {
            // Position, texture coordinates
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

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    texture.bind(GL_TEXTURE_2D);
    glDrawArrays(GL_QUADS, 0, 4);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
