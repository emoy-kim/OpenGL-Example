include_directories("${CMAKE_SOURCE_DIR}/3rd_party/glad/include")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/glfw3/include")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/glm")
include_directories("${CMAKE_SOURCE_DIR}/3rd_party/freeimage/include")
link_directories("${CMAKE_SOURCE_DIR}/3rd_party/glad/lib/linux")
link_directories("${CMAKE_SOURCE_DIR}/3rd_party/glfw3/lib/linux")

if(${CMAKE_BUILD_TYPE} MATCHES Debug)
    link_directories("${CMAKE_SOURCE_DIR}/3rd_party/freeimage/lib/debug")
else()
    link_directories("${CMAKE_SOURCE_DIR}/3rd_party/freeimage/lib/release")
endif()