cmake_minimum_required (VERSION 3.1.0)
project(FMRI CXX)

# Allow us to define custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Enable modern C++ features
set(CMAKE_CXX_STANDARD 17)

# Define executable and dependencies
file(GLOB fmri_SRC
	"src/*.cpp"
	"src/*.hpp"
)

add_executable(fmri ${fmri_SRC})

# Enable better warnings
target_compile_options(fmri PRIVATE "-Wall" "-Wextra" "-pedantic")

# Enable CUDA if available
find_package(CUDA)
if (CUDA_FOUND)
	include_directories(${CUDA_INCLUDE_DIRS})
	target_link_libraries(fmri ${CUDA_LIBS})
else()
	add_definitions(-DCPU_ONLY)
endif()


# Library dependencies
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

# Require png++ (dumping to file)
find_package(png++ REQUIRED)
include_directories(${png++_INCLUDE_DIRS})
target_link_libraries(fmri ${png++_LIBRARIES})

# Require GLUT (visualisation)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
target_link_libraries(fmri ${GLUT_LIBRARIES})

# Require OpenGL (visualisation)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
target_link_libraries(fmri ${OPENGL_LIBRARIES})

# Require GLEW (visulalisation)
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
target_link_libraries(fmri ${GLEW_LIBRARIES})


# Allow the package to be installed
install(TARGETS fmri DESTINATION bin)
