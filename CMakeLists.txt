cmake_minimum_required(VERSION 2.8)

PROJECT(FMRI_TEST)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

include_directories(
advance_dialog
brain_strip_atlas
fmri
qcustomplot
registration
SubWidgetBold
MainWindow
PearsonCorrelation
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(advance_dialog)
add_subdirectory(brain_strip_atlas)
add_subdirectory(fmri)
add_subdirectory(qcustomplot)
add_subdirectory(registration)
add_subdirectory(SubWidgetBold)
add_subdirectory(MainWindow)
add_subdirectory(PearsonCorrelation)

add_executable(FMRI_TEST main.cpp)

target_link_libraries(FMRI_TEST widget_bold)
target_link_libraries(FMRI_TEST Dialog_Advance)
target_link_libraries(FMRI_TEST qcustomplot)
target_link_libraries(FMRI_TEST fmriEngine)
target_link_libraries(FMRI_TEST MainWindow)
target_link_libraries(FMRI_TEST PearsonCorrelationMethod)

#itk_register
#itkStripTsImageFilter
if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(FMRI_TEST ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(FMRI_TEST ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(FMRI_TEST vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()