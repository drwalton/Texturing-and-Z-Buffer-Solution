#include <vector>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model* model = nullptr;
const int width = 1080;
const int height = 1080;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
	bool steep = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		steep = true;
	}
	if (p0.x > p1.x) {
		std::swap(p0, p1);
	}

	for (int x = p0.x; x <= p1.x; x++) {
		float t = (x - p0.x) / (float)(p1.x - p0.x);
		int y = p0.y*(1. - t) + p1.y*t;
		if (steep) {
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
	}
}

float areaOfTriangle(const Vec2f& a, const Vec2f& b, const Vec2f& c)
{
	Vec2f side0 = b - a;
	Vec2f side1 = c - a;
	float cross = side0.x * side1.y - side0.y * side1.x;
	return fabsf(0.5f * cross);
}

Vec2f vecToFloat(const Vec2i& v)
{
	return Vec2f(v.x, v.y);
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, 
	Vec2f tex0, Vec2f tex1, Vec2f tex2, 
	TGAImage& image, const TGAImage& texture, float intensity) {
	// Grab floating-point copies of the vertex positions *before* they are sorted
	// and their order is scrambled relative to the texture coordinates.
	Vec2f t0f = vecToFloat(t0), t1f = vecToFloat(t1), t2f = vecToFloat(t2);

	if (t0.y == t1.y && t0.y == t2.y) return; // Ignore degenerate triangles 
	// sort the vertices, t0, t1, t2 lower−to−upper (bubblesort yay!) 
	if (t0.y > t1.y) std::swap(t0, t1);
	if (t0.y > t2.y) std::swap(t0, t2);
	if (t1.y > t2.y) std::swap(t1, t2);
	int total_height = t2.y - t0.y;
	for (int i = 0; i <= total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here 
		Vec2i A = t0 + (t2 - t0) * alpha;
		Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		if (A.x > B.x) std::swap(A, B);
		for (int j = floor(A.x); j <= ceil(B.x); j++) {
			TGAColor color;
			// Texture mapping code.
			// Work out barycentric coordinates based on triangle areas
			Vec2f p(j, t0.y + i);
			float mainTriangleArea = areaOfTriangle(t0f, t1f, t2f);

			// Barycentric coordinates
			float b0 = fmaxf(fminf(areaOfTriangle(p, t1f, t2f) / mainTriangleArea, 1.0f), 0.0f);
			float b1 = fmaxf(fminf(areaOfTriangle(p, t0f, t2f) / mainTriangleArea, 1.0f), 0.0f);
			float b2 = 1.0f - (b0 + b1);

			Vec2f texCoord = b0 * tex0 + b1 * tex1 + b2 * tex2;

			Vec2i pixCoord(texCoord.x * texture.get_width(), (1.f - texCoord.y) * texture.get_height());

			// *** I've included some code I used to debug this application - it can be helpful to show
			// intermediate values like this to see if something is going wrong ***

			// Debugging - try showing barycentric coords.
			//TGAColor baryColor(b0 * 255, b1 * 255, b2 * 255, 0);
			//image.set(j, t0.y + i, baryColor); // attention, due to int casts t0.y+i != A.y 
			//continue;

			// Debugging - try showing tex coords.
			//TGAColor texColor(texCoord.x * 255, texCoord.y * 255, 0, 0);
			//image.set(j, t0.y + i, texColor); // attention, due to int casts t0.y+i != A.y 
			//continue;

			color = texture.get(pixCoord.x, pixCoord.y);
			color.r *= intensity;
			color.g *= intensity;
			color.b *= intensity;
		
			image.set(j, t0.y + i, color); // attention, due to int casts t0.y+i != A.y 
		}
	}
}

int main(int argc, char** argv) {
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("../cc_t.obj");
	}
	
	TGAImage image(width, height, TGAImage::RGB);
	TGAImage texture;
	texture.read_tga_file("../chibiCarlo.tga");

	Vec3f light_dir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		std::vector<int> tface = model->tface(i);
		Vec2i screen_coords[3];
		Vec3f world_coords[3];
		Vec2f tex_coords[3];
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
			world_coords[j] = v;
			tex_coords[j] = model->tex(tface[j]);
		}
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) {
			triangle(screen_coords[0], screen_coords[1], screen_coords[2], tex_coords[0], tex_coords[1], tex_coords[2], image, texture, intensity);
		}
	}

	image.flip_vertically(); // Origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}
