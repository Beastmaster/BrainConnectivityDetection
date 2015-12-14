cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME BC_Tool)
PROJECT(${PROJECT_NAME})
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

#find_package(Qt4 REQUIRED)
#include(${QT_USE_FILE})
# find and use QT5 lib
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)


include_directories(
advance_dialog
brain_strip_atlas
fmri
qcustomplot
registration
SubWidgetBold
MainWindow
PearsonCorrelation
libICA
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
add_subdirectory(libICA)


#qt .cxx files
SET(QT_CXX_FILES
main.cpp
)

#QT4
#QT4_WRAP_CPP(MOCSrcs ${QT_CXX_FILES})
#include source dir
#include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_executable(${PROJECT_NAME} ${QT_CXX_FILES} )



#add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} widget_bold)
target_link_libraries(${PROJECT_NAME} Dialog_Advance)
target_link_libraries(${PROJECT_NAME} qcustomplot)
target_link_libraries(${PROJECT_NAME} fmriEngine)
target_link_libraries(${PROJECT_NAME} MainWindow)
target_link_libraries(${PROJECT_NAME} PearsonCorrelationMethod)
target_link_libraries(${PROJECT_NAME} libICA)

#itk_register
#itkStripTsImageFilter
if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(${PROJECT_NAME} ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(${PROJECT_NAME} vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()