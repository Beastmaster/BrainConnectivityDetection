message("seek to itk strip")
cmake_minimum_required(VERSION 2.8)
 
PROJECT(itkStripTsImageFilter)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#add_library(itkStripTsImageFilter itkStripTsImageFilter.hxx)
