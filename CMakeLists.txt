cmake_minimum_required(VERSION 3.8.2)
project(shapemaptool)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package( TinyXML2 REQUIRED)
find_package( nlohmann_json 3.2.0 REQUIRED)
set(LIBS ${LIBS} ${TinyXML2_LIBRARIES} ${nlohmann_json_LIBRARIES})
set(SOURCES
  main.cpp
)
include_directories(shapemaptool PUBLIC include ${nlohmann_json_INCLUDE_DIRS} ${TinyXML2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR})
add_executable(shapemaptool ${SOURCES})
target_link_libraries(shapemaptool PRIVATE  ${LIBS} nlohmann_json::nlohmann_json)
