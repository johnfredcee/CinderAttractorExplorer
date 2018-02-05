
CMAKE_MINIMUM_REQUIRED(VERSION 2.8) 


PROJECT(AttractorExplorer)

add_definitions(-D_UNICODE)
add_definitions(-DUNICODE) 

# Overide with cmake -DCMAKE_BUILD_TYPE=Debug
if(NOT CMAKE_BUILD_TYPE)
   message("Build type not set (defaults to release)")
   message("-DCMAKE_BUILD_TYPE=Debug for debug")
   set( CMAKE_BUILD_TYPE Release )
endif(NOT CMAKE_BUILD_TYPE)


################################################################################
# Add local path for finding packages, set the local version first
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/Cmake")

set(CINDER_ROOT "D:\\Dev\\libraries\\Cinder")
find_package(Cinder REQUIRED)
message(STATUS "cinder = ${Cinder_FOUND}")

#incude cinder and boost header files and libraries.
include_directories(${CINDER_INCLUDE_DIRS})
link_directories(${CINDER_LIBRARY_DIRS})

# the Cinder program source code.
INCLUDE_DIRECTORIES("include" "blocks/NanoVG/include" "blocks/NanoVG/deps/nanovg/src")
ADD_EXECUTABLE(AttractorExplorer WIN32 "blocks/NanoVG/deps/nanovg/src/nanovg.c"  "blocks/NanoVG/src/ci_nanovg_gl.cpp"  "blocks/NanoVG/src/ci_nanovg.cpp"  "blocks/nanovg/src/SvgRenderer.cpp" "src/CinderProjectApp.cpp")

TARGET_LINK_LIBRARIES(AttractorExplorer ${CINDER_LIBRARIES})

foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
   if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
   endif(${flag_var} MATCHES "/MD")
endforeach(flag_var)