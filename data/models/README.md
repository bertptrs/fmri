# FMRI example models

The following sections describe the models included in this repository,
with attribution. The repository also includes deinplaced versions of
the models, where relevant.

## alexnet

AlexNet trained on ILSVRC 2012, almost exactly as described in ImageNet
classification with deep convolutional neural networks by Krizhevsky et
al. in NIPS 2012. (Trained by Evan Shelhamer @shelhamer)

## caffenet

AlexNet trained on ILSVRC 2012, with a minor variation from the version
as described in ImageNet classification with deep convolutional neural
networks by Krizhevsky et al. in NIPS 2012. (Trained by Jeff Donahue
@jeffdonahue)

## vgg-16

The model is an improved version of the 16-layer model used by the VGG
team in the ILSVRC-2014 competition. The details can be found in the
following [arXiv paper](http://arxiv.org/pdf/1409.1556):

    Very Deep Convolutional Networks for Large-Scale Image Recognition
    K. Simonyan, A. Zisserman
    arXiv:1409.1556

Please cite the paper if you use the model.

In the paper, the model is denoted as the configuration `D` trained with
scale jittering. The input images should be zero-centered by mean pixel
(rather than mean image) subtraction. Namely, the following BGR values
should be subtracted: `[103.939, 116.779, 123.68]`.


## vgg-19

The model is an improved version of the 19-layer model used by the VGG
team in the ILSVRC-2014 competition. The details can be found in the
following [arXiv paper](http://arxiv.org/pdf/1409.1556):

    Very Deep Convolutional Networks for Large-Scale Image Recognition
    K. Simonyan, A. Zisserman
    arXiv:1409.1556

Please cite the paper if you use the model.

In the paper, the model is denoted as the configuration `E` trained with
scale jittering. The input images should be zero-centered by mean pixel
(rather than mean image) subtraction. Namely, the following BGR values
should be subtracted: `[103.939, 116.779, 123.68]`.
