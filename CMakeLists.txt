cmake_minimum_required (VERSION 3.9.0)
project(FMRI CXX)

# Allow us to define custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Enable modern C++ features
set(CMAKE_CXX_STANDARD 17)

# Define executable and dependencies
file(GLOB fmri_SRC
	"src/fmri/*.cpp"
	"src/fmri/*.hpp"
)

option(WITH_LAUNCHER "build GUI launcher" ON)
option(WITH_DEINPLACE "build deinplace tool" ON)

add_executable(fmri ${fmri_SRC})

# Enable better warnings
target_compile_options(fmri PRIVATE "-Wall" "-Wextra" "-pedantic")

# Prefer GLNVD if available
if (POLICY CMP0072)
	cmake_policy(SET CMP0072 NEW)
endif()

# Find dependencies
find_package(Caffe REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem program_options)
find_package(OpenCV 3 REQUIRED COMPONENTS core imgproc imgcodecs)
find_package(Threads REQUIRED)

target_link_libraries(fmri PUBLIC
	Caffe::Caffe
	GLUT::GLUT
	OpenGL::GLU
	Boost::program_options
	opencv_core
	opencv_imgproc
	opencv_imgcodecs
	Threads::Threads
	)

target_include_directories(fmri PUBLIC
	${Caffe_INCLUDE_DIRS}
)

install(TARGETS fmri DESTINATION bin)

if (WITH_DEINPLACE)
	# Build instructions for the deinplace tool
	find_package(Protobuf REQUIRED)
	add_executable(fmri-deinplace "src/tools/deinplace.cpp")
	target_compile_options(fmri-deinplace PRIVATE "-Wall" "-Wextra" "-pedantic")
	target_link_libraries(fmri-deinplace Caffe::Caffe protobuf::libprotobuf)
	install(TARGETS fmri-deinplace DESTINATION bin)
endif()

if (WITH_LAUNCHER)
	# Build isntructions for the launcher tool
	find_package(GTK3 REQUIRED COMPONENTS gtk gtkmm)
	add_executable(fmri-launcher src/tools/launcher.cpp)
	target_compile_options(fmri-launcher PRIVATE "-Wall" "-Wextra" "-pedantic")
	target_link_libraries(fmri-launcher GTK3::gtkmm Boost::filesystem GTK3::gtk)
	install(TARGETS fmri-launcher DESTINATION bin)
endif()

# Allow the package to be installed
