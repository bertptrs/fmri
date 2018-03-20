# Functional MRI for Neural Networks

An attempt to create real-time in depth visualisation for neural
networks.

## Requirements

The following programs/libraries need to be installed on your system:

- CMake (at least 3.8)
- Caffe
- Google log (glog)
- OpenCV (at least 2.6, 3.x works fine.)
- Boost System
- PNG++

Note that most of these are dependencies of Caffe, and thus will
probably already be on your system. For OS-specific instructions, see
below.

### Arch Linux

1. Install either `caffe` or `caffe-cpu` from the AUR.
2. Install other dependencies:
    - `png++`
    - `cmake`
    - `freeglut`
3. Do the build as normal

### Ubuntu

Caffe is available from the Ubuntu repositories from version 17.10
(Artful Aardvark) onwards, and can be used to simplify the installation
process. The commands required are as follows:

    # Install dependencies
    sudo apt install libcaffe-cpu-dev freeglut3-dev libpng++-dev \
        build-essential cmake libopencv-dev libboost-system-dev \
        libgoogle-glog-dev libblas-dev libprotobuf-dev

    # Do the build normally

Older versions of Ubuntu need some help, as the `caffe` package does not
exist, and the supplied `cmake` version is too old.

1. First, install a recent version of [cmake](https://cmake.org/). Any
   version from 3.8 onwards will do.
2. Then, install caffe from source. Installation instructions can be
   found [on the Caffe
   website](http://caffe.berkeleyvision.org/install_apt.html). Be sure
   to also install the development headers.
3. Run the build as normal.

## Building

The build process is based on CMake. This means you can easily create a
build as follows. Starting from the project root:

    mkdir build && cd build
    cmake ..
    make

Compilation is a little slow due to the inclusion of Boost.

## Usage

This program can operate on most Caffe models, provided they don't
split/merge the input, since the visualisers cannot handle more than
one input.

There is one valid model provided, Caffenet. It can be downloaded by
running `./download-models.sh` from the data folder.

After that, you can, for example, run the program as follows
(assuming you are in the build directory)

    ./fmri -n ../data/models/caffenet/model-dedup.prototxt \
        -w ../data/models/caffenet/bvlc_reference_caffenet.caffemodel \
        -l ../data/ilsvrc12/synset_words.txt \
        ../data/samples/*.jpg

This will run the network on the deduplicated caffenet (see: [limitations](#lmitations))
with the correct weights file and the labels file supplied, on all jpegs
located in the samples folder. More advanced usage can be discovered using
the `-h` option of the executable.

### Controls

You can move around with the WASD keys, and look around using the mouse.
Arrow keys change the currently loaded input.

## Limitations

The following documents the limitations currently present in the program,
including possible workarounds. Note that these may change at any time.

### No in-place layers

Due to the way Caffe works, this program cannot properly extract state
data from networks that use in-place computation, since it cannot observe
the input- and output state of each layer.

To work around this, a simple script `deinplace.py` is included in the tools
folder. To use it, first run make (requires `protoc` to be on your PATH) and
then run it on your program. To see what options it supports, use its `-h`
option.

### Single input/output

The visualisation structuree only supports linear networks. That is: network
where each layer only ever reads from one input layer and only produces one
output. This is not a fundamental limitation, and could be reworked in future
versions.

### Data input layer only

The program only supports reading input from image files, rather than using
LLDB or LevelDB sources. Render your inputs to an image format of choice to
use the program.
