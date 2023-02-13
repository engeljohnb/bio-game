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

#include <unistd.h>
#include <cglm/cglm.h>
#include "asset_loading.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "utils.h"

char *B_get_texture_name(const char *parent_directory, const char *node_name)
{
	// First, remove all the numbers and dots -- we only need the base name.
	char new_node_name[256] = {0};
	strncpy(new_node_name, node_name, 256);
	for (size_t i = 0; i < strnlen(new_node_name, 256); ++i)
	{
		if (new_node_name[i] == '.')
		{
			new_node_name[i] = '\0';
		}
		if ((new_node_name[i] >= '0') && (new_node_name[i] <= '9'))
		{
			new_node_name[i] = '\0';
		}
	}

	char *final_name = BG_MALLOC(char, 512);
	strncpy(final_name, parent_directory, 512);
	strncat(final_name, "/", 512);
	strncat(final_name, new_node_name, 512);
	strncat(final_name, "_texture.jpg", 512);

	if (file_exists(final_name))
	{
		return final_name;
	}

	BG_FREE(final_name);
	return NULL;
}

void B_assign_all_color_textures(ActorModel *model, B_Texture texture)
{
	model->color_texture = texture;
	for (int i = 0; i < model->num_children; ++i)
	{
		B_assign_all_color_textures(model->children[i], texture);
	}
}

//TODO B_free_Texture
//TODO: Document that the texture data must be RGB.
B_Texture B_send_texture_data_to_gpu(unsigned char *pixel_data, int width, int height)
{
	B_Texture texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

B_Texture B_send_texture_to_gpu(const char *filename)
{
	B_Texture texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width = 0;
	int height = 0;
	unsigned char *pixel_data = stbi_load(filename, &width, &height, NULL, 0);

	if (pixel_data == NULL)
	{
		fprintf(stderr, "B_send_texture_to_gpu error: Could not load file %s\n", filename);
		pixel_data = BG_MALLOC(unsigned char, 16*16);
		width = 16;
		height = 16;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel_data);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (pixel_data != NULL)
	{
		stbi_image_free(pixel_data);
	}
	else
	{
		BG_FREE(pixel_data);
	}

	return texture;
}

C_STRUCT aiMatrix4x4 cglm_to_assimp_mat4(mat4 source)
{
	C_STRUCT aiMatrix4x4 dest = {0};

	dest.a1 = source[0][0];
	dest.a2 = source[1][0];
	dest.a3 = source[2][0];
	dest.a4 = source[3][0];

	dest.b1 = source[0][1];
	dest.b2 = source[1][1];
	dest.b3 = source[2][1];
	dest.b4 = source[3][1];

	dest.c1 = source[0][2];
	dest.c2 = source[1][2];
	dest.c3 = source[2][2];
	dest.c4 = source[3][2];

	dest.d1 = source[0][3];
	dest.d2 = source[1][3];
	dest.d3 = source[2][3];
	dest.d4 = source[3][3];

	return dest;
}

void assimp_to_cglm_mat4(C_STRUCT aiMatrix4x4 source, mat4 dest)
{
	mat4 mat =  { { source.a1, source.b1, source.c1, source.d1 },
		      { source.a2, source.b2, source.c2, source.d2 },
		      { source.a3, source.b3, source.c3, source.d3 },
		      { source.a4, source.b4, source.c4, source.d4 } };
	glm_mat4_copy(mat, dest);
}

void B_load_ai_mesh_iter(const C_STRUCT aiScene *scene, C_STRUCT aiNode *node, ActorModel *model)
{
	VertexData vertex_data;
	memset(&vertex_data, 0, sizeof(VertexData));
	ActorMesh *b_mesh = BG_MALLOC(ActorMesh, 1);
	C_STRUCT aiMesh *a_mesh = scene->mMeshes[node->mMeshes[0]];

	vertex_data.num_vertices = a_mesh->mNumVertices;
	b_mesh->num_vertices = a_mesh->mNumVertices;
	vertex_data.vertices = BG_MALLOC(A_Vertex, a_mesh->mNumVertices);
	vertex_data.faces = NULL;
	for (unsigned int j = 0; j < a_mesh->mNumVertices; ++j)
	{
		vertex_data.vertices[j].position[0] = a_mesh->mVertices[j].x;
		vertex_data.vertices[j].position[1] = a_mesh->mVertices[j].y;
		vertex_data.vertices[j].position[2] = a_mesh->mVertices[j].z;

		if (a_mesh->mNormals != NULL)
		{
			vertex_data.vertices[j].normal[0] = a_mesh->mNormals[j].x;
			vertex_data.vertices[j].normal[1] = a_mesh->mNormals[j].y;
			vertex_data.vertices[j].normal[2] = a_mesh->mNormals[j].z;
		}
		else
		{
			vertex_data.vertices[j].normal[0] = 0.0f;
			vertex_data.vertices[j].normal[1] = 0.0f;
			vertex_data.vertices[j].normal[2] = 0.0f;
		}

		if (a_mesh->mTextureCoords[0] != NULL)
		{
			/* This is weird. Double check the assimp docs once you start using textures */
			vertex_data.vertices[j].tex_coords[0] = a_mesh->mTextureCoords[0][j].x;
			vertex_data.vertices[j].tex_coords[1] = a_mesh->mTextureCoords[0][j].y;
			vertex_data.vertices[j].tex_coords[2] = 0.0f;
		}
		else
		{
			vertex_data.vertices[j].tex_coords[0] = 0.0f;
			vertex_data.vertices[j].tex_coords[1] = 0.0f;
			vertex_data.vertices[j].tex_coords[2] = 0.0f;
		}

		for (int k = 0; k < 4; ++k)
		{
			vertex_data.vertices[j].bone_ids[k] = -1;
			vertex_data.vertices[j].bone_weights[k] = 0.0f;
		}
		if (a_mesh->mNumBones)
		{
			int index_counter = 0;
			model->num_bones = a_mesh->mNumBones;
			for (int k = 0; k < (int)a_mesh->mNumBones; ++k)
			{
				for (int l = 0; l < (int)a_mesh->mBones[k]->mNumWeights; ++l)
				{
					if (a_mesh->mBones[k]->mWeights[l].mVertexId == j)
					{
						vertex_data.vertices[j].bone_ids[index_counter] = (GLint)k;
						vertex_data.vertices[j].bone_weights[index_counter] = a_mesh->mBones[k]->mWeights[l].mWeight;
						index_counter++;
					} 
				}

			}
		}
	}
	if (a_mesh->mNumFaces)
	{
		int num_elements = 0;
		for (unsigned int j = 0; j < a_mesh->mNumFaces; ++j)
		{
			C_STRUCT aiFace face = a_mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; ++k)
			{
				num_elements++;
			}
		} 
		vertex_data.num_faces = num_elements;
		b_mesh->num_faces = num_elements;
		vertex_data.faces = BG_MALLOC(unsigned int, num_elements);
		int index_counter = 0;
		for (unsigned int j = 0; j < a_mesh->mNumFaces; ++j)
		{
			C_STRUCT aiFace face = a_mesh->mFaces[j];
			for (unsigned int k = 0; k < face.mNumIndices; ++k)
			{
				vertex_data.faces[index_counter++] = face.mIndices[k];
			}
		}
	}

	b_mesh->active = 1;
	B_send_mesh_to_gpu(b_mesh, &vertex_data);
	BG_FREE(vertex_data.vertices);
	BG_FREE(vertex_data.faces);
	model->mesh = b_mesh;
	model->bone_array = B_load_bones(scene, a_mesh);
	model->current_animation = NULL;
}

C_STRUCT aiNode *B_get_root_model(C_STRUCT aiNode *root)
{
	C_STRUCT aiNode *return_node = NULL;
	if (root == NULL)
	{
		return NULL;
	}

	if (root->mNumMeshes)
	{
		return_node = root;
	}
	else
	{
		for (unsigned int i = 0; i < root->mNumChildren; ++i)
		{
			return_node = B_get_root_model(root->mChildren[i]);
			if (return_node != NULL)
			{
				break;
			}
		}
	}
	return return_node;
}

B_Texture B_load_texture(const char *node_name, const char *parent_directory)
{
	char *filename = B_get_texture_name(parent_directory, node_name);
	B_Texture texture = 0;
	if (filename == NULL)
	{
		unsigned char *pixel_data = BG_MALLOC(unsigned char, 16*16);
		texture = B_send_texture_data_to_gpu(pixel_data, 16, 16);
		BG_FREE(pixel_data);
	}
	else
	{
		texture = B_send_texture_to_gpu(filename);
		BG_FREE(filename);
	}
	return texture;
}

void B_load_ai_mesh(const C_STRUCT aiScene *scene, C_STRUCT aiNode *node, ActorModel *model, ActorModel *parent, const char *parent_directory)
{
	strncpy(model->name, node->mName.data, 255);
	B_load_ai_mesh_iter(scene, node, model);
	model->color_texture = B_load_texture(node->mName.data, parent_directory);
	model->parent = parent;
	if (node->mNumChildren)
	{
		model->num_children = node->mNumChildren;
		model->children = BG_MALLOC(ActorModel*, node->mNumChildren);
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			model->children[i] = BG_MALLOC(ActorModel, 1);
			B_load_ai_mesh(scene, node->mChildren[i], model->children[i], model, parent_directory);
		}
	}
}

void B_apply_node_transformations(C_STRUCT aiNode *node, ActorModel *model, mat4 parent_transform)
{
	ActorModel *child = model;
	mat4 local_space;
	mat4 world_space;
	assimp_to_cglm_mat4(node->mTransformation, local_space);
	glm_mat4_mul(parent_transform, local_space, world_space);

	int traverse_down = 0;
	if (strncmp(model->name, node->mName.data, 255) == 0)
	{
		glm_mat4_copy(local_space, model->local_space);
		glm_mat4_copy(world_space, model->world_space);
		glm_mat4_copy(model->world_space, model->original_position);
		traverse_down++;
	}
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		if (traverse_down)
		{
			child = model->children[i];
		}
		B_apply_node_transformations(node->mChildren[i], child, world_space);
	}

}

void B_load_ai_scene(const C_STRUCT aiScene *scene, ActorModel *model, const char *parent_directory)
{
	C_STRUCT aiNode *root_model = B_get_root_model(scene->mRootNode);
	B_load_ai_mesh(scene, root_model, model, NULL, parent_directory);
	mat4 root_transform;
	assimp_to_cglm_mat4(scene->mRootNode->mTransformation, root_transform);
	B_apply_node_transformations(scene->mRootNode, model, GLM_MAT4_IDENTITY);	
}

ActorModel *B_load_model_from_file(const char *filename)
{
	ActorModel *model = NULL;
	model = BG_MALLOC(ActorModel, 1);
	const C_STRUCT aiScene *scene = aiImportFile(filename, aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	char *dir_name = get_directory_name(filename);
	B_load_ai_scene(scene, model, dir_name);
	aiReleaseImport(scene);
	return model;
}

int is_bone(C_STRUCT aiNode *node)
{
	if (node->mNumMeshes)
	{
		return 0;
	}
	if ((strstr(node->mName.data, "armature")) ||
	    (strstr(node->mName.data, "Armature")))
	{
		return 0;
	}
	if (strstr(node->mName.data, "empty"))
	{
		return 0;
	}
	return 1;
}


int count_child_bones(C_STRUCT aiNode *node)
{
	int counter = 0;
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		if (is_bone(node))
		{
			counter++;
		}
	}
	return counter;
}

C_STRUCT aiNode *B_get_root_bone(C_STRUCT aiNode *root)
{
	C_STRUCT aiNode *return_node = NULL;
	if (root == NULL)
	{
		return NULL;
	}

	if (is_bone(root))
	{
		return_node = root;
	}
	else
	{
		for (unsigned int i = 0; i < root->mNumChildren; ++i)
		{
			return_node = B_get_root_bone(root->mChildren[i]);
			if (return_node != NULL)
			{
				break;
			}
		}
	}
	return return_node;
}

void B_load_bone_array_iter(C_STRUCT aiNode *node, Bone **bone_array, Bone *current_bone, Bone *parent, C_STRUCT aiBone **bones, int num_bones)
{
	if (!is_bone(node))
	{
		exit(-1);
	}

	memset(current_bone, 0, sizeof(Bone));
	strncpy(current_bone->name, node->mName.data, 255);

	mat4 local_space;
	assimp_to_cglm_mat4(node->mTransformation, local_space);

	if (parent != NULL)
	{
		glm_mat4_mul(parent->world_space, local_space, current_bone->world_space);
	}
	else
	{
		glm_mat4_identity(current_bone->world_space);
	}

	current_bone->id = -1;
	for (int i = 0; i < num_bones; ++i)
	{
		if (strncmp(bones[i]->mName.data, current_bone->name, 255) == 0)
		{
			current_bone->id = i;
			assimp_to_cglm_mat4(bones[i]->mOffsetMatrix, current_bone->inverse_bind);
			glm_mat4_mul(local_space, current_bone->inverse_bind, current_bone->current_transform);
			break;
		}
	}

	if (current_bone->id < 0)
	{
		fprintf(stderr, "load_bone_hierarchy error: %s has no ID\n", node->mName.data);
	}

	int num_children = count_child_bones(node);
	int child_index = 0;
	current_bone->num_children = num_children;
	current_bone->children = BG_MALLOC(int, num_children);
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		for (int j = 0; j < num_bones; ++j)
		{
			if (strncmp(node->mChildren[i]->mName.data, bones[j]->mName.data, 255) == 0)
			{
				current_bone->children[child_index++] = j;
			}
		}
	}

	bone_array[current_bone->id] = current_bone;
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		B_load_bone_array_iter(node->mChildren[i], bone_array, bone_array[current_bone->children[i]], current_bone, bones, num_bones);
	}
}

void create_bone_array(Bone *bone_array[], Bone *bone_node)
{
	bone_array[bone_node->id] = bone_node;
	for (int i = 0; i < bone_node->num_children; ++i)
	{
		create_bone_array(bone_array, bone_array[bone_node->children[i]]);
	}
}

Bone **B_load_bones(const C_STRUCT aiScene *scene, C_STRUCT aiMesh *mesh)
{
	if (!scene->mNumAnimations)
	{
		return NULL;
	}
	if (!mesh->mNumBones)
	{
		return NULL;
	}

	Bone **bone_array = BG_MALLOC(Bone*, mesh->mNumBones);
	for (unsigned int i = 0; i < mesh->mNumBones; ++i)
	{
		bone_array[i] = BG_MALLOC(Bone, 1);
	}	

	B_load_bone_array_iter(B_get_root_bone(scene->mRootNode), bone_array, bone_array[0], NULL, mesh->mBones, mesh->mNumBones);
	return bone_array;
}

void B_load_animation_nodes(C_STRUCT aiNode *ai_node, 
			    C_STRUCT aiNodeAnim **channels, int num_channels,
			    C_STRUCT aiBone **ai_bones, int num_bones,
			    AnimationNode *current_node,
			    AnimationNode **node_array)
{
	memset(current_node, 0, sizeof(AnimationNode));
	for (int i = 0; i < num_channels; ++i)
	{
		if (strncmp(channels[i]->mNodeName.data, ai_node->mName.data, 255) == 0)
		{
			current_node->id = -1;
			for (int j = 0; j < num_bones; ++j)
			{
				if (strncmp(ai_bones[j]->mName.data, ai_node->mName.data, 255) == 0)
				{
					current_node->id = j;
				}
			}
			if (current_node->id < 0)
			{
				fprintf(stderr, "Error: no id for animation node %s\n", ai_node->mName.data);
				exit(-1);
			}

			strncpy(current_node->name, channels[i]->mNodeName.data, 255);
			current_node->num_position_keys = channels[i]->mNumPositionKeys;
			current_node->num_rotation_keys = channels[i]->mNumRotationKeys;
			current_node->num_scale_keys = channels[i]->mNumScalingKeys;

			current_node->position_keys = BG_MALLOC(vec3, channels[i]->mNumPositionKeys);
			current_node->position_times = BG_MALLOC(float, channels[i]->mNumPositionKeys);
			for (unsigned int j = 0; j < channels[i]->mNumPositionKeys; ++j)
			{
				current_node->position_keys[j][0] = channels[i]->mPositionKeys[j].mValue.x;
				current_node->position_keys[j][1] = channels[i]->mPositionKeys[j].mValue.y;
				current_node->position_keys[j][2] = channels[i]->mPositionKeys[j].mValue.z;
				current_node->position_times[j] = channels[i]->mPositionKeys[j].mTime;
			}

			current_node->rotation_keys = BG_MALLOC(vec4, channels[i]->mNumRotationKeys);
			current_node->rotation_times = BG_MALLOC(float, channels[i]->mNumRotationKeys);
			for (unsigned int j = 0; j < channels[i]->mNumRotationKeys; ++j)
			{
				current_node->rotation_keys[j][0] = channels[i]->mRotationKeys[j].mValue.x;
				current_node->rotation_keys[j][1] = channels[i]->mRotationKeys[j].mValue.y;
				current_node->rotation_keys[j][2] = channels[i]->mRotationKeys[j].mValue.z;
				current_node->rotation_keys[j][3] = channels[i]->mRotationKeys[j].mValue.w;
				current_node->rotation_times[j] = channels[i]->mRotationKeys[j].mTime;
			}

			current_node->scale_keys = BG_MALLOC(vec3, channels[i]->mNumScalingKeys);
			current_node->scale_times = BG_MALLOC(float, channels[i]->mNumScalingKeys);
			for (unsigned int j = 0; j < channels[i]->mNumScalingKeys; ++j)
			{
				current_node->scale_keys[j][0] = channels[i]->mScalingKeys[j].mValue.x;
				current_node->scale_keys[j][1] = channels[i]->mScalingKeys[j].mValue.y;
				current_node->scale_keys[j][2] = channels[i]->mScalingKeys[j].mValue.z;
				current_node->scale_times[j] = channels[i]->mScalingKeys[j].mTime;
			}

			current_node->children = BG_MALLOC(int, ai_node->mNumChildren);
			current_node->num_children = ai_node->mNumChildren;
			int child_index = 0;
			for (int j = 0; j < num_channels; ++j)
			{
				for (unsigned int k = 0; k < ai_node->mNumChildren; ++k)
				{
					if (strncmp(ai_node->mChildren[k]->mName.data, ai_bones[j]->mName.data, 255) == 0)
					{
						current_node->children[child_index++] = j;
					}
				}
			}

			for (int j = 0; j < num_bones; ++j)
			{
				if (strncmp(ai_bones[j]->mName.data, ai_node->mName.data, 255) == 0)
				{
					mat4 offset_matrix;
					assimp_to_cglm_mat4(ai_bones[j]->mOffsetMatrix, offset_matrix);
					glm_mat4_copy(offset_matrix, current_node->inverse_bind);
				}
			}

			mat4 transformation;
			assimp_to_cglm_mat4(ai_node->mTransformation, transformation);
			glm_mat4_copy(transformation, current_node->current_transform);

			node_array[current_node->id] = current_node;
			for (int j = 0; j < current_node->num_children; ++j)
			{
				B_load_animation_nodes(ai_node->mChildren[j], channels, num_channels, ai_bones, num_bones, node_array[current_node->children[j]], node_array);
			}
		}
	}

	
}
	
Animation *B_load_animation(const C_STRUCT aiScene *scene, C_STRUCT aiAnimation *ai_animation)
{
	if (!scene->mNumAnimations)
	{
		return NULL;
	}

	Animation *animation = BG_MALLOC(Animation, 1);

	C_STRUCT aiNode *model = B_get_root_model(scene->mRootNode);
	C_STRUCT aiMesh *mesh = scene->mMeshes[model->mMeshes[0]];
	C_STRUCT aiBone **bones = mesh->mBones;

	animation->node_array = BG_MALLOC(AnimationNode*, ai_animation->mNumChannels);
	for (unsigned int i = 0; i < ai_animation->mNumChannels; ++i)
	{
		animation->node_array[i] = BG_MALLOC(AnimationNode, 1);
	}
	B_load_animation_nodes(B_get_root_bone(scene->mRootNode), 
			       ai_animation->mChannels, (int)ai_animation->mNumChannels, 
			       bones, (int)mesh->mNumBones, 
			       animation->node_array[0], animation->node_array);
	animation->duration = ai_animation->mDuration;
	animation->num_nodes = mesh->mNumBones;


	return animation;
}

Animation **B_load_animations_from_file(const char *filename, int *num_animations)
{
	const C_STRUCT aiScene *scene = aiImportFile(filename, aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	*num_animations = scene->mNumAnimations;
	Animation **animations = BG_MALLOC(Animation*, scene->mNumAnimations);
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
	{
		animations[i] = B_load_animation(scene, scene->mAnimations[i]);
	}

	aiReleaseImport(scene);
	return animations;
}

void B_send_mesh_to_gpu(ActorMesh *mesh, VertexData *vertex_data)
{
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	size_t stride = sizeof(GLfloat)*9 + sizeof(GLint)*4 + sizeof(GLfloat)*4;

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, vertex_data->num_vertices*stride, vertex_data->vertices, GL_DYNAMIC_DRAW);
	if (vertex_data->faces != NULL)
	{
		glGenBuffers(1, &mesh->ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertex_data->num_faces*sizeof(unsigned int), vertex_data->faces, GL_DYNAMIC_DRAW);
	}

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*3));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(GLfloat)*6));
	glVertexAttribIPointer(3, 4, GL_INT, stride, (void*)(sizeof(GLfloat)*9));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)((sizeof(GLfloat)*9)+sizeof(GLint)*4));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
}

