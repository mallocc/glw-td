#########################
# Tree for this file should be:
#
# src/            (*.cpp)
# inc/            (*.h)
# build/          (build files)
# CMakeLists.txt  (this file)
#
# Leave '#_files' in place for an outside run script
#
# Run cmake from within 'build/'
# Run make from within 'build/'
# Run the target from within 'build/'
#########################

# cmake version
cmake_minimum_required(VERSION 3.2)

# project name
project(glw-td)

# set the include directory for header files
set(GLWINC ${PROJECT_SOURCE_DIR}/../glw/inc)
include_directories(${GLWINC})

# set the source directory for the source files
set(GLWSRC ${PROJECT_SOURCE_DIR}/../glw/src)

# specifiy each source file to be compiled 
set(GLWSRC_FILES
${GLWSRC}/GTextureManager.cpp
${GLWSRC}/GGUI.cpp
${GLWSRC}/ImageLoader.cpp
${GLWSRC}/GFrameBufferObject.cpp
${GLWSRC}/GCamera.cpp
${GLWSRC}/GPrimativeFactory.cpp
${GLWSRC}/GVertexBufferObject.cpp
${GLWSRC}/GShaderProgramManager.cpp
${GLWSRC}/GShaderVariableHandle.cpp
${GLWSRC}/GShaderProgram.cpp
${GLWSRC}/GContent.cpp
)

# set the include directory for header files
set(INC ${PROJECT_SOURCE_DIR}/inc)
include_directories(${INC})

# set the source directory for the source files
set(SRC ${PROJECT_SOURCE_DIR}/src)

# specifiy each source file to be compiled 
set(SRC_FILES
#files
#${SRC}/TD.cpp
${SRC}/main.cpp
)

# link source files to the project already specified
add_executable(glw-td ${SRC_FILES} ${GLWSRC_FILES})

# link libraries to the project to compile
target_link_libraries(glw-td PUBLIC GL GLU GLEW glfw3 X11 Xxf86vm Xrandr pthread Xi dl Xinerama Xcursor)
