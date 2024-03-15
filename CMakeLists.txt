cmake_minimum_required(VERSION 3.6)

project(MeshWireframeDrawer LANGUAGES CXX)

add_executable(Texturing-and-Z-Buffer
    main.cpp

    model.cpp
    model.h

    tgaimage.cpp
    tgaimage.h

    geometry.h
)