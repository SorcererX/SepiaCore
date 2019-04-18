project(ImagePlayer)

aux_source_directory(src SRC_LIST)
set( HEADER_FILES
     ${CMAKE_SOURCE_DIR}/ImagePlayer/defs/imageplayer.h )

find_package(OpenCV REQUIRED)

include_directories(defs)
include_directories(/usr/local/include)

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )
target_link_libraries( ${PROJECT_NAME} -lboost_system -lboost_program_options -lrt -pthread )
