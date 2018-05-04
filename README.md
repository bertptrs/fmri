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

Note that most of these are dependencies of Caffe, and thus will
probably already be on your system. For OS-specific instructions, see
below.

### Arch Linux

1. Install either `caffe` or `caffe-cpu` from the AUR.
2. Install other dependencies:
    - `cmake`
    - `freeglut`
    - `gtkmm3`
3. Do the build as normal

### Ubuntu

Caffe is available from the Ubuntu repositories from version 17.10
(Artful Aardvark) onwards, and can be used to simplify the installation
process. The commands required are as follows:

    # Install dependencies
    sudo apt install libcaffe-cpu-dev freeglut3-dev libgtkmm-3.0-dev \
        build-essential cmake libopencv-dev libboost-system-dev \
        libgoogle-glog-dev libblas-dev libprotobuf-dev \
        libboost-filesystem-dev libboost-program-options-dev

    # Do the build normally

These commands were tested on Ubuntu 18.04. Versions as old as 16.04
will work, but can be very painful and labor intesnive to set up.

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

There are a few valid models provided. It can be downloaded by
running `./download-models.sh` from the data folder.

After that, you can, for example, run the program as follows
(assuming you are in the build directory)

    ./fmri -n ../data/models/caffenet/model-dedup.prototxt \
        -w ../data/models/caffenet/bvlc_reference_caffenet.caffemodel \
        -l ../data/ilsvrc12/synset_words.txt \
        ../data/samples/*.jpg

This will run the network on the deduplicated caffenet (see: [limitations](#limitations))
with the correct weights file and the labels file supplied, on all jpegs
located in the samples folder. More advanced usage can be discovered using
the `-h` option of the executable.

Additionally, a graphical launcher is included, called `fmri-launcher`.
It is built by default unless disabled at compile time. The interface
contains a button or a chooser for every option in the program.

### Controls

You can move around with the WASD keys, and look around using the mouse.
Arrow keys change the currently loaded input. F1 brings up an overlay
that tells you all other options.

## Limitations

The following documents the limitations currently present in the program,
including possible workarounds. Note that these may change at any time.

### No in-place layers

Due to the way Caffe works, this program cannot properly extract state
data from networks that use in-place computation, since it cannot observe
the input- and output state of each layer.

To work around this, a simple program `deinplace` is included as a tool,
which will rewrite an existing network to not use in-place computation.
Its options are documented with its `-h` flag.

### Single input/output

The visualisation structuree only supports linear networks. That is: network
where each layer only ever reads from one input layer and only produces one
output. This is not a fundamental limitation, and could be reworked in future
versions.

### Data input layer only

The program only supports reading input from image files, rather than using
LLDB or LevelDB sources. Render your inputs to an image format of choice to
use the program.
