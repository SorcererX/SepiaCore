project(SensorConverter)
aux_source_directory(src SRC_LIST)
set( HEADER_FILES
     ${CMAKE_SOURCE_DIR}/SensorConverter/defs/rectification.h
     ${CMAKE_SOURCE_DIR}/SensorConverter/defs/processthread.h
     ${CMAKE_SOURCE_DIR}/SensorConverter/defs/grfmt_jpeg.h )

find_package( OpenCV REQUIRED )
include_directories(defs)
include_directories(/usr/local/include)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/Lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
target_link_libraries( ${PROJECT_NAME} -ljpeg )
target_link_libraries( ${PROJECT_NAME} -lboost_system -lboost_thread -lrt -pthread )
