cmake_minimum_required(VERSION 3.1.0)
project(VulkanTutorials)

set(QT_DIR "C:/qt" CACHE PATH "qt sdk dir")
set(VULKAN_DIR "C:/vulkan" CACHE PATH "vulkan sdk dir") 

set(QT_KEY_PATH $CACHE{QT_DIR}/include/QtCore)
if(NOT EXISTS ${QT_KEY_PATH})
	message("invalid entry QT_DIR : " ${QT_DIR})
	message("can not find path: " ${QT_KEY_PATH})
	return()
endif()

set(VULKAN_KEY_FILE $CACHE{VULKAN_DIR}/include/vulkan/vulkan.h)
if(NOT EXISTS ${VULKAN_KEY_FILE})
	message("invalid entry VULKAN_DIR : " ${VULKAN_DIR})
	message("can not find file: " ${VULKAN_KEY_FILE})
	return()
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED false)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PROJECT_SOURCE_DIR}/bin)

set(CMAKE_DEBUG_POSTFIX _d)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(Qt5_DIR ${QT_DIR}/lib/cmake/Qt5)
include_directories(${QT_DIR}/include)
include_directories(${VULKAN_DIR}/include)
link_directories(${QT_DIR}/lib)
link_directories(${VULKAN_DIR}/lib)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_subdirectory(Tutorial01)
