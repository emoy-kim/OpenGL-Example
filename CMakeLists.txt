cmake_minimum_required(VERSION 3.8)

project("OpenGL-Example")

include(cmake/check-compiler.cmake)

set(CMAKE_CXX_STANDARD 17)

set(
	SOURCE_FILES 
		main.cpp
		Light.cpp
		Camera.cpp
		Object.cpp
		Shader.cpp
		Renderer.cpp
)

configure_file(ProjectPath.h.in ${PROJECT_BINARY_DIR}/ProjectPath.h @ONLY)

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