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
	VertexData vertex_data;
	memset(&vertex_data, 0, sizeof(VertexData));
	B_Mesh *b_mesh = malloc(sizeof(B_Mesh));
	C_STRUCT aiMesh *a_mesh = scene->mMeshes[node->mMeshes[0]];
	memset(b_mesh, 0, sizeof(B_Mesh));
	vertex_data.num_vertices = a_mesh->mNumVertices;
	b_mesh->num_vertices = a_mesh->mNumVertices;
	vertex_data.vertices = (B_Vertex *)malloc(sizeof(B_Vertex) * a_mesh->mNumVertices);
	memset(vertex_data.vertices, 0, sizeof(B_Vertex) * a_mesh->mNumVertices);
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
		vertex_data.faces = (unsigned int *)malloc(sizeof(unsigned int) * num_elements);
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
	model->meshes[0] = b_mesh;
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

void B_load_ai_mesh(const C_STRUCT aiScene *scene, C_STRUCT aiNode *node, B_Model *model, B_Model *parent)
{
	strncpy(model->name, node->mName.data, 255);
	B_load_ai_mesh_iter(scene, node, model);
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
	// If it's it has meshes, it's not a bone
	if (!is_bone(node))
	{
		exit(-1);
	}

	memset(current_bone, 0, sizeof(Bone));
	strncpy(current_bone->name, node->mName.data, 255);

	assimp_to_cglm_mat4(node->mTransformation, current_bone->current_local);

	if (parent != NULL)
	{
		glm_mat4_mul(parent->world_space, current_bone->current_local, current_bone->world_space);
	}
	else
	{
		glm_mat4_identity(current_bone->world_space);
	}

	glm_mat4_copy(current_bone->current_local, current_bone->current_transform);
	current_bone->id = -1;
	for (int i = 0; i < num_bones; ++i)
	{
		if (strncmp(bones[i]->mName.data, current_bone->name, 255) == 0)
		{
			current_bone->id = i;
			assimp_to_cglm_mat4(bones[i]->mOffsetMatrix, current_bone->inverse_bind);
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
	current_bone->children = (int *)malloc(sizeof(int)*num_children);
	memset(current_bone->children, 0, sizeof(int)*num_children);
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


void print_bone_hierarchy_iter(Bone **bone_array, Bone *bone, int layer)
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
	for (int i = 0; i < bone->num_children; ++i)
	{
		print_bone_hierarchy_iter(bone_array, bone_array[bone->children[i]], layer+1);
	}
}

void print_bone_hierarchy(Bone **bone_array, Bone *root)
{
	print_bone_hierarchy_iter(bone_array, root, 0);
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

	Bone **bone_array = (Bone **)malloc(sizeof(Bone*) * mesh->mNumBones);
	memset(bone_array, 0, sizeof(Bone*) * mesh->mNumBones);

	for (unsigned int i = 0; i < mesh->mNumBones; ++i)
	{
		bone_array[i] = (Bone *)malloc(sizeof(Bone));
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

			current_node->position_keys = (vec3 *)malloc(sizeof(vec3) * channels[i]->mNumPositionKeys);
			current_node->position_times = (float *)malloc(sizeof(float) * channels[i]->mNumPositionKeys);
			memset(current_node->position_keys, 0, sizeof(vec3));
			memset(current_node->position_times, 0, sizeof(float));
			for (unsigned int j = 0; j < channels[i]->mNumPositionKeys; ++j)
			{
				current_node->position_keys[j][0] = channels[i]->mPositionKeys[j].mValue.x;
				current_node->position_keys[j][1] = channels[i]->mPositionKeys[j].mValue.y;
				current_node->position_keys[j][2] = channels[i]->mPositionKeys[j].mValue.z;
				current_node->position_times[j] = channels[i]->mPositionKeys[j].mTime;
			}

			current_node->rotation_keys = (vec4 *)malloc(sizeof(vec4) * channels[i]->mNumRotationKeys);
			current_node->rotation_times = (float *)malloc(sizeof(float) * channels[i]->mNumRotationKeys);
			memset(current_node->rotation_keys, 0, sizeof(vec4) * channels[i]->mNumRotationKeys);
			memset(current_node->rotation_times, 0, sizeof(float) * channels[i]->mNumRotationKeys);
			for (unsigned int j = 0; j < channels[i]->mNumRotationKeys; ++j)
			{
				current_node->rotation_keys[j][0] = channels[i]->mRotationKeys[j].mValue.x;
				current_node->rotation_keys[j][1] = channels[i]->mRotationKeys[j].mValue.y;
				current_node->rotation_keys[j][2] = channels[i]->mRotationKeys[j].mValue.z;
				current_node->rotation_keys[j][3] = channels[i]->mRotationKeys[j].mValue.w;
				current_node->rotation_times[j] = channels[i]->mRotationKeys[j].mTime;
			}

			current_node->scale_keys = (vec3 *)malloc(sizeof(vec3) * channels[i]->mNumScalingKeys);
			current_node->scale_times = (float *)malloc(sizeof(float) * channels[i]->mNumScalingKeys);
			memset(current_node->scale_keys, 0, sizeof(vec3) * channels[i]->mNumScalingKeys);
			memset(current_node->scale_times, 0, sizeof(float) * channels[i]->mNumScalingKeys);
			for (unsigned int j = 0; j < channels[i]->mNumScalingKeys; ++j)
			{
				current_node->scale_keys[j][0] = channels[i]->mScalingKeys[j].mValue.x;
				current_node->scale_keys[j][1] = channels[i]->mScalingKeys[j].mValue.y;
				current_node->scale_keys[j][2] = channels[i]->mScalingKeys[j].mValue.z;
				current_node->scale_times[j] = channels[i]->mScalingKeys[j].mTime;
			}

			current_node->children = (int *)malloc(sizeof(int)*ai_node->mNumChildren);
			memset(current_node->children, 0, sizeof(int)*ai_node->mNumChildren);
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
	Animation *animation = (Animation *)malloc(sizeof(Animation));
	memset(animation, 0, sizeof(Animation));

	C_STRUCT aiNode *model = B_get_root_model(scene->mRootNode);
	C_STRUCT aiMesh *mesh = scene->mMeshes[model->mMeshes[0]];
	C_STRUCT aiBone **bones = mesh->mBones;

	animation->node_array = (AnimationNode **)malloc(sizeof(AnimationNode*) * ai_animation->mNumChannels);
	memset(animation->node_array, 0, sizeof(AnimationNode*) * ai_animation->mNumChannels);
	for (unsigned int i = 0; i < ai_animation->mNumChannels; ++i)
	{
		animation->node_array[i] = (AnimationNode *)malloc(sizeof(AnimationNode));
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
	Animation **animations = (Animation **)malloc(sizeof(Animation *) * scene->mNumAnimations);
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
	{
		animations[i] = B_load_animation(scene, scene->mAnimations[i]);
	}

	aiReleaseImport(scene);
	return animations;
}

void B_send_mesh_to_gpu(B_Mesh *mesh, VertexData *vertex_data)
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
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		print_hierarchy_iter(node->mChildren[i], layer+1);
	}

}

void print_hierarchy(C_STRUCT aiNode *root)
{
	print_hierarchy_iter(root, 0);
}
