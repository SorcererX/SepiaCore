project(SensorInterface)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libusb REQUIRED libusb-1.0)
pkg_check_modules(v4l2 REQUIRED libv4l2)
pkg_check_modules(aravis REQUIRED aravis-0.8)
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS
                      PROTO_HDRS
                      ${CMAKE_SOURCE_DIR}/messages.proto)

include_directories(defs)
include_directories(/usr/include)
include_directories(/usr/local/include)


add_executable(${PROJECT_NAME} ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${libusb_INCLUDE_DIRS}
    ${v4l2_INCLUDE_DIRS}
    ${aravis_INCLUDE_DIRS})

target_link_libraries( ${PROJECT_NAME} PRIVATE
    -L/usr/local/lib -lSepiaStream
    ${libusb_LIBRARIES}
    ${v4l2_LIBRARIES}
    ${PROTOBUF_LIBRARIES}
    ${aravis_LIBRARIES}
    -lrt )

target_sources( ${PROJECT_NAME} PRIVATE
    src/araviscamera.cpp
    src/araviscapture.cpp
    src/aravisimage.cpp
    src/aravisstream.cpp
    src/leptoncapture.cpp
    src/leptoninterface.cpp
    src/main.cpp
    src/sensorinterface.cpp
    src/v4l2camera.cpp
    src/v4l2capture.cpp
    src/v4l2interface.cpp
    src/usbreset.cpp
    defs/araviscamera.h
    defs/araviscapture.h
    defs/aravisimage.h
    defs/aravisstream.h
    defs/sensorinterface.h
    defs/usbreset.h
    defs/v4l2camera.h
    defs/v4l2capture.h
    defs/v4l2interface.h
    defs/leptoncapture.h
    defs/leptoninterface.h )
target_link_libraries( ${PROJECT_NAME}  )
