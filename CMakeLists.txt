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
	target_include_directories(fmri PUBLIC ${CUDA_INCLUDE_DIRS})
	target_link_libraries(fmri PUBLIC ${CUDA_LIBS})
else()
	add_definitions(-DCPU_ONLY)
endif()

# Find dependencies
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(Caffe REQUIRED)
find_package(Glog REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
find_package(png++ REQUIRED)

target_include_directories(fmri PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(fmri PUBLIC
	${OpenCV_LIBS}
	${Caffe_LIBRARIES}
	${png++_LIBRARIES}
	${GLOG_LIBRARIES}
	Boost::system
	GLUT::GLUT
	OpenGL::GLU
	)

target_include_directories(fmri PUBLIC
	${Caffe_INCLUDE_DIRS}
	)

# Allow the package to be installed
install(TARGETS fmri DESTINATION bin)
