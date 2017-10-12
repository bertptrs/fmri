cmake_minimum_required (VERSION 3.1.0)
project(FMRI CXX)

# Allow us to define custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

set(CMAKE_CXX_STANDARD 17)

file(GLOB fmri_SRC
	"src/*.cpp"
)

# Build without GPU support for quicker development
add_definitions(-DCPU_ONLY)


# Locate libraries

add_executable(fmri ${fmri_SRC})

# Require OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(fmri ${OpenCV_LIBS})

# Require Boost
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(fmri ${Boost_LIBRARIES})

# Require Caffe
find_package(Caffe REQUIRED)
include_directories(${Caffe_INCLUDE_DIRS})
target_link_libraries(fmri ${Caffe_LIBRARIES})

# Require glog
find_package(Glog REQUIRED)
include_directories(${GLOG_INCLUDE_DIRS})
target_link_libraries(fmri ${GLOG_LIBRARIES})

# Require png++
find_package(png++ REQUIRED)
include_directories(${png++_INCLUDE_DIRS})
target_link_libraries(fmri ${png++_LIBRARIES})
