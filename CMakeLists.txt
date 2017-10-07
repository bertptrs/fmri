cmake_minimum_required (VERSION 3.1.0)
project(FMRI CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB fmri_SRC
	"src/*.cpp"
)

# Build without GPU support for quicker development
add_definitions(-DCPU_ONLY)


# Locate libraries
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(fmri ${fmri_SRC})

# Add libraries to link
target_link_libraries(fmri ${OpenCV_libs})
target_link_libraries(fmri ${Boost_LIBRARIES})
# These do not have CMake support, so link manually
target_link_libraries(fmri caffe glog)
