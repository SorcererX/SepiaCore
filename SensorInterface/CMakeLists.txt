project(SensorInterface)
aux_source_directory(src SRC_LIST)
list( REMOVE_ITEM SRC_LIST "src/fc2capture.cpp" )
set( HEADER_FILES
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/sensorinterface.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/usbreset.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/v4l2camera.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/v4l2capture.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/v4l2interface.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/ximeacapture.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/leptoncapture.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/leptoninterface.h
#     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/fc2capture.h
     ${CMAKE_SOURCE_DIR}/SensorInterface/defs/spinnakercapture.h)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS
                      PROTO_HDRS
                      ${CMAKE_SOURCE_DIR}/messages.proto)

include_directories(defs)
include_directories(/opt/XIMEA/include)
include_directories(/usr/include)
include_directories(/usr/local/include)
include_directories(/usr/include/spinnaker)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} -L/usr/lib -lSpinnaker )
#target_link_libraries( ${PROJECT_NAME} -L/usr/lib -lflycapture )
target_link_libraries( ${PROJECT_NAME} -L/opt/XIMEA/lib -lm3api )
target_link_libraries( ${PROJECT_NAME} -L/usr/lib -lusb-1.0 )
target_link_libraries( ${PROJECT_NAME} -lboost_system -lboost_thread -lv4l2 -lrt )
target_link_libraries( ${PROJECT_NAME} ${PROTOBUF_LIBRARIES} )
