project(SensorViewer)
aux_source_directory(src SRC_LIST)
set( HEADER_FILES
     ${CMAKE_SOURCE_DIR}/SensorViewer/defs/settings.h
     ${CMAKE_SOURCE_DIR}/SensorViewer/defs/processthread.h )

find_package(OpenCV REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(defs)
include_directories(/usr/local/include)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/Lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
target_link_libraries( ${PROJECT_NAME} OpenGL::GL glfw -lGLU -lrt -pthread )
