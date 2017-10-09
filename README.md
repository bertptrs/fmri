# Functional MRI for Neural Networks

An attempt to create real-time in depth visualisation for neural
networks.

## Requirements

The following programs/libraries need to be installed on your system:

- CMake (at least 3.1)
- Caffe
- Google log (glog)
- OpenCV
- Boost System

## Building

The build process is based on CMake. This means you can easily create a
build as follows. Starting from the project root:

```
mkdir build && cd build
cmake ..
make
```

Compilation is a little slow due to the inclusion of Boost.
