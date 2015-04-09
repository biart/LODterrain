#include "glm\glm.hpp"
#include <vector>
#pragma once

using namespace glm;

template<typename T>
class Matrix
{
public:
	Matrix() : dims(0, 0) {};
	Matrix(unsigned int x, unsigned int y) : dims(x, y), data(x * y) {};
	Matrix(uvec2 d) : dims(d), data(d.x * d.y) {};
	~Matrix() {};

	T& operator[](uvec2 index)
	{
		if (index.x > dims.x || index.y > dims.y)
			throw std::out_of_range;
		return data[index.x * dims.x + dims.y];
	}
	const T& operator[](uvec2 index) const
	{
		if (index.x > dims.x || index.y > dims.y)
			throw std::out_of_range;
		return data[index.x * dims.x + dims.y];
	}
	Resize(uvec2 newdims)
	{
		dims = newdims;
		data.resize(dims.x * dims.y);
	}
private:
	std::vector<T> data;
	uvec2 dims;
};

