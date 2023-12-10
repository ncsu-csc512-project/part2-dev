cmake_minimum_required(VERSION 3.10)

project(SeminalInputFeaturesDetection)

set(LT_LLVM_INSTALL_DIR "/usr/local/opt/llvm" CACHE PATH "LLVM installation directory")

# Add the location of LLVMConfig.cmake to CMake search paths (so that find_package can locate it)
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

# Find the LLVM package
find_package(LLVM CONFIG REQUIRED)

if("${LLVM_VERSION_MAJOR}" VERSION_LESS 16)
    message(FATAL_ERROR "Found LLVM ${LLVM_VERSION_MAJOR}, but need LLVM 16 or above")
endif()

# Add LLVM's headers to our include path
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
# Set the C++ standard used by LLVM
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
# LLVM is built without RTTI by default, keep consistent with that
if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# Specify where our headers are
include_directories(include/)
include_directories(/usr/local/include)
# Define our shared library for InputDetectionPass
# add_library(InputDetectionPass MODULE
#     lib/SeminalInputFeatures/InputDetectionPass.cpp
# )

# Define our shared library for DefUseAnalysisPass
add_library(DefUseAnalysisPass MODULE
    lib/SeminalInputFeatures/DefUseAnalysisPass.cpp
)

# Specify required C++ features for our modules
#target_compile_features(InputDetectionPass PRIVATE cxx_range_for cxx_auto_type)
target_compile_features(DefUseAnalysisPass PRIVATE cxx_range_for cxx_auto_type)

# Link against LLVM libraries
#target_link_libraries(InputDetectionPass LLVM)
target_link_libraries(DefUseAnalysisPass LLVM)
