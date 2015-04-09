#include "Scene.h"

void Scene::Draw(const Window& window)
{
	const mat4 worldmatrix(
		1.0f, 0.0f, 0.0f, 0.0f, // x-axis is pointing to the right
		0.0f, 1.0f, 0.0f, 0.0f, // y-axis is pointing up
		0.0f, 0.0f,-1.0f, 0.0f, // and z-axis is pointing to the front of us
		0.0f, 0.0f, 0.0f, 1.0f
		);

	// Enable depth test
	OPENGL_CALL(glDepthFunc(GL_LESS));
	// Clear buffer
	OPENGL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// Get view projection matrix
	mat4 vpmatrix = activeCamera->GetProjectionMatrix() * worldmatrix * activeCamera->GetViewMatrix();
	mat4 mvpmatrix;

	//
	// Render terrain
	//
	mvpmatrix = vpmatrix * terrain.GetModelMatrix();
	//load model-view-projection matrix to GPU memory as uniform
	glUniformMatrix4fv(
		glGetUniformLocation(window.program.program, "MVPmatrix"),
		1,			//we have only one matrix calculated by CPU. You can easily shift these calculations to GPU
		GL_FALSE,	//we want to use traditional matrix arithmetic so we needn't to transpose matrices
		value_ptr(mvpmatrix) //Our matrix
		);
	//load wireframe thickness to GPU memory as uniform
	glUniform1fv(
		glGetUniformLocation(window.program.program, "wireframeThickness"),
		1,
		&wireframeThickness
		);
	//load wireframe color to GPU memory as uniform
	glUniform3fv(
		glGetUniformLocation(window.program.program, "wireframeColor"),
		1,
		&wireframeColor[0]
		);
	OPENGL_CHECK_FOR_ERRORS();

	//draw elements of the terrain
	//currently loaded shader program will process all of these elements
	DrawTerrainNode(
		window, terrain.heightmap.Heap()
		);

	// swap buffers and show result on the screen
	window.SwapBuffers();

	OPENGL_CHECK_FOR_ERRORS();
}

void Scene::DrawTerrainNode(
	const Window& window, const QuadTree<TerrainNode>::Iterator& node) const
{
	if (node->enabled)
	{
		if (terrain.showSurface)
		{
			glBindVertexArray(node->vaoID);
			int sparse_bits = 0;
			for (int i = 0; i < QTREE_NEIGHBOURS_COUNT; i++)
			if (node.Neighbour(i) && node.Neighbour(i).Parent() && node.Neighbour(i).Parent()->enabled)
				sparse_bits |= (1 << i);
			int offset = terrain.lodResolution * terrain.lodResolution * 6 * sparse_bits * sizeof(uint32);
			glDrawElements(GL_TRIANGLES, terrain.GetIndicesBufferSize(sparse_bits), GL_UNSIGNED_INT, (GLvoid*)offset); // draw colored surface
		}
	}
	else
	{
		DrawTerrainNode(window, node.Child(1));
		DrawTerrainNode(window, node.Child(0));
		DrawTerrainNode(window, node.Child(3));
		DrawTerrainNode(window, node.Child(2));
	}
}
