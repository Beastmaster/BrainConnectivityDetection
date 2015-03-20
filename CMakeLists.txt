
cmake_minimum_required(VERSION 2.8)
 
PROJECT(FMRI_TEST)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})
 
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB UI_FILES *.ui)
file(GLOB QT_WRAP *.h)
file(GLOB CXX_FILES *.cpp)  

QT4_WRAP_UI(UISrcs ${UI_FILES})
QT4_WRAP_CPP(MOCSrcs ${QT_WRAP})
add_executable(FMRI_TEST MACOSX_BUNDLE ${CXX_FILES} ${UISrcs} ${MOCSrcs})


if(VTK_LIBRARIES)
  if(${VTK_VERSION} VERSION_LESS "6")
    target_link_libraries(FMRI_TEST ${VTK_LIBRARIES} ${ITK_LIBRARIES} QVTK ${ITK_VERSION_MAJOR} )
  else()
    target_link_libraries(FMRI_TEST ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${ITK_VERSION_MAJOR})
  endif()
else()
  target_link_libraries(FMRI_TEST vtkHybrid QVTK vtkViews ${QT_LIBRARIES})
endif()