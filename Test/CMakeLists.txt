project(Test)
set( HEADER_FILES )

aux_source_directory(src SRC_LIST)
include_directories(/usr/local/include)
include_directories(defs)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${HEADER_FILES})
target_link_libraries( ${PROJECT_NAME} -L/usr/local/Lib -lSepiaStream )
target_link_libraries( ${PROJECT_NAME} -lboost_system -lboost_program_options -lboost_thread -lrt -lz )
