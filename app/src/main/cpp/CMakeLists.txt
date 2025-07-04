# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project("customweb")

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(litehtml)

include_directories(litehtml)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(nativeexample SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp)

add_library(southern_litehtml SHARED
        southern_lite_html.cpp
)

#file(GLOB GUMBO_SOURCES
#        litehtml/src/gumbo/*.c
#)

#add_library(gumbo STATIC ${GUMBO_SOURCES})

#target_include_directories(gumbo PUBLIC
#        litehtml/src/gumbo/include
#        litehtml/src/gumbo/include/gumbo
#)

# NDK libraries
#find_library(
#        log-lib
#        log
#)
find_library(
        jnigraphics-lib
        jnigraphics
)

# Add litehtml files on the southern_litehtml native lib
target_include_directories(
        southern_litehtml
        PRIVATE
        litehtml/include
        litehtml/include/litehtml
        litehtml/src/gumbo/include
        litehtml/src/gumbo/include/gumbo
        litehtml/src/gumbo
        litehtml/src
)

file(GLOB LITEHTML_SOURCES
        litehtml/src/*.cpp
)

add_library(litehtml_static STATIC ${LITEHTML_SOURCES})

target_include_directories(litehtml_static PUBLIC
        litehtml/include
        litehtml/include/litehtml
        litehtml/src/gumbo/include
        litehtml/src/gumbo/include/gumbo
)


# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(
        nativeexample
        # List libraries link to the target library
        android
        log
)

target_link_libraries(
        southern_litehtml
        # List libraries link to the target library
        android
        log
        ${jnigraphics-lib}
        gumbo
        litehtml_static
)