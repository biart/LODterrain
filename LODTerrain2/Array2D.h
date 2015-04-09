#include <vector>
#include "Common.h"

#ifndef ARRAY2D_H
#define ARRAY2D_H

template<typename T>
class Array2D
{
public:
	//Constructor
	Array2D(uvec2 size, const T& elem) : dims(size) 
	{
		data = vector<T>(size.x * size.y, elem);
	}
	Array2D(uvec2 size = uvec2(0)) : dims(size)
	{
		data = vector<T>(size.x * size.y);
	}
	Array2D(Array2D&& that)
	{
		dims = move(that.dims);
		data = move(that.data);
	}
	Array2D& operator=(Array2D&& that)
	{
		dims = move(that.dims);
		data = move(that.data);
		return *this;
	}
	//Operator bool() determining if array is not empty
	operator bool()
	{
		return dims.x * dims.y > 0;
	}
	//Indexing operators
	T& operator[](const ivec2& coords)
	{
		int i = (coords.x % dims.x) * dims.y + coords.y % dims.y;
		return data[i];
	}
	T& operator[](const uvec2& coords)
	{
		int i = (coords.x % dims.x) * dims.y + coords.y % dims.y;
		return data[i];
	}
	const T& operator[](const ivec2& coords) const
	{
		int i = (coords.x % dims.x) * dims.y + coords.y % dims.y;
		return data[i];
	}
	const T& operator[](const uvec2& coords) const
	{
		int i = (coords.x % dims.x) * dims.y + coords.y % dims.y;
		return data[i];
	}
	T& At(unsigned x, unsigned y)
	{
		int i = (x % dims.x) * dims.y + y % dims.y;
		return data[i];
	}
	const T& At(unsigned x, unsigned y) const
	{
		int i = (x % dims.x) * dims.y + y % dims.y;
		return data[i];
	}

	//Clear data
	void Clear() { dims = uvec2(0); data.clear() }

	//Add row or column
	void AddRows(unsigned n, const T& elem = T())
	{
		dims.x += n;
		data.resize(dims.x * dims.y, elem);
	}
	void AddColumns(unsigned n, const T& elem = T())
	{
		data.resize(dims.x * (dims.y + n), elem);
		for (int i = dims.x - 1; i > 0; i--)
		for (int j = dims.y - 1; j >= 0; j--)
		{
			swap(data[i*dims.y + j], data[i*(dims.y + n) + j]);
		}
		dims.y += n;
	}
	void RemoveRows(unsigned n)
	{
		dims.x = max(dims.x - n, 0);
		data.resize(dims.x * dims.y);
	}
	void RemoveColumns(unsigned n)
	{
		int newY = max(dims.y - n, 0);
		for (int i = 1; i < dims.x; i++)
		for (int j = 0; j < newY; j++)
		{
			data[i*newY + j] = data[i*dims.y + j];
		}
		dims.y = newY;
		data.resize(dims.x * dims.y);
	}
	//Change dimensions
	void Resize(uvec2 newsize)
	{
		dims = newsize;
		data.resize(dims.x * dims.y);
	}
	void Reshape(uvec2 newsize, const T& elem = T())
	{
		if (newsize.y > dims.y)
			AddColumns(newsize.y - dims.y);
		else if (newsize.y < dims.y)
			RemoveColumns(dims.y - newsize.y);
		dims.x = newsize.x;
		data.resize(dims.x * dims.y);
	}
	//Fill
	void Fill(const T& element)
	{
		for (i : data) i = element;
	}

	//Getters
	const T* GetRawPointer() const { return data.data(); }
	const vector<T>& GetPlainData() const { return data; }
	uvec2 GetSize() const { return dims; }
	int GetElementsCount() const { return dims.x * dims.y; }

private:
	//Size
	uvec2 dims;
	//One-dimentional data vector
	vector<T> data;
};

#endif // ARRAY2D_H