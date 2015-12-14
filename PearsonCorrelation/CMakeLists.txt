message("seek to PearsonCorrelationMethod dir")
cmake_minimum_required(VERSION 2.8)
 
PROJECT(PearsonCorrelationMethod)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)

#include dependent head files
include_directories(
../fmri
)
#include source dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(Plain_CXX
PearsonCorrelationMethod.cpp
GenerateTimeCourse.cpp
GraphAnalysis.cpp
)
set(Plain_Head
PearsonCorrelationMethod.h
PearsonCorrelation_algorithm.hpp
GenerateTimeCourse.h
GraphAnalysis.h
)

set(QT_UI_FILES
ROIBasedPanel.ui
)

#qt .h files
set(QT_HEAD_FILES
ROIBasedPanel.h
)

#qt .cxx files
SET(QT_CXX_FILES
ROIBasedPanel.cpp
)

QT5_WRAP_UI(UISrcs ${QT_UI_FILES})
QT5_WRAP_CPP(MOCSrcs ${QT_HEAD_FILES})
add_library(PearsonCorrelationMethod ${QT_CXX_FILES} ${UISrcs} ${MOCSrcs} ${QT_HEAD_FILES} ${Plain_CXX} ${Plain_Head})

if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(PearsonCorrelationMethod ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(PearsonCorrelationMethod ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(PearsonCorrelationMethod vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()