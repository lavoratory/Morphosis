cmake_minimum_required(VERSION 3.20)

project(MorphosisSpray
    VERSION 1.1.0
    LANGUAGES CXX
)

if(NOT APPLE AND NOT WIN32)
    message(FATAL_ERROR "MorphosisSpray currently supports macOS and Windows only.")
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment target")
endif()

if(APPLE)
    if(NOT MORPHOSIS_RESOLUME_PLUGIN_DIR)
        set(MORPHOSIS_RESOLUME_PLUGIN_DIR "$ENV{HOME}/Documents/Resolume Arena/Extra Effects" CACHE PATH "Resolume plugin install directory")
    endif()
elseif(WIN32)
    if(NOT MORPHOSIS_RESOLUME_PLUGIN_DIR)
        set(MORPHOSIS_RESOLUME_PLUGIN_DIR "$ENV{USERPROFILE}/Documents/Resolume Arena/Extra Effects" CACHE PATH "Resolume plugin install directory")
    endif()
endif()

add_library(MorphosisSpray MODULE
    source/MorphosisSpray.cpp
    source/MorphosisSpray.h
    deps/ffgl/source/lib/FFGLSDK.cpp
)

target_include_directories(MorphosisSpray PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/ffgl/source/lib"
)

target_compile_features(MorphosisSpray PRIVATE cxx_std_17)
if(APPLE)
    target_compile_definitions(MorphosisSpray PRIVATE TARGET_OS_MAC=1)
elseif(WIN32)
    target_compile_definitions(MorphosisSpray PRIVATE TARGET_OS_WIN=1 GLEW_STATIC)
endif()
if(MSVC)
    target_compile_options(MorphosisSpray PRIVATE /W4 /wd4996 /wd4100 /wd4189)
else()
    target_compile_options(MorphosisSpray PRIVATE
        -Wall
        -Wextra
        -Wno-deprecated-declarations
        -Wno-unused-parameter
    )
endif()

if(APPLE)
    target_link_libraries(MorphosisSpray PRIVATE
        "-framework OpenGL"
        "-framework Carbon"
        "-framework AppKit"
    )

    set_target_properties(MorphosisSpray PROPERTIES
        PREFIX ""
        BUNDLE TRUE
        BUNDLE_EXTENSION "bundle"
        OUTPUT_NAME "MorphosisSpray"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "MorphosisSpray"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.morphosis.ffgl.MorphosisSpray"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.morphosis.ffgl.MorphosisSpray"
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "bundle"
    )
elseif(WIN32)
    set(GLEW_USE_STATIC_LIBS ON)
    find_package(GLEW REQUIRED)

    target_link_libraries(MorphosisSpray PRIVATE
        GLEW::GLEW
        opengl32
        gdi32
        user32
    )

    set_target_properties(MorphosisSpray PROPERTIES
        PREFIX ""
        OUTPUT_NAME "MorphosisSpray"
    )
endif()

install(TARGETS MorphosisSpray
    BUNDLE DESTINATION "${MORPHOSIS_RESOLUME_PLUGIN_DIR}"
    LIBRARY DESTINATION "${MORPHOSIS_RESOLUME_PLUGIN_DIR}"
    RUNTIME DESTINATION "${MORPHOSIS_RESOLUME_PLUGIN_DIR}"
)
