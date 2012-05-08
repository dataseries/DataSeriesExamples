#
# (c) Copyright 2012, Hewlett-Packard Development Company, LP
#
#  See the file named COPYING for license details
#
# Main cmake control file
#
# Configure with cmake -DCMAKE_INSTALL_PREFIX=$BUILD_OPT [<path-to-source>]
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)

PROJECT(TriangleFind)

SET(CMAKE_MODULE_PATH "${CMAKE_INSTALL_PREFIX}/share/cmake-modules" ${CMAKE_MODULE_PATH})

SET(DATASERIES_FIND_REQUIRED TRUE)
INCLUDE(FindDataSeries)
INCLUDE(FindLibXml2)
INCLUDE(LintelFind)

INCLUDE_DIRECTORIES(${DATASERIES_INCLUDE_DIR}
    ${LIBXML2_INCLUDE_DIR}
)

ADD_EXECUTABLE(triangle-find-ds triangle-find-ds.cpp)

ADD_EXECUTABLE(terence terence.c)

ADD_EXECUTABLE(terence2 terence2.c)

LINTEL_WITH_LIBRARY(TCMALLOC stdio.h tcmalloc_minimal)

TARGET_LINK_LIBRARIES(triangle-find-ds ${DATASERIES_LIBRARIES}) # ${TCMALLOC_LIBRARY})

INSTALL(TARGETS triangle-find-ds DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENABLE_TESTING()
ADD_TEST(few-edges ${CMAKE_CURRENT_SOURCE_DIR}/check-few-edges.sh)
