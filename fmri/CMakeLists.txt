message("seek to fmri")

cmake_minimum_required(VERSION 3.0)
project(fmriEngine)

find_package(ITK)
include (${ITK_USE_FILE})
message("ITK_USE_FILE IS ${ITK_USE_FILE}")

find_package(VTK)
include (${VTK_USE_FILE})
message("ITK_USE_FILE IS ${VTK_USE_FILE}")

include_directories("${PROJECT_SOURCE_DIR}")
message("${PROJECT_SOURCE_DIR}")


file(GLOB LOCAL_MODULE_SRCS *.cxx *.cpp)
file(GLOB LOCAL_MODULE_HD *.h)


add_library(fmriEngine ${LOCAL_MODULE_SRCS} ${LOCAL_MODULE_HD})


if(0)
if(VTK_LIBRARIES)
	target_link_libraries(fmriEngine ${VTK_LIBRARIES})
	message("use vtk_library")
ELSE()
	target_link_libraries(fmriEngine vtkHybrid vtkWidgets)
	message("vtklib not found")
endif()

if(ITK_LIBRARIES)
	target_link_libraries(fmriEngine ${ITK_LIBRARIES})
ELSE()
	target_link_libraries(fmriEngine vtkHybrid vtkWidgets)
endif()

endif()
