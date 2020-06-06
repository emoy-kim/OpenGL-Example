target_link_libraries(OpenGL-Example glad glfw3dll)

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
   target_link_libraries(OpenGL-Example FreeImaged)
else()
   target_link_libraries(OpenGL-Example FreeImage)
endif()