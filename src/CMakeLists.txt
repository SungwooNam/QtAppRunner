enable_testing()

set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Test REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable( unittest 
    Fixture.cpp
    testAppRunner.cpp
)

include_directories( 
    ../include
)

target_link_libraries( unittest 
    gtest
    gtest_main
    pthread
    Qt5::Test
    Qt5::Widgets
)

include(GoogleTest)
gtest_discover_tests(unittest)