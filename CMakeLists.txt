
cmake_minimum_required (VERSION 2.6)
project (VisUtils)

#find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED)
#find_package(OpenCV REQUIRED)
find_package(OpenCV COMPONENTS core imgproc highgui REQUIRED )
#find_package(GLFW3 REQUIRED)
#find_package(GLEW REQUIRED)
#find_package(glm REQUIRED)

# Ask the user to give the paths to GLFW, GLEW and GLM.
# On Linux this can be found automatically with pkg-config; 
# see http://stackoverflow.com/questions/6889264/cmake-flags-for-opengl-using-glew-and-glfw, for instance.

set(GLFW_INCLUDE_PATH "/usr/include" CACHE PATH "The directory that contains GLEW/glfw3.h" )
set(GLFW_LIBRARY "/usr/lib/x86_64-linux-gnu/libglfw.so" CACHE FILEPATH "libglfw.a or glfw.lib" )

set(GLEW_INCLUDE_PATH "/usr/include" CACHE PATH "The directory that contains GL/glew.h" )
set(GLEW_LIBRARY "/usr/lib/x86_64-linux-gnu/libGLEW.so" CACHE FILEPATH "libglew.a or glew.lib" )

SET(CMAKE_CXX_STANDARD 11)
#SET(CMAKE_CXX_FLAGS "-std=c++11")
SET(CMAKE_CXX_FLAGS "-O3")

#set(GLM_INCLUDE_PATH "/include" CACHE PATH "The directory that contains glm/glm.hpp" )

set(MY_HEADER_FILES
  controls.hpp
  common/quaternion_utils.hpp
  common/shader.hpp
  common/vboindexer.hpp
  byte4color.hpp
  #camcontrol.hpp
  event.hpp
  glutils.hpp
  node.hpp
  property.hpp
  renderer.hpp
  string_tokenizer.hpp
  playback.hpp
  visdefs.hpp
  dendmaker.hpp
  )


set(MY_SOURCE_FILES3
  controls.cpp
  common/quaternion_utils.cpp
  common/shader.cpp
  common/vboindexer.cpp
  string_tokenizer.cpp
  main.cpp
#  test.cpp
  )


include_directories(
  .
  ${GLFW_INCLUDE_PATH}
  ${GLEW_INCLUDE_PATH}
  ${GLM_INCLUDE_PATH}
  )

add_executable(
  vis
  ${MY_HEADER_FILES}
  ${MY_SOURCE_FILES3}
  )
  


target_link_libraries(
  vis
  ${OPENGL_LIBRARY} # filled by "find_package(OpenGL REQUIRED)"
  ${GLFW_LIBRARY}
  ${GLEW_LIBRARY}
  ${BOOST_LIBRARY}
  ${OpenCV_LIBS}
  )

file(COPY instanced.vertexshader DESTINATION ${CMAKE_BINARY_DIR})
file(COPY instanced.fragmentshader DESTINATION ${CMAKE_BINARY_DIR})

