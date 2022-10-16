/*
    Bio-Game is a game for designing your own microorganism. 
    Copyright (C) 2022 John Engel 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <cglm/cglm.h>
#include <glad/glad.h>
#include "window.h"
#include "graphics.h"

B_Model B_create_triangle()
{
	B_Model triangle;
	B_Mesh mesh;
	memset(triangle.meshes, 0, sizeof(B_Mesh)*MAX_MESHES);
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		triangle.meshes[i].active = 0;
	}
	B_Vertex vertices[] = { { {-1.0, -1.0, 0.0}, { 0.0, 0.0, -1.0}, { 0.0, 0.0, 0.0} },
		 	      { {1.0, -1.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0, 0.0} },
			      { {0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 0.0, 0.0} } };
	mesh.active = 1;
	mesh.num_vertices = 3;
	mesh.vertices = malloc(mesh.num_vertices * sizeof(B_Vertex));
	mesh.vertices = memcpy(mesh.vertices, vertices, mesh.num_vertices*sizeof(B_Vertex));
	glGenVertexArrays(1, &mesh.vao);
	glBindVertexArray(mesh.vao);
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh.num_vertices*sizeof(B_Vertex), mesh.vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(B_Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(B_Vertex), (void*)sizeof(B_Vertex));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(B_Vertex), (void*)(sizeof(B_Vertex)*2));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	triangle.meshes[0] = mesh;
	return triangle;
}
/*
B_Model B_create_cube()
{
	B_Model cube;
	B_Vertex vertices[] = 
	{
	     { { {-0.5f, -0.5f, -0.5f},   {0.0f,  0.0f, -1.0f} , { 0, 0, 0} },
	       { {0.5f, -0.5f, -0.5f } ,  {0.0f,  0.0f, -1.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f, -0.5f} ,  {0.0f,  0.0f, -1.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f, -0.5f} ,  {0.0f,  0.0f, -1.0f} , { 0, 0, 0} },
	       { {-0.5f,  0.5f, -0.5f} ,  {0.0f,  0.0f, -1.0f} , { 0, 0, 0} },
	       { {-0.5f, -0.5f, -0.5f} ,  {0.0f,  0.0f, -1.0f} , { 0, 0, 0} },
	
	       { {-0.5f, -0.5f,  0.5f} ,  {0.0f,  0.0f, 1.0f}  , { 0, 0, 0} }, 
	       { { 0.5f, -0.5f,  0.5f} ,  {0.0f,  0.0f, 1.0f}  , { 0, 0, 0} }, 
	       { { 0.5f,  0.5f,  0.5f} ,  {0.0f,  0.0f, 1.0f}  , { 0, 0, 0} },
	       { { 0.5f,  0.5f,  0.5f} ,  {0.0f,  0.0f, 1.0f}  , { 0, 0, 0} },
	       { {-0.5f,  0.5f,  0.5f} ,  {0.0f,  0.0f, 1.0f}  , { 0, 0, 0} },
	       { {-0.5f, -0.5f,  0.5f} ,  {0.0f,  0.0f, 1.0f}  , { 0, 0, 0} },
	
	       { {-0.5f,  0.5f,  0.5f} , -{1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f,  0.5f, -0.5f} , -{1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f, -0.5f, -0.5f} , -{1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f, -0.5f, -0.5f} , -{1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f, -0.5f,  0.5f} , -{1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f,  0.5f,  0.5f} , -{1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	
	       { { 0.5f,  0.5f,  0.5f} ,  {1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f, -0.5f} ,  {1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f, -0.5f, -0.5f} ,  {1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f, -0.5f, -0.5f} ,  {1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f, -0.5f,  0.5f} ,  {1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f,  0.5f} ,  {1.0f,  0.0f,  0.0f} , { 0, 0, 0} },
	
	       { {-0.5f, -0.5f, -0.5f} ,  {0.0f, -1.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f, -0.5f, -0.5f} ,  {0.0f, -1.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f, -0.5f,  0.5f} ,  {0.0f, -1.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f, -0.5f,  0.5f} ,  {0.0f, -1.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f, -0.5f,  0.5f} ,  {0.0f, -1.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f, -0.5f, -0.5f} ,  {0.0f, -1.0f,  0.0f} , { 0, 0, 0} },
	
	       { {-0.5f,  0.5f, -0.5f} ,  {0.0f,  1.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f, -0.5f} ,  {0.0f,  1.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f,  0.5f} ,  {0.0f,  1.0f,  0.0f} , { 0, 0, 0} },
	       { { 0.5f,  0.5f,  0.5f} ,  {0.0f,  1.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f,  0.5f,  0.5f} ,  {0.0f,  1.0f,  0.0f} , { 0, 0, 0} },
	       { {-0.5f,  0.5f, -0.5f} ,  {0.0f,  1.0f,  0.0f} , { 0, 0, 0} }
	};
	glGenVertexArrays(1, &cube.vao);
	glGenBuffers(1, &cube.vbo);
}
*/

void B_blit_model(B_Model model, B_Shader shader)
{
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		if (model.meshes[i].active)
		{
			glBindVertexArray(model.meshes[i].vao);
			glUseProgram(shader);
			vec4 color = {0.0f, 1.0f, 0.0f, 1.0f};
			B_set_uniform_vec4(shader, "color", color);
			glDrawArrays(GL_TRIANGLES, 0, model.meshes[i].num_vertices);
		}
	}
}

void B_free_model(B_Model model)
{
	for (int i = 0; i < MAX_MESHES; ++i)
	{
		if(model.meshes[i].active)
		{	
			//glDeleteBuffers(1, &model.meshes[i].ebo);
			glDeleteBuffers(1, &model.meshes[i].vbo);
		}
		else
		{
			break;
		}

	}
}

void B_set_uniform_vec4(B_Shader shader, char *name, vec4 value)
{
	glUseProgram(shader);
	glUniform4f(glGetUniformLocation(shader, name), value[0], value[1], value[2], value[3]);
}
