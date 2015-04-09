/*
	Image class
	Class for processing images
	Can read only uncompressed TGA format yet
*/
#define _CRT_SECURE_NO_WARNINGS

#ifndef TGALOADER_H
#define TGALOADER_H

#include "Array2D.h"

class Image : public Array2D<vec3>
{
public:
	//Image constructors
	Image() {}
	Image(const string& filename) { Load(filename); }

	//Interpolation index operator
	vec3 operator[](vec2 index) const
	{
		float x = index.x * (GetSize().x - 1), y = index.y * (GetSize().y - 1);
		int i = static_cast<int>(x), j = static_cast<int>(y);
		x -= static_cast<float>(i);	y -= static_cast<float>(j);
		return 
			At(i, j) * (1.0f - x) * (1.0f - y) +
			At(i, j + 1) * (1.0f - x) * y +
			At(i + 1, j) * x * (1.0f - y) +
			At(i + 1, j + 1) * x * y;
	}

	//Basic file operations:
	// load from file or save to file
	bool Load(const string& filename);
	bool Save(const string& filename) const;

    const int uncompressedColor = 2;
    const int uncompressedGreyscale = 3;
};

#endif // TGALOADER_H