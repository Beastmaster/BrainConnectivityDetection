message("seek to dialog advance")
cmake_minimum_required(VERSION 2.8)
 
PROJECT(Dialog_Advance)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

#qt .ui files
set(QT_UI_FILES
Dialog_Paradigm_Advance.ui
)

#qt .h files
set(QT_HEAD_FILES
Dialog_Paradigm_Advance.h
)


#qt .cxx files
SET(QT_CXX_FILES
Dialog_Paradigm_Advance.cpp
)

QT4_WRAP_UI(UISrcs ${QT_UI_FILES})
QT4_WRAP_CPP(MOCSrcs ${QT_HEAD_FILES})
add_library(Dialog_Advance  ${QT_CXX_FILES} ${UISrcs} ${MOCSrcs} ${QT_HEAD_FILES})


if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(Dialog_Advance ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(Dialog_Advance ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(Dialog_Advance vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()