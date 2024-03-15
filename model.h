#pragma once

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> texs_;
	std::vector<std::vector<int> > faces_, tfaces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec2f tex(int i);
	std::vector<int> face(int idx);
	std::vector<int> tface(int idx);
};

