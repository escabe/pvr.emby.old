project(pvr.emby)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR})

enable_language(CXX)

find_package(Kodi REQUIRED)
find_package(kodiplatform REQUIRED)
find_package(p8-platform REQUIRED)
find_package(Curl REQUIRED)

include_directories(${kodiplatform_INCLUDE_DIRS}
                    ${p8-platform_INCLUDE_DIRS}
                    ${KODI_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIR})

set(DEPLIBS ${kodiplatform_LIBRARIES}
            ${p8-platform_LIBRARIES}
            ${CURL_LIBRARY})



set(PVREMBY_SOURCES src/client.cpp
                    src/PVREmby.cpp)


set(PVREMBY_HEADERS src/client.h
                    src/PVREmby.h)

build_addon(pvr.emby PVREMBY DEPLIBS)

include(CPack)
