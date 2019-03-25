cmake_minimum_required(VERSION 3.2)
project(libplyxx)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(TestBigEndian)

file(GLOB LIB_SOURCES "libplyxx/*.cpp")
file(GLOB TEST_SOURCES "test/*.cpp")

set(LIBPLYXX_MAJOR_VERSION 0)
set(LIBPLYXX_MINOR_VERSION 5)
set(LIBPLYXX_PATCH_VERSION 0)
set(LIBPLYXX_VERSION ${LIBPLYXX_MAJOR_VERSION}.${LIBPLYXX_MINOR_VERSION}.${LIBPLYXX_PATCH_VERSION})

include_directories("libplyxx")

add_library(libplyxx0 STATIC ${LIB_SOURCES})
add_library(libplyxx1 SHARED ${LIB_SOURCES})
add_executable(libplyxx_test ${TEST_SOURCES})
target_link_libraries(libplyxx_test libplyxx0)
set_target_properties(libplyxx0 PROPERTIES OUTPUT_NAME "libplyxx" VERSION ${LIBPLYXX_VERSION})
set_target_properties(libplyxx1 PROPERTIES OUTPUT_NAME "libplyxx" VERSION ${LIBPLYXX_VERSION})
install(TARGETS libplyxx0 libplyxx1 DESTINATION lib)
install(FILES libplyxx/libplyxx.h libplyxx/textio.h DESTINATION include)

include_directories(${PROJECT_BINARY_DIR})
configure_file(config.h.in config.h)
configure_file(test/data/test.ply test.ply COPYONLY)
configure_file(test/data/test_bin.ply test_bin.ply COPYONLY)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT libplyxx_test)
