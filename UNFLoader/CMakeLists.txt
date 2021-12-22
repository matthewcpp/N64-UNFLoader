cmake_minimum_required(VERSION 3.1)

project(UNFLoader CXX)

set(device_lib_sources
	device_everdrive.h device_everdrive.cpp
	device_64drive.h device_64drive.cpp
	device_sc64.h device_sc64.cpp
	device.h device.cpp
	helper.h helper.cpp
	helper_internal.h
)

add_library(lib64device STATIC ${device_lib_sources})

target_include_directories(lib64device PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)


if(WIN32)
	target_compile_definitions(lib64device PRIVATE UNFLOADER_CMAKE _CRT_SECURE_NO_WARNINGS)
	target_link_libraries(lib64device PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include/ftd2xx.lib)
endif()

set(unf_loader_sources
	main.h main.cpp
	debug.h debug.cpp
	Include/lodepng.cpp
)

add_executable(UNFLoader ${unf_loader_sources})

target_link_libraries(UNFLoader PRIVATE lib64device)

if(WIN32)

	target_link_libraries(UNFLoader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/pdcurses.lib)
endif()