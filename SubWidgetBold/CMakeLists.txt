message("seek to SubWidgetBold")
cmake_minimum_required(VERSION 2.8)
 
PROJECT(widget_bold)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

#find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})

#include dependent head files
include_directories(
../advance_dialog
../brain_strip_atlas
../fmri
../qcustomplot
)
#include source dir
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})


set(QT_UI_FILES
SubWidgetParadigmInBold.ui
MyProgressDialog.cpp
)

#qt .h files
set(QT_HEAD_FILES
SubWidgetParadigmInBold.h
MyProgressDialog.h
)

#qt .cxx files
SET(QT_CXX_FILES
SubWidgetParadigmInBold.cpp
)


# find and use QT5 lib
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
QT5_WRAP_UI(UISrcs ${QT_UI_FILES})
QT5_WRAP_CPP(MOCSrcs ${QT_HEAD_FILES})

#QT4_WRAP_UI(UISrcs ${QT_UI_FILES})
#QT4_WRAP_CPP(MOCSrcs ${QT_HEAD_FILES})
add_library(widget_bold ${QT_CXX_FILES} ${UISrcs} ${MOCSrcs} ${QT_HEAD_FILES})



#target_link_libraries(widget_bold itk_register qcustomplot fmriEngine itkStripTsImageFilter Dialog_Advance)

if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(widget_bold ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(widget_bold ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(widget_bold vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()