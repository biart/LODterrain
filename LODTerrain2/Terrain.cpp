#include "Terrain.h"

//Auxiliary functions
inline vec2 UniteSegments(const vec2& a, const vec2& b)
{
	return vec2(min(a.x, b.x), max(a.x, b.x));
}

Terrain::Terrain(int lodRes, int maxLevel)
{
	if (maxLevel < 1)
		throw invalid_argument("Invalid level of details. It must be 1 or higher.");
	lodResolution = lodRes;
	maxLOD = maxLevel;
	scale = vec3(1.0f);
	position = orientation = vec3(0.0f);
	showGrid = showSurface = true;
}

Terrain::~Terrain(void) {}

bool Terrain::LoadFromFile(const string& filename)
{
	//Unload previous terrain, if exists
	Unload();

	WriteToLog("Loading heightmap from TGA file...\n");
	Image img;
	if (!img.Load(filename))
	{
		WriteToLog("ERROR: Failed to load heightmap.\n");
		return false;
	}

	//Load neccessary data to GPU
	WriteToLog("Generating indices...\n");
	GenerateIndices();
	WriteToLog("Loading all vertex data to VAO...\n");
	LoadVertices(heightmap.Heap(), img);
	WriteToLog("OK: Terrain was loaded\n");
	return true;
}

mat4 Terrain::GetModelMatrix() const
{
	mat4 mmatrix = translate(position);
	mmatrix *= rotate(mat4(1.0f), orientation.z, vec3(0.0f, 0.0f, 1.0f));
	mmatrix *= rotate(mat4(1.0f), orientation.y, vec3(0.0f, 1.0f, 0.0f));
	mmatrix *= rotate(mat4(1.0f), orientation.x, vec3(1.0f, 0.0f, 0.0f));
	return glm::scale(mmatrix, scale);
}

vec2 Terrain::LoadVertices(
	const QuadTree<TerrainNode>::Iterator& node, 
	const Image& hmap
	)
{
	//Setup vertex data
	vec2 res = vec2(1.0f, 0.0f);
	int verticesCount = (lodResolution + 1) * (lodResolution + 1);
	vector<vec3> vertices(verticesCount), colors(verticesCount);
	float deltaX = 1.0f / node.LayerSize() / lodResolution;
	float deltaY = 1.0f / node.LayerSize() / lodResolution;
	float x = node.OffsetFloat().x;
	for (int i = 0; i <= lodResolution; i++, x += deltaX)
	{
		float y = node.OffsetFloat().y;
		for (int j = 0; j <= lodResolution; j++, y += deltaY)
		{
			float h = hmap[vec2(x, y)].x;
			res = UniteSegments(res, vec2(h));

			vertices[i*lodResolution + i + j] = vec3(x, h, y);
			colors[i*lodResolution + i + j] = vec3(0.2f, 0.2f + h, 0.4f - h);
		}
	}

	// VAO allocation
	glGenVertexArrays(1, &node->vaoID);
	// VAO setup
	glBindVertexArray(node->vaoID);
	// VBOs allocation
	glGenBuffers(2, node->vboID);
	// VBOs setup
	// vertices buffer
	glBindBuffer(GL_ARRAY_BUFFER, node->vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * 3 * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// colors buffer
	glBindBuffer(GL_ARRAY_BUFFER, node->vboID[1]);
	glBufferData(GL_ARRAY_BUFFER, colors.size() * 3 * sizeof(GLfloat), colors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(1);
	// indices buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GetIndicesBufferID());

	OPENGL_CHECK_FOR_ERRORS();
	// release vertex data
	vertices.clear();
	colors.clear();

	if (node.Level() < maxLOD)
	{
        for (int i : {0, 1, 2, 3})
            res = UniteSegments(res, LoadVertices(node.Add(i), hmap));
	}
	node->heights = res;

	return res;
}

void Terrain::UnloadVertices(const QuadTree<TerrainNode>::Iterator& node)
{
	if (node->vaoID)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(2, node->vboID);
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &node->vaoID);
		node->vaoID = NULL;
	}

	for (int i = 0; i < QTREE_CHILDREN_COUNT; i++)
		if (node.Child(i))
			UnloadVertices(node.Child(i));
}

void Terrain::Unload()
{
	UnloadVertices(heightmap.Heap());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &indicesBufferID);
	WriteToLog("OK: Terrain was unloaded\n");
}

void Terrain::GenerateIndices()
{
	const int VBOSize = lodResolution * lodResolution * 6 * 16;
	vector<GLuint> indices(VBOSize);
	vector<GLuint>::iterator ptr;
	int i, u, v, count;
	for(i = 0; i < 16; i++)
	{
		ptr = indices.begin() + lodResolution * lodResolution * 6 * i;
		count = 0;
		//
		//UPPER SIDE
		//
		if(i & TERRAIN_GRID_SPARSE_UPPER)
		{
			v = 0;
			ptr[count++] = v;
			ptr[count++] = v + lodResolution + 2;
			ptr[count++] = v + 2;
 
			ptr[count++] = v + 2;
			ptr[count++] = v + lodResolution + 2;
			ptr[count++] = v + lodResolution + 3;

			for (v = 2; v < lodResolution-2 ; v+=2)
			{
				ptr[count++] = v;
				ptr[count++] = v + lodResolution + 1;
				ptr[count++] = v + lodResolution + 2;
 
				ptr[count++] = v;
				ptr[count++] = v + lodResolution + 2;
				ptr[count++] = v + 2;
 
				ptr[count++] = v + 2;
				ptr[count++] = v + lodResolution + 2;
				ptr[count++] = v + lodResolution + 3;
			}

			ptr[count++] = v;						
			ptr[count++] = v + lodResolution + 1;	
			ptr[count++] = v + lodResolution + 2;	
 
			ptr[count++] = v;						
			ptr[count++] = v + lodResolution + 2;	
			ptr[count++] = v + 2;					
		}
		else
		{
			v = 0;
			ptr[count++] = v;						
			ptr[count++] = v + 2 + lodResolution;	
			ptr[count++] = v + 1;					

			for (v = 1; v < lodResolution-1 ; v++)
			{
				ptr[count++] = v;						
				ptr[count++] = v + 2 + lodResolution;	
				ptr[count++] = v + 1;					
 
				ptr[count++] = v;						
				ptr[count++] = v + lodResolution + 1;	
				ptr[count++] = v + lodResolution + 2;	
			}

			ptr[count++] = v;						
			ptr[count++] = v + 1 + lodResolution;	
			ptr[count++] = v + 1;					
		}
		//
		//RIGHT SIDE
		//
		if(i & TERRAIN_GRID_SPARSE_RIGHT)
		{
			u = 0;
			unsigned int aux = u*(lodResolution+1) + lodResolution;
			ptr[count++] = aux;							  
			ptr[count++] = aux + lodResolution;			
			ptr[count++] = aux + 2*lodResolution + 2;	
 
			ptr[count++] = aux + lodResolution;			
			ptr[count++] = aux + 2*lodResolution + 1;	
			ptr[count++] = aux + 2*lodResolution + 2;	

			for (u = 2; u < lodResolution-2 ; u+=2)
			{
				aux = u*(lodResolution+1) + lodResolution;
				ptr[count++] = aux;
				ptr[count++] = aux - 1;						
				ptr[count++] = aux + lodResolution;			
 
				ptr[count++] = aux;
				ptr[count++] = aux + lodResolution;			
				ptr[count++] = aux + 2*lodResolution + 2;	
 
				ptr[count++] = aux + lodResolution;			
				ptr[count++] = aux + 2*lodResolution + 1;	
				ptr[count++] = aux + 2*lodResolution + 2;	
			}

			aux = u*(lodResolution+1) + lodResolution;
			ptr[count++] = aux;  
			ptr[count++] = aux - 1;						
			ptr[count++] = aux + lodResolution;			
 
			ptr[count++] = aux; 
			ptr[count++] = aux + lodResolution;			
			ptr[count++] = aux + 2*lodResolution + 2;	
		}
		else
		{
			u = 0;
			unsigned int aux = u*(lodResolution+1) + lodResolution;
			ptr[count++] = aux;
			ptr[count++] = aux + lodResolution;			
			ptr[count++] = aux + lodResolution + 1;		

			for (u = 1; u < lodResolution-1 ; u++)
			{
				aux = u*(lodResolution+1) + lodResolution;
				ptr[count++] = aux;							   
				ptr[count++] = aux - 1;						
				ptr[count++] = aux + lodResolution;			
 
				ptr[count++] = aux; 
				ptr[count++] = aux + lodResolution;			
				ptr[count++] = aux + lodResolution + 1;		
			}
			aux = u*(lodResolution+1) + lodResolution;
			ptr[count++] = aux;
			ptr[count++] = aux - 1;						
			ptr[count++] = aux + lodResolution + 1;
		}
		//
		//LOWER SIDE
		//
		if(i & TERRAIN_GRID_SPARSE_LOWER)
		{
			v = 0;
			unsigned int aux = (lodResolution-1)*(lodResolution+1) + v;
			ptr[count++] = aux + 1;					
			ptr[count++] = aux + lodResolution + 1;	
			ptr[count++] = aux + lodResolution + 3;	
 
			ptr[count++] = aux + 2;					
			ptr[count++] = aux + 1;					
			ptr[count++] = aux + lodResolution + 3;	

			for (v = 2; v < lodResolution-2 ; v+=2)
			{
				aux = (lodResolution-1)*(lodResolution+1) + v;
				ptr[count++] = aux + 1;					
				ptr[count++] = aux;						
				ptr[count++] = aux + lodResolution + 1;	
 
				ptr[count++] = aux + 1;					
				ptr[count++] = aux + lodResolution + 1;	
				ptr[count++] = aux + lodResolution + 3;	
 
				ptr[count++] = aux + 2;					
				ptr[count++] = aux + 1;					
				ptr[count++] = aux + lodResolution + 3;	
			}
			aux = (lodResolution-1)*(lodResolution+1) + v;
			ptr[count++] = aux + 1;
			ptr[count++] = aux;
			ptr[count++] = aux + lodResolution + 1;
 
			ptr[count++] = aux + 1;					
			ptr[count++] = aux + lodResolution + 1;	
			ptr[count++] = aux + lodResolution + 3;	
		}
		else
		{
			v = 0;
			unsigned int aux = (lodResolution-1)*(lodResolution+1) + v;
			ptr[count++] = aux + 1;					
			ptr[count++] = aux + lodResolution + 1;	
			ptr[count++] = aux + lodResolution + 2;	

			for (v = 1; v < lodResolution-1 ; v++)
			{
				aux = (lodResolution-1)*(lodResolution+1) + v;
				ptr[count++] = aux + 1;					
				ptr[count++] = aux;						
				ptr[count++] = aux + lodResolution + 1;	
 
				ptr[count++] = aux + 1;					
				ptr[count++] = aux + lodResolution + 1;	
				ptr[count++] = aux + lodResolution + 2;	
			}
			aux = (lodResolution - 1)*(lodResolution + 1) + v;
			ptr[count++] = aux;
			ptr[count++] = aux + lodResolution + 1;
			ptr[count++] = aux + lodResolution + 2;
		}
		//
		//LEFT SIDE
		//
		if(i & TERRAIN_GRID_SPARSE_LEFT)
		{
			u = 0;
			unsigned int aux = u*(lodResolution+1);
			ptr[count++] = aux; 
			ptr[count++] = aux + 2*lodResolution + 2;	
			ptr[count++] = aux + lodResolution + 2;		
 
			ptr[count++] = aux + lodResolution + 2;		
			ptr[count++] = aux + 2*lodResolution + 2;	
			ptr[count++] = aux + 2*lodResolution + 3;	

			for (u = 2; u < lodResolution-2 ; u+=2)
			{
				aux = u*(lodResolution+1);
				ptr[count++] = aux + 1;
				ptr[count++] = aux;						
				ptr[count++] = aux + lodResolution + 2;		
 
				ptr[count++] = aux; 
				ptr[count++] = aux + 2*lodResolution + 2;	
				ptr[count++] = aux + lodResolution + 2;		
 
				ptr[count++] = aux + lodResolution + 2;		
				ptr[count++] = aux + 2*lodResolution + 2;	
				ptr[count++] = aux + 2*lodResolution + 3;	
			}

			aux = u*(lodResolution+1);
			ptr[count++] = aux + 1;
			ptr[count++] = aux;						
			ptr[count++] = aux + lodResolution + 2;		
 
			ptr[count++] = aux; 
			ptr[count++] = aux + 2*lodResolution + 2;	
			ptr[count++] = aux + lodResolution + 2;		
		}
		else
		{
			u = 0;
			unsigned int aux = u*(lodResolution+1);
			ptr[count++] = aux; 
			ptr[count++] = aux + lodResolution + 1;		
			ptr[count++] = aux + lodResolution + 2;		

			for (u = 1; u < lodResolution-1 ; u++)
			{
				aux = u*(lodResolution+1);
				ptr[count++] = aux;
				ptr[count++] = aux + lodResolution + 2;		
				ptr[count++] = aux + 1;						
 
				ptr[count++] = aux;  
				ptr[count++] = aux + lodResolution + 1;		
				ptr[count++] = aux + lodResolution + 2;		
			}
			aux = u*(lodResolution+1);
			ptr[count++] = aux;							   
			ptr[count++] = aux + lodResolution + 1;		
			ptr[count++] = aux + 1;						
		}
		//
		//CENTRAL PART
		//
		for (u = 1; u < lodResolution-1; u++)
		{
			for (v = 1; v < lodResolution-1 ; v++)
			{
				const unsigned int aux = u*(lodResolution+1) + v;
				ptr[count++] = aux;						
				ptr[count++] = aux + 2 + lodResolution;	
				ptr[count++] = aux + 1;					
 
				ptr[count++] = aux;						
				ptr[count++] = aux + lodResolution + 1;	
				ptr[count++] = aux + lodResolution + 2;	
			}
		}
		indicesBufferSize[i] = count;
	}

	glGenBuffers(1, &indicesBufferID);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicesBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, VBOSize * sizeof(uint32), indices.data(), GL_STATIC_DRAW);

	OPENGL_CHECK_FOR_ERRORS();
}

inline float ClosestSegmentPoint(float x, float a, float b)
{
	return min(abs(x - a), abs(x - b));
}

void Terrain::DisableNodes(const QuadTree<TerrainNode>::Iterator& node)
{
	if (node->enabled)
	{
		node->enabled = false;
		if (node.Parent())
		{
			for (int i = 0; i < QTREE_NEIGHBOURS_COUNT; i++)
			if (node.Neighbour(i) && node.Neighbour(i).Parent())
			{
				DisableNodes(node.Neighbour(i).Parent());
			}
		}
	}
}

void Terrain::EnableNodes(const QuadTree<TerrainNode>::Iterator& node)
{
	node->enabled = true;
	for (int i = 0; i < QTREE_CHILDREN_COUNT; i++)
	{
		if (node.Child(i))
			EnableNodes(node.Child(i));

	}
}

void Terrain::RenewNodes(const vec3& viewpoint, const QuadTree<TerrainNode>::Iterator& node)
{
	//Check if this node must be enabled using morph-factor
	float sz = static_cast<float>(node.LayerSize());
	float l = node.Offset().x / sz, r = (node.Offset().x + 1) / sz;
	float u = node.Offset().y / sz, d = (node.Offset().y + 1) / sz;
	vec3 rel_pos = vec3(inverse(GetModelMatrix()) * vec4(viewpoint, 1.0f));
	rel_pos.x = ClosestSegmentPoint(rel_pos.x, l, r);
	rel_pos.y = ClosestSegmentPoint(rel_pos.y, node->heights.x, node->heights.y);
	rel_pos.z = ClosestSegmentPoint(rel_pos.z, u, d);
	if (length(rel_pos)*length(rel_pos) / 
		(1.0f + node->heights.y - node->heights.x) / 
		(r - l) / (d - u) > 5.0f || 
		node.Level() == maxLOD)
	{
		//This node is probably enabled
	}
	else
	{
		//This node is not enabled
		//continue checking its children
		DisableNodes(node);
		RenewNodes(viewpoint, node.Child(1));
		RenewNodes(viewpoint, node.Child(0));
		RenewNodes(viewpoint, node.Child(3));
		RenewNodes(viewpoint, node.Child(2));
	}
}
