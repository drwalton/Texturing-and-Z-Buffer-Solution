#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) throw std::runtime_error("Couldn't load file!");
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "vt")) {
            iss >> trash;
            iss >> trash;
            Vec2f vt;
            for (int i=0;i<2;i++) iss >> vt.raw[i];
            texs_.push_back(vt);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f, tf;
            int itrash, idx, tidx;
            iss >> trash;
            while (iss >> idx >> trash >> tidx >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                tidx--;
                tf.push_back(tidx);
            }
            faces_.push_back(f);
            tfaces_.push_back(tf);
        }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::tface(int idx) {
    return tfaces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::tex(int i) {
    return texs_[i];
}

