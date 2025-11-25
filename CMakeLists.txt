cmake_minimum_required(VERSION 3.4...3.18)
set(CMAKE_CXX_STANDARD 17)
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

project(fastwfc)

set(SAMPLES_XML ${CMAKE_SOURCE_DIR}/samples.xml)
set(SAMPLES_DIR ${CMAKE_SOURCE_DIR}/samples)

if(NOT EXISTS ${SAMPLES_XML})
  message(FATAL_ERROR "samples.xml not found at ${SAMPLES_XML}")
endif()

if(NOT EXISTS ${SAMPLES_DIR})
  message(FATAL_ERROR "samples directory not found at ${SAMPLES_DIR}")
endif()

set(SAMPLES_OUTPUT_DIR ${CMAKE_BINARY_DIR})
set(SAMPLES_OUTPUT ${SAMPLES_OUTPUT_DIR}/samples.xml)
set(SAMPLES_OUTPUT_SAMPLES_DIR ${SAMPLES_OUTPUT_DIR}/samples)

add_custom_target(copy_samples ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SAMPLES_XML} ${SAMPLES_OUTPUT}
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${SAMPLES_DIR} ${SAMPLES_OUTPUT_SAMPLES_DIR}
  BYPRODUCTS ${SAMPLES_OUTPUT}
  COMMENT "Ensuring samples.xml and samples directory are available in the build tree"
)

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories("${CMAKE_SOURCE_DIR}/src/external/fast-wfc/src/include")

set(FASTWFC_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/external/fast-wfc/src )

file (GLOB SOURCE_FILES "${FASTWFC_SOURCE_PATH}/lib/*.cpp" "${CMAKE_SOURCE_DIR}/src/*.cpp")
file (GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/src/external/fast-wfc/src/include/*.hpp" "${CMAKE_SOURCE_DIR}/src/*.hpp")
file (GLOB PYTHON_FILES "${CMAKE_SOURCE_DIR}/src/python/*.cpp" "${CMAKE_SOURCE_DIR}/src/python/*.hpp")

# Set up such that XCode organizes the files
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES} ${HEADER_FILES} ${PYTHON_FILES} )

add_library(FASTWFC_LIB ${SOURCE_FILES} ${HEADER_FILES})
set_target_properties(FASTWFC_LIB PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(FASTWFC_LIB copy_samples)

#
#  add_executable(${PROJECT_NAME} src/main.cpp.bak)
#  target_link_libraries(${PROJECT_NAME} FASTWFC_LIB)

add_subdirectory(pybind11)
pybind11_add_module(fastwfc
	${SOURCE_FILES}
	${HEADER_FILES}
	${PYTHON_FILES}
)
add_dependencies(fastwfc copy_samples)

target_link_libraries(fastwfc PUBLIC ${FASTWFC_LIB})

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(fastwfc
                          PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
