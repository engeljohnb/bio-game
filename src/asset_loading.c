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
#include "asset_loading.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include "utils.h"

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

void B_load_ai_mesh_iter(const C_STRUCT aiScene *scene, C_STRUCT aiNode *node, B_Model *model)
{
	for (unsigned int i = 0; i < node->mNumMeshes; ++i)
	{
		B_Mesh *b_mesh = malloc(sizeof(B_Mesh));
		C_STRUCT aiMesh *a_mesh = scene->mMeshes[node->mMeshes[i]];
		memset(b_mesh, 0, sizeof(B_Mesh));
		b_mesh->num_vertices = a_mesh->mNumVertices;
		b_mesh->vertices = (B_Vertex *)malloc(sizeof(B_Vertex) * a_mesh->mNumVertices);
		memset(b_mesh->vertices, 0, sizeof(B_Vertex) * a_mesh->mNumVertices);
		for (unsigned int j = 0; j < a_mesh->mNumVertices; ++j)
		{
			b_mesh->vertices[j].position[0] = a_mesh->mVertices[j].x;
			b_mesh->vertices[j].position[1] = a_mesh->mVertices[j].y;
			b_mesh->vertices[j].position[2] = a_mesh->mVertices[j].z;

			if (a_mesh->mNormals != NULL)
			{
				b_mesh->vertices[j].normal[0] = a_mesh->mNormals[j].x;
				b_mesh->vertices[j].normal[1] = a_mesh->mNormals[j].y;
				b_mesh->vertices[j].normal[2] = a_mesh->mNormals[j].z;
			}
			else
			{
				b_mesh->vertices[j].normal[0] = 0.0f;
				b_mesh->vertices[j].normal[1] = 0.0f;
				b_mesh->vertices[j].normal[2] = 0.0f;
			}

			if (a_mesh->mTextureCoords[0] != NULL)
			{
				/* This is weird. Double check the assimp docs once you start using textures */
				b_mesh->vertices[j].tex_coords[0] = a_mesh->mTextureCoords[0][j].x;
				b_mesh->vertices[j].tex_coords[1] = a_mesh->mTextureCoords[0][j].y;
				b_mesh->vertices[j].tex_coords[2] = 0.0f;
			}
			else
			{
				b_mesh->vertices[j].tex_coords[0] = 0.0f;
				b_mesh->vertices[j].tex_coords[1] = 0.0f;
				b_mesh->vertices[j].tex_coords[2] = 0.0f;
			}

			for (int k = 0; k < 4; ++k)
			{
				b_mesh->vertices[j].bone_ids[k] = -1;
				b_mesh->vertices[j].bone_weights[k] = 0.0f;
			}
			if (a_mesh->mNumBones)
			{
				int index_counter = 0;
				for (int k = 0; k < (int)a_mesh->mNumBones; ++k)
				{
					for (int l = 0; l < (int)a_mesh->mBones[k]->mNumWeights; ++l)
					{
						if (a_mesh->mBones[k]->mWeights[l].mVertexId == j)
						{
							b_mesh->vertices[j].bone_ids[index_counter] = (GLint)k;
							b_mesh->vertices[j].bone_weights[index_counter] = a_mesh->mBones[k]->mWeights[l].mWeight;
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
			b_mesh->num_faces = num_elements;
			b_mesh->faces = (unsigned int *)malloc(sizeof(unsigned int) * num_elements);
			int index_counter = 0;
			for (unsigned int j = 0; j < a_mesh->mNumFaces; ++j)
			{
				C_STRUCT aiFace face = a_mesh->mFaces[j];
				for (unsigned int k = 0; k < face.mNumIndices; ++k)
				{
					b_mesh->faces[index_counter++] = face.mIndices[k];
				}
			}
		}
		b_mesh->active = 1;
		B_setup_mesh_gl(b_mesh);
		model->meshes[i] = b_mesh;
	}
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

void B_load_ai_mesh(const C_STRUCT aiScene *scene, C_STRUCT aiNode *node, B_Model *model, B_Model *parent)
{
	B_load_ai_mesh_iter(scene, node, model);
	strncpy(model->name, node->mName.data, 255);
	model->parent = parent;
	if (node->mNumChildren)
	{
		model->num_children = node->mNumChildren;
		model->children = (B_Model **)malloc(sizeof(B_Model *) * node->mNumChildren);
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			model->children[i] = (B_Model *)malloc(sizeof(B_Model));
			memset(model->children[i], 0, sizeof(B_Model));
			B_load_ai_mesh(scene, node->mChildren[i], model->children[i], model);
		}
	}
}

void B_apply_node_transformations(C_STRUCT aiNode *node, B_Model *model, mat4 parent_transform)
{
	B_Model *child = model;
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

void B_load_ai_scene(const C_STRUCT aiScene *scene, B_Model *model)
{
	C_STRUCT aiNode *root_model = B_get_root_model(scene->mRootNode);
	B_load_ai_mesh(scene, root_model, model, NULL);
	mat4 root_transform;
	assimp_to_cglm_mat4(scene->mRootNode->mTransformation, root_transform);
	B_apply_node_transformations(scene->mRootNode, model, GLM_MAT4_IDENTITY);	
}

B_Model *B_load_model_from_file(const char *filename)
{
	B_Model *model = NULL;
	model = (B_Model *)malloc(sizeof(B_Model));
	memset(model, 0, sizeof(B_Model));
	const C_STRUCT aiScene *scene = aiImportFile(filename, aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	B_load_ai_scene(scene, model);
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

Bone *B_load_bone_hierarchy(C_STRUCT aiNode *node, C_STRUCT aiNodeAnim **channels, int num_channels, Bone *parent, C_STRUCT aiBone **bones, int num_bones)
{
	// If it's it has meshes, it's not a bone
	if (!is_bone(node))
	{
		exit(0);
		return NULL;
	}

	Bone *current_bone = (Bone *)malloc(sizeof(Bone));
	memset(current_bone, 0, sizeof(Bone));
	strncpy(current_bone->name, node->mName.data, 255);

	assimp_to_cglm_mat4(node->mTransformation, current_bone->local_space);
	glm_mat4_copy(current_bone->local_space, current_bone->current_local);

	if (parent != NULL)
	{
		glm_mat4_mul(parent->world_space, current_bone->local_space, current_bone->world_space);
	}
	else
	{
		glm_mat4_identity(current_bone->world_space);
	}

	glm_mat4_copy(current_bone->local_space, current_bone->current_transform);
	current_bone->id = -1;
	for (int i = 0; i < num_bones; ++i)
	{
		if (strncmp(bones[i]->mName.data, current_bone->name, 255) == 0)
		{
			current_bone->id = i;
			assimp_to_cglm_mat4(bones[i]->mOffsetMatrix, current_bone->offset);
			break;
		}
	}

	if (current_bone->id < 0)
	{
		fprintf(stderr, "load_bone_hierarchy error: %s has no ID\n", node->mName.data);
	}

	for (int i = 0; i < num_channels; ++i)
	{
		if (strncmp(channels[i]->mNodeName.data, current_bone->name, 255) == 0)
		{
			current_bone->num_position_keys = channels[i]->mNumPositionKeys;
			current_bone->num_rotation_keys = channels[i]->mNumRotationKeys;
			current_bone->num_scale_keys = channels[i]->mNumScalingKeys;

			current_bone->position_keys = (vec3 *)malloc(sizeof(vec3) * channels[i]->mNumPositionKeys);
			current_bone->position_times = (float *)malloc(sizeof(float) * channels[i]->mNumPositionKeys);
			memset(current_bone->position_keys, 0, sizeof(vec3));
			memset(current_bone->position_times, 0, sizeof(float));
			for (unsigned int j = 0; j < channels[i]->mNumPositionKeys; ++j)
			{
				current_bone->position_keys[j][0] = channels[i]->mPositionKeys[j].mValue.x;
				current_bone->position_keys[j][1] = channels[i]->mPositionKeys[j].mValue.y;
				current_bone->position_keys[j][2] = channels[i]->mPositionKeys[j].mValue.z;
				current_bone->position_times[j] = channels[i]->mPositionKeys[j].mTime;
			}

			current_bone->rotation_keys = (vec4 *)malloc(sizeof(vec4) * channels[i]->mNumRotationKeys);
			current_bone->rotation_times = (float *)malloc(sizeof(float) * channels[i]->mNumRotationKeys);
			memset(current_bone->rotation_keys, 0, sizeof(vec4) * channels[i]->mNumRotationKeys);
			memset(current_bone->rotation_times, 0, sizeof(float) * channels[i]->mNumRotationKeys);
			for (unsigned int j = 0; j < channels[i]->mNumRotationKeys; ++j)
			{
				current_bone->rotation_keys[j][0] = channels[i]->mRotationKeys[j].mValue.x;
				current_bone->rotation_keys[j][1] = channels[i]->mRotationKeys[j].mValue.y;
				current_bone->rotation_keys[j][2] = channels[i]->mRotationKeys[j].mValue.z;
				current_bone->rotation_keys[j][3] = channels[i]->mRotationKeys[j].mValue.w;
				current_bone->rotation_times[j] = channels[i]->mRotationKeys[j].mTime;
			}

			current_bone->scale_keys = (vec3 *)malloc(sizeof(vec3) * channels[i]->mNumScalingKeys);
			current_bone->scale_times = (float *)malloc(sizeof(float) * channels[i]->mNumScalingKeys);
			memset(current_bone->scale_keys, 0, sizeof(vec3) * channels[i]->mNumScalingKeys);
			memset(current_bone->scale_times, 0, sizeof(float) * channels[i]->mNumScalingKeys);
			for (unsigned int j = 0; j < channels[i]->mNumScalingKeys; ++j)
			{
				current_bone->scale_keys[j][0] = channels[i]->mScalingKeys[j].mValue.x;
				current_bone->scale_keys[j][1] = channels[i]->mScalingKeys[j].mValue.y;
				current_bone->scale_keys[j][2] = channels[i]->mScalingKeys[j].mValue.z;
				current_bone->scale_times[j] = channels[i]->mScalingKeys[j].mTime;
			}
		}
	}

	int num_children = count_child_bones(node);
	int child_index = 0;
	current_bone->num_children = num_children;
	current_bone->children = (Bone **)malloc(sizeof(Bone*)*num_children);
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		if (!is_bone(node->mChildren[i]))
		{
			continue;
		}
		current_bone->children[child_index++] = B_load_bone_hierarchy(node->mChildren[i], channels, num_channels, current_bone, bones, num_bones);
	}

	return current_bone;
}


void print_bone_hierarchy_iter(Bone *bone, int layer)
{
	if (bone == NULL)
	{
		fprintf(stderr, "print_bone_hierarchy error: invalid bone\n");
		return;
	}

	int current_index = 0;
	char string[512] = {0};
	for (int i = 0; i < layer; ++i)
	{
		string[current_index++] = ' ';
		string[current_index++] = ' ';
	}
	char *begin_name = (char *)string + current_index;
	memcpy(begin_name, bone->name, strnlen(bone->name, 255));
	fprintf(stderr, "%s\n", string);
	for (int i = 0; i < bone->num_children; ++i)
	{
		print_bone_hierarchy_iter(bone->children[i], layer+1);
	}
}

void print_bone_hierarchy(Bone *root)
{
	print_bone_hierarchy_iter(root, 0);
}

void create_bone_array(Bone *bone_array[], Bone *bone_node)
{
	bone_array[bone_node->id] = bone_node;
	for (int i = 0; i < bone_node->num_children; ++i)
	{
		create_bone_array(bone_array, bone_node->children[i]);
	}
}

Animation *B_load_animation(const C_STRUCT aiScene *scene, C_STRUCT aiAnimation ai_animation)
{
	if (!scene->mNumAnimations)
	{
		return NULL;
	}
	Animation *animation = (Animation *)malloc(sizeof(Animation));
	memset(animation, 0, sizeof(Animation));

	C_STRUCT aiNode *model= B_get_root_model(scene->mRootNode);
	C_STRUCT aiMesh *mesh = scene->mMeshes[model->mMeshes[0]];
	Bone * bone_hierarchy = B_load_bone_hierarchy(B_get_root_bone(scene->mRootNode), ai_animation.mChannels, ai_animation.mNumChannels, NULL, mesh->mBones, mesh->mNumBones);
	animation->duration = ai_animation.mDuration;
	animation->num_bones = mesh->mNumBones;
	animation->bone_hierarchy = bone_hierarchy;
	create_bone_array(animation->bone_array, bone_hierarchy);

	return animation;
}

Animation **B_load_animations_from_file(const char *filename, int *num_animations)
{
	const C_STRUCT aiScene *scene = aiImportFile(filename, aiProcess_FlipUVs | aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	*num_animations = scene->mNumAnimations;
	Animation **animations = (Animation **)malloc(sizeof(Animation *) * scene->mNumAnimations);
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
	{
		animations[i] = B_load_animation(scene, *scene->mAnimations[i]);
	}
	return animations;
}

void B_setup_mesh_gl(B_Mesh *mesh)
{
	glGenVertexArrays(1, &mesh->vao);
	glBindVertexArray(mesh->vao);

	size_t stride = sizeof(GLfloat)*9 + sizeof(GLint)*4 + sizeof(GLfloat)*4;

	glGenBuffers(1, &mesh->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices*stride, mesh->vertices, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &mesh->ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_faces*sizeof(unsigned int), mesh->faces, GL_DYNAMIC_DRAW);

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

void print_hierarchy_iter(C_STRUCT aiNode *node, int layer)
{
	int current_index = 0;
	char string[512] = {0};
	for (int i = 0; i < layer; ++i)
	{
		string[current_index++] = ' ';
		string[current_index++] = ' ';
	}
	char *begin_name = (char *)string + current_index;
	memcpy(begin_name, node->mName.data, strnlen(node->mName.data, 256));
	fprintf(stderr, "%s\n", string);
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		print_hierarchy_iter(node->mChildren[i], layer+1);
	}

}

void print_hierarchy(C_STRUCT aiNode *root)
{
	print_hierarchy_iter(root, 0);
}
