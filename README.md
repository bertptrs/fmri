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
