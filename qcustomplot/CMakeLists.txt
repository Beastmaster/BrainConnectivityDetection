message("seek to qcustomplot")
cmake_minimum_required(VERSION 2.8)
 
PROJECT(qcustomplot)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#qt .ui files
#set(QT_UI_FILES
#${FMRI_MODULE_ui_dir}/Dialog_Paradigm_Advance.ui
#)

#qt .h files
set(QT_HEAD_FILES
qcustomplot.h
)

#qt .cxx files
SET(QT_CXX_FILES
qcustomplot.cpp
)

QT5_WRAP_UI(UISrcs ${UI_FILES})
QT5_WRAP_CPP(MOCSrcs ${QT_HEAD_FILES})
add_library(qcustomplot ${QT_CXX_FILES} ${UISrcs} ${MOCSrcs} ${QT_HEAD_FILES})


if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(qcustomplot ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(qcustomplot ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(qcustomplot vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()