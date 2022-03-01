# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

set(PROJECT_NAME tau)


project(${PROJECT_NAME})
set(SRC_DIR ../../../../../../src)
set (JNI_UTILS_SRC_DIR JniUtils)
set (JNI_EDITOR_SRC_DIR Editor)

set (OBOE_DIR ../../../../thrids/oboe)
add_subdirectory(${OBOE_DIR} ./oboe-bin)
include_directories(${OBOE_DIR}/include)

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -Wall -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -Wall -s")
#
#SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS}   -O0 -Wall -g2 -ggdb")
#SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS}  -Os -Wall -s")

include_directories( "${SRC_DIR}/core/" )
include_directories( "${SRC_DIR}/thrids/" )

include_directories( "${SRC_DIR}/thrids/DaisySP/" )

#set (tau_sources
#        jni/jniEqualizer.cpp
#        jni/jniTau.cpp
#        jni/jniTauEditor.cpp
#        jni/jniSoundFont.cpp
#        jni/native-lib.cpp
#        jni/jniBode.cpp
#        jni/jniFilter.cpp
#        jni/jniMorphLBHFilter.cpp)


file(GLOB_RECURSE cpp_src "${SRC_DIR}/*.cpp" "jni/*.cpp"  "${JNI_UTILS_SRC_DIR}/*.cpp" "${JNI_EDITOR_SRC_DIR}/*.cpp")  #指定当前目录下的所有.cpp文件（包括子目录）
add_library( ${PROJECT_NAME} SHARED ${tau_sources}  ${cpp_src} )


include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
add_library(cpufeatures STATIC ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       ${PROJECT_NAME}
                       cpufeatures
                       oboe
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

#target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror "$<$<CONFIG:RELEASE>:-Ofast>")
