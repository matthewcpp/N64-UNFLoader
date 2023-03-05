cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake)

project(UNFLoader CXX)

set(device_lib_sources
	device_everdrive.h device_everdrive.cpp
	device_64drive.h device_64drive.cpp
	device_sc64.h device_sc64.cpp
	device.h device_context.h device.cpp
)

add_library(lib64device SHARED ${device_lib_sources})

target_include_directories(lib64device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

if(WIN32)
	target_compile_definitions(lib64device 
		PUBLIC UNFLOADER_CMAKE SHARED_DEVICE_LIB _CRT_SECURE_NO_WARNINGS
		PRIVATE BUILD_SHARED_DEVICE_LIB
	)
	target_link_libraries(lib64device PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/ftd2xx.lib)
endif()

set(unf_loader_sources
	main.h main.cpp
	debug.h debug.cpp
	helper.h helper.cpp
	helper_internal.h
	Include/lodepng.cpp
)

add_executable(UNFLoader ${unf_loader_sources})

target_link_libraries(UNFLoader PRIVATE lib64device)

if(WIN32)

	target_link_libraries(UNFLoader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/pdcurses.lib)
endif()

# Dependencies:
# "sdl2",
# "websocketpp"

#add_executable(wstest wstest/main.cpp)
#find_package(websocketpp CONFIG REQUIRED)
#target_include_directories(wstest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
#target_link_libraries(wstest PRIVATE websocketpp::websocketpp lib64device)
#set_property(TARGET wstest PROPERTY CXX_STANDARD 17)
#set_property(TARGET wstest PROPERTY CXX_STANDARD_REQUIRED ON)

#add_executable(sdltest sdltest/main.cpp)
#find_package(SDL2 CONFIG REQUIRED)
#target_link_libraries(sdltest PUBLIC SDL2::SDL2 SDL2::SDL2main lib64device)

find_package(tiny-process-library CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_package(unofficial-http-parser CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(restinio CONFIG REQUIRED)

set(asset_transfer_sources
	asset_transfer/converter.hpp asset_transfer/converter.cpp
	asset_transfer/device_interface.hpp
	asset_transfer/device_messages.hpp
	asset_transfer/n64_device.hpp asset_transfer/n64_device.cpp
	asset_transfer/main.cpp
	asset_transfer/processor.hpp asset_transfer/processor.cpp
	asset_transfer/settings.hpp asset_transfer/settings.cpp
	asset_transfer/web_service.hpp asset_transfer/web_service.cpp
)

add_executable(asset_transfer ${asset_transfer_sources})
target_include_directories(asset_transfer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(asset_transfer PRIVATE 
	lib64device 
	tiny-process-library::tiny-process-library 
	stduuid 
	nlohmann_json::nlohmann_json
	restinio::restinio)


set_property(TARGET asset_transfer PROPERTY CXX_STANDARD 17)
set_property(TARGET asset_transfer PROPERTY CXX_STANDARD_REQUIRED ON)