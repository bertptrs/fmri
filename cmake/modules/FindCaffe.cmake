#
# Try to find the Caffe library and include path.
# Once done this will define
#
# CAFFE_FOUND
# Caffe_INCLUDE_DIRS
# Caffe_LIBRARIES
#
# A custom location can be specified for the library using Caffe_DIR
#

set (Caffe_DIR "" CACHE STRING
  "Custom location of the root directory of a Caffe installation")

find_path(Caffe_INCLUDE_DIR
  NAMES caffe/caffe.hpp
  PATHS ${Caffe_DIR}
  DOC "The directory where caffe.hpp resides")

find_library(Caffe_LIBRARY
  NAMES caffe
  PATH_SUFFIXES build/lib
  PATHS ${Caffe_DIR}
  DOC "The Caffe library")

find_package(Glog REQUIRED)
find_package(Boost 1.55 REQUIRED COMPONENTS system)

find_package(CUDA)
if (CUDA_FOUND)
	message("CUDA found, enabling support")
    set(Caffe_ADDITIONAL_LIBS CUDA::CUDA)
else()
	message("No CUDA, compiling CPU-only mode")
    set(Caffe_DEFINITIONS "CPU_ONLY")
endif()


# handle the QUIETLY and REQUIRED arguments and set CAFFE_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Caffe
  "Could NOT find Caffe. Declare Caffe_DIR (either using -DCaffe_DIR=<path> flag or via CMakeGUI/ccmake) to point to root directory of library."
  Caffe_LIBRARY Caffe_INCLUDE_DIR)

if (CAFFE_FOUND)
    add_library(Caffe::Caffe UNKNOWN IMPORTED)
    set_target_properties(Caffe::Caffe PROPERTIES
        INTERFACE_COMPILE_DEFINITIONS "${Caffe_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${Caffe_INCLUDE_DIR}"
        IMPORTED_LOCATION "${Caffe_LIBRARY}"
        INTERFACE_LINK_LIBRARIES "Glog::Glog;Boost::system;${Caffe_ADDITIONAL_LIBS}"
        )

    set(Caffe_LIBRARIES ${Caffe_LIBRARY})
    set(Caffe_INCLUDE_DIRS ${Caffe_INCLUDE_DIR})
endif(CAFFE_FOUND)

mark_as_advanced(
  Caffe_INCLUDE_DIR
  Caffe_LIBRARY)
