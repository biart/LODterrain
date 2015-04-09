/*
	Terrain class
	Defines 3D-landscape
*/

#ifndef TERRAIN_H
#define TERRAIN_H

#include "Common.h"
#include "Camera.h"
#include "DenseQuadTree.h"
#include "TGALoader.h"
#include <vector>

#define TERRAIN_GRID_SPARSE_UPPER 8
#define TERRAIN_GRID_SPARSE_RIGHT 4
#define TERRAIN_GRID_SPARSE_LOWER 2
#define TERRAIN_GRID_SPARSE_LEFT 1

#define DEFAULT_LOD_RESOLUTION 32
#define DEFAULT_LOD_MAXIMUM 6

#pragma once

struct TerrainNode
{
public:
	GLuint vaoID = NULL;
	GLuint vboID[2];

	vec2 heights;

	bool enabled;
};

class Terrain
{
public:
	//Constructor and destructor
	Terrain(int lodRes = DEFAULT_LOD_RESOLUTION, int maxLOD = DEFAULT_LOD_MAXIMUM);
	~Terrain(void);
	
	//Get model matrix
	mat4 GetModelMatrix() const;
	//Load heightmap from image file
	bool LoadFromFile(const string& filename);

	//Position, orientation and scale in 3D-space
	vec3 position;
	vec3 orientation;
	vec3 scale;

	int lodResolution;
	int maxLOD;

	float GetMorphFactor(Camera* cam);
	int GetHmapResolution() const
	{
		return lodResolution * pow(2, maxLOD) + 1;
	}
	GLuint GetIndicesBufferID() const { return indicesBufferID; }
	int GetIndicesBufferSize(int i) const { return indicesBufferSize[i]; }
	void Renew(const vec3& viewpoint) 
	{ 
		EnableNodes(heightmap.Heap()); 
		RenewNodes(viewpoint, heightmap.Heap()); 
	}
	void Unload();

	//Show grid
	bool showGrid;
	//Show surface
	bool showSurface;
	QuadTree<TerrainNode> heightmap;

private:
	GLuint indicesBufferID; //VBO for 16 sets of indices
	int indicesBufferSize[16];
	//Generate sixteen versions of index arrays for each case of sparse/dense egdes
	void GenerateIndices();

	//Load all nodes data to GPU recursively
	vec2 LoadVertices(
		const QuadTree<TerrainNode>::Iterator& node,
		const Image& hmap
		);

	//Unload all nodes data from GPU recursively
	void UnloadVertices(const QuadTree<TerrainNode>::Iterator& node);
	//Determine which nodes must be rendered
	void RenewNodes(const vec3& viewpoint, const QuadTree<TerrainNode>::Iterator& node);
	//Set some neighbour nodes disabled to avoid too big difference in detalization levels
	void DisableNodes(const QuadTree<TerrainNode>::Iterator& node);
	//Set all nodes enabled
	void EnableNodes(const QuadTree<TerrainNode>::Iterator& node);
};

struct TerrainGeneratorNode
{
	float heights[4];
};

class TerrainGenerator
{
public:
	int resolution;
	string outputdir;
	void Generate(uvec2 a, uvec2 b);

private:
	Array2D<float> LDHeights;
	Array2D<float> HDHeights;

};

template<typename T>
inline T mean(const T& a, const T& b)
{
	return (a + b) / T(2);
}

/*
void TerrainGenerator::Generate(uvec2 a, uvec2 c)
{
	float offset = (a.x - c.x) / float(LDHeights.GetSize().x);
	uvec2 p[5] = { a, uvec2(c.x, a.y), c, uvec2(a.x, c.y), a };
	float centralHeight = 0.0f;
	uvec2 central;
	for (int i = 0; i < 4; i++)
	{
		centralHeight += LDHeights[mean(p[i], p[i + 1])] =
			mean(LDHeights[p[i]], LDHeights[p[i + 1]]) +
			glm::linearRand(-offset, +offset);
		central += p[i];
	}
	LDHeights[central / uvec2(4)] = centralHeight / 
		mean(LDHeights[p[i]], LDHeights[p[i + 1]]) +
		glm::linearRand(-offset, +offset);
}
*/

#endif //TERRAIN_H