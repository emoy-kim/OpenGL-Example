cmake_minimum_required(VERSION 3.8)

project("OpenGL-Example")

include(cmake/check-compiler.cmake)

set(CMAKE_CXX_STANDARD 17)

set(
	SOURCE_FILES 
		main.cpp
		source/light.cpp
		source/camera.cpp
		source/object.cpp
		source/shader.cpp
		source/renderer.cpp
)

configure_file(include/project_constants.h.in ${PROJECT_BINARY_DIR}/project_constants.h @ONLY)

include_directories("include")
if(MSVC)
   include(cmake/add-libraries-windows.cmake)
else()
   include(cmake/add-libraries-linux.cmake)
endif()

add_executable(OpenGL-Example ${SOURCE_FILES})

if(MSVC)
   include(cmake/target-link-libraries-windows.cmake)
else()
   include(cmake/target-link-libraries-linux.cmake)
endif()

target_include_directories(OpenGL-Example PUBLIC ${CMAKE_BINARY_DIR})