project(SensorController)
aux_source_directory(src SRC_LIST)
set( HEADER_FILES
     ${CMAKE_SOURCE_DIR}/SensorController/defs/ximeacontroller.h)

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS
                      PROTO_HDRS
                      ${CMAKE_SOURCE_DIR}/messages.proto)

include_directories(defs)
include_directories(/usr/local/include)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES} ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} -lrt )
target_link_libraries( ${PROJECT_NAME} ${PROTOBUF_LIBRARIES} )
