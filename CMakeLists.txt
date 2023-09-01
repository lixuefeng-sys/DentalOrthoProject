
cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

if(POLICY CMP0020)
  cmake_policy(SET CMP0020 NEW)
  cmake_policy(SET CMP0071 NEW)
endif()

PROJECT(DentalOrtho)

find_package(VTK COMPONENTS 
  vtkCommonColor
  vtkCommonCore
  vtkFiltersCore
  vtkFiltersSources
  vtkFiltersExtraction
  vtkFiltersModeling
  vtkInteractionStyle
  vtkInteractionWidgets
  vtkRenderingContextOpenGL2
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingGL2PSOpenGL2
  vtkRenderingOpenGL2
  vtkRenderingAnnotation
  vtkGUISupportQt
  vtkRenderingQt
  vtkIOGeometry
  vtkCommonExecutionModel
  QUIET
)

if(NOT VTK_FOUND)
  message(FATAL_ERROR "${CMAKE_PROJECT_NAME}: ${VTK_NOT_FOUND_MESSAGE}")
endif()

if(NOT(vtkGUISupportQt_LOADED OR TARGET VTK::GUISupportQt))
  message(FATAL_ERROR "${CMAKE_PROJECT_NAME}: VTK not built with Qt support.")
endif()

if(NOT DEFINED VTK_QT_VERSION)
  set(VTK_QT_VERSION 5)
endif()

set(qt_components Core Gui Widgets Test)
if(${VTK_QT_VERSION} VERSION_GREATER_EQUAL 6)
  list(APPEND qt_components OpenGLWidgets)
endif()
list(SORT qt_components)
# We have ui files, so this will also bring in the macro:
#   qt5_wrap_ui or qt_wrap_ui from Widgets.
find_package(Qt${VTK_QT_VERSION} QUIET REQUIRED COMPONENTS ${qt_components} )

foreach(_qt_comp IN LISTS qt_components)
  list(APPEND qt_modules "Qt${VTK_QT_VERSION}::${_qt_comp}")
endforeach()

message (STATUS "VTK_VERSION: ${VTK_VERSION}, Qt Version: ${Qt${VTK_QT_VERSION}Widgets_VERSION}")

# Instruct CMake to run moc and uic automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

file(GLOB UI_FILES *.ui)
file(GLOB H_FILES *.h)
file(GLOB CXX_FILES *.cxx)
file(GLOB RES_FILES *.qrc)

file(GLOB MODEL_FILES *.stl)

file(GLOB CONFIG_FILES *.ini)

# CMAKE_AUTOMOC in ON so the MOC headers will be automatically wrapped.
add_executable(${CMAKE_PROJECT_NAME} MACOSX_BUNDLE
  ${CXX_FILES} ${UI_FILES} ${H_FILES} ${RES_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${qt_modules})
target_link_libraries(${CMAKE_PROJECT_NAME} ${VTK_LIBRARIES})

# copy model files to build dir
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${MODEL_FILES} $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}>)

# vtk_module_autoinit is needed
vtk_module_autoinit(
  TARGETS ${CMAKE_PROJECT_NAME}
  MODULES ${VTK_LIBRARIES}
)
