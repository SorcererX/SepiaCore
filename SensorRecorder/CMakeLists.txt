project(SensorRecorder)
aux_source_directory(src SRC_LIST)
set( HEADER_FILES
     ${CMAKE_SOURCE_DIR}/SensorRecorder/defs/record.h
     ${CMAKE_SOURCE_DIR}/SensorRecorder/defs/playback.h )

find_package(OpenCV REQUIRED)
include_directories(defs)
include_directories(/usr/include)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/Lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
target_link_libraries( ${PROJECT_NAME} -lpthread -lrt )
