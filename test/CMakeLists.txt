cmake_minimum_required(VERSION 3.10)

project(tests)
include(CTest)
message("cmake unit-test file")


set(CMAKE_CXX_FLAGS_RELEASE "-O3")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(
tests main.cpp
    ../src/OpenSidescan/utilities/opencvhelper.cpp
    ../src/OpenSidescan/detector/roidetector.cpp
    ../src/OpenSidescan/detector/detector.cpp
    ../src/OpenSidescan/detector/houghdetector.cpp
    ../src/OpenSidescan/inventoryobject/inventoryobject.cpp
    ../src/OpenSidescan/sidescan/sidescanimage.cpp
    ../src/OpenSidescan/sidescan/sidescanfile.cpp
    ../src/thirdParty/MBES-lib/src/datagrams/DatagramParser.cpp
    ../src/thirdParty/MBES-lib/src/datagrams/DatagramParserFactory.cpp
    ../src/thirdParty/MBES-lib/src/datagrams/s7k/S7kParser.cpp
    ../src/thirdParty/MBES-lib/src/datagrams/kongsberg/KongsbergParser.cpp
    ../src/thirdParty/MBES-lib/src/datagrams/xtf/XtfParser.cpp
    ../src/thirdParty/MBES-lib/src/utils/NmeaUtils.cpp
    ../src/thirdParty/MBES-lib/src/utils/StringUtils.cpp
    ../src/thirdParty/MBES-lib/src/sidescan/SidescanPing.cpp
    ../src/thirdParty/MBES-lib/src/getopt.c
)

if(UNIX)
	message("*nix unit-test")

	# find_package will locate dependencies wheter they are installed from source or from a package manager
	find_package (Eigen3 3.3 REQUIRED NO_MODULE)
	find_package(OpenCV 4.5 REQUIRED)
	find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)
	target_link_libraries (tests stdc++fs)
	target_link_libraries(tests ${OpenCV_LIBS})
	target_link_libraries (tests Eigen3::Eigen)
endif()

if(WIN32)
	message("windows unit-test")
	include_directories(C:/LIBS/eigen)
	find_package(OpenCV 4.5 REQUIRED PATHS C:/LIBS/opencv4.5/opencv/build/x64/vc15/lib)
	find_package(Qt5 5.12 COMPONENTS Widgets Core REQUIRED PATHS C:/Qt2/5.12.11/msvc2015_64/lib/cmake/Qt5)
	find_package(Qt5Xml REQUIRED PATHS C:/Qt2/5.12.11/msvc2015_64/lib)

	target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Xml ${OpenCV_LIBS})
	
endif()

set_target_properties(tests PROPERTIES LINKER_LANGUAGE CXX)

# add_test(name target param)
add_test("test0" tests)
