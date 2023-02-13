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
#include <string.h>
#include "actor.h"
#include "camera.h"
#include "window.h"
#include "actor_rendering.h"
#include "utils.h"

void B_draw_actor_model(ActorModel *model, Camera camera, B_Shader shader)
{
	// TODO: Is this "if" really necessary?
	if (model->mesh->active)
	{
		static float current_time = 0.0f;
		glBindVertexArray(model->mesh->vao);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model->color_texture);
		B_set_uniform_int(shader, "color_texture", 0);

		mat4 projection_view;
		glm_mat4_mul(camera.projection_space, camera.view_space, projection_view);
		B_set_uniform_mat4(shader, "projection_view_space", projection_view);
		B_set_uniform_mat4(shader, "world_space", model->world_space);

		if (model->current_animation != NULL)
		{
			for (int i = 0; i < model->current_animation->num_nodes; ++i)
			{
				advance_animation(model->current_animation->node_array[i], current_time);
			}
			apply_animation(model->current_animation->node_array, model->current_animation->node_array[0], 
					 model->bone_array, model->bone_array[0], 
					 GLM_MAT4_IDENTITY);
			for (int i = 0; i < model->current_animation->num_nodes; ++i)
			{
				int id = model->bone_array[i]->id;
				char string[128] = {0};
				snprintf(string, 128, "bone_matrices[%i]", id);
				B_set_uniform_mat4(shader, string, model->bone_array[id]->current_transform);
			}
			for (int i = model->num_bones; i < MAX_BONES; ++i)
			{
				char string[128] = {0};
				snprintf(string, 128, "bone_matrices[%i]", i);
				B_set_uniform_mat4(shader, string, GLM_MAT4_IDENTITY);
			}
			current_time += 15.0;
			if (current_time >= model->current_animation->duration)
			{
				current_time = 0.0f;
			}
		}
		else
		{
			for (int i = 0; i < MAX_BONES; ++i)
			{
				char string[128] = {0};
				snprintf(string, 128, "bone_matrices[%i]", i);
				B_set_uniform_mat4(shader, string, GLM_MAT4_IDENTITY);
			}
		}

		glUseProgram(shader);
		if (model->mesh->num_faces)
		{
			glDrawElements(GL_TRIANGLES, model->mesh->num_faces, GL_UNSIGNED_INT, 0);
		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, model->mesh->num_vertices);
		}
	}
	for (int i = 0; i < model->num_children; ++i)
	{
		B_draw_actor_model(model->children[i], camera, shader);
	}

}

PointLight create_point_light(vec3 position, vec3 color, float intensity)
{
	PointLight light = { 0 };
	glm_vec3_copy(position, light.position);
	glm_vec3_copy(color, light.color);
	light.intensity = intensity;
	return light;
}

int get_animation_position_index(AnimationNode *node, float current_time)
{
	int animation_index = -1;
	for (int i = 0; i < node->num_position_keys-1; ++i)
	{
		if (node->position_times[i+1] > current_time)
		{
			animation_index = i;
			break;
		}
	}
	if (animation_index < 0)
	{
		fprintf(stderr, "get_animation_position_index error: could not get animation position index\n");
		exit(-1);
	}
	return animation_index;
}

int get_animation_scale_index(AnimationNode *node, float current_time)
{
	int animation_index = -1;
	for (int i = 0; i < node->num_scale_keys-1; ++i)
	{
		if (node->scale_times[i+1] > current_time)
		{
			animation_index = i;
			break;
		}
	}
	if (animation_index < 0)
	{
		fprintf(stderr, "get_animation_scale_index error: could not get animation scale index\n");
	}
	return animation_index;
}

int get_animation_rotation_index(AnimationNode *node, float current_time)
{
	int animation_index = -1;
	for (int i = 0; i < node->num_rotation_keys-1; ++i)
	{
		if (node->rotation_times[i+1] > current_time)
		{
			animation_index = i;
			break;
		}
	}
	if (animation_index < 0)
	{
		fprintf(stderr, "get_animation_rotation_index error: could not get animation rotation index\n");
	}
	return animation_index;
}

void advance_animation_position(AnimationNode *node, float current_time)
{
	if (!node->num_position_keys)
	{
		return;
	}
	int position_index_0 = get_animation_position_index(node, current_time);
	int position_index_1 = position_index_0 + 1;
	float position_time_factor = glm_percent(node->position_times[position_index_0], node->position_times[position_index_1], current_time);

	vec3 position_0;
	vec3 position_1;
	vec3 position;
	glm_vec3_copy(node->position_keys[position_index_0], position_0);
	glm_vec3_copy(node->position_keys[position_index_1], position_1);
	glm_vec3_lerp(position_0, position_1, position_time_factor, position);

	mat4 final_transform;

	glm_mat4_identity(final_transform);
	glm_translate(final_transform, position);
	glm_mat4_mul(final_transform, node->inverse_bind, final_transform);
	glm_mat4_copy(final_transform, node->current_transform);
}

void apply_animation(AnimationNode **node_array, AnimationNode *node, 
		      Bone **bone_array, Bone *bone, 
		      mat4 parent_transform)
{
	mat4 current_model_space;
	glm_mat4_mul(parent_transform, node->current_transform, current_model_space);
	for (int i = 0; i < bone->num_children; ++i)
	{
		apply_animation(node_array, node_array[node->children[i]], bone_array, bone_array[bone->children[i]], current_model_space);
	}
	glm_mat4_mul(current_model_space, bone->inverse_bind, bone->current_transform);
}

void advance_animation(AnimationNode *node, float current_time)
{
	if ((node->num_position_keys == 0) ||
	    (node->num_rotation_keys == 0) ||
	    (node->num_scale_keys == 0))
	{
		return;
	}
	int position_index_0 = get_animation_position_index(node, current_time);
	int position_index_1 = position_index_0 + 1;
	int scale_index_0 = get_animation_scale_index(node, current_time);
	int scale_index_1 = scale_index_0 + 1;
	int rotation_index_0 = get_animation_rotation_index(node, current_time);
	int rotation_index_1 = rotation_index_0 + 1;

	float position_time_factor = glm_percent(node->position_times[position_index_0], node->position_times[position_index_1], current_time);
	float scale_time_factor = glm_percent(node->scale_times[scale_index_0], node->scale_times[scale_index_1], current_time);
	float rotation_time_factor = glm_percent(node->rotation_times[rotation_index_0], node->rotation_times[rotation_index_1], current_time);

	vec3 scale_0;
	vec3 scale_1;
	vec3 scale;
	glm_vec3_copy(node->scale_keys[scale_index_0], scale_0);
	glm_vec3_copy(node->scale_keys[scale_index_1], scale_1);

	vec3 position_0;
	vec3 position_1;
	vec3 position;
	glm_vec3_copy(node->position_keys[position_index_0], position_0);
	glm_vec3_copy(node->position_keys[position_index_1], position_1);

	vec4 rotation_0_vec;
	vec4 rotation_1_vec;
	glm_vec4_copy(node->rotation_keys[rotation_index_0], rotation_0_vec);
	glm_vec4_copy(node->rotation_keys[rotation_index_1], rotation_1_vec);
	versor rotation_0;
	versor rotation_1;
	versor rotation;
	glm_quat_init(rotation_0, rotation_0_vec[0], rotation_0_vec[1], rotation_0_vec[2], rotation_0_vec[3]);
	glm_quat_init(rotation_1, rotation_1_vec[0], rotation_1_vec[1], rotation_1_vec[2], rotation_1_vec[3]);

	glm_vec3_lerp(position_0, position_1, position_time_factor, position);
	glm_vec3_lerp(scale_0, scale_1, scale_time_factor, scale);
	glm_quat_lerp(rotation_0, rotation_1, rotation_time_factor, rotation);
	mat4 rotation_mat4;
	glm_quat_mat4(rotation, rotation_mat4);

	mat4 final_transform;
	glm_mat4_identity(final_transform);

	glm_translate(final_transform, position);
	glm_mat4_mul(final_transform, rotation_mat4, final_transform);
	glm_scale(final_transform, scale);

	glm_mat4_copy(final_transform, node->current_transform);
	return;
}

void B_free_mesh(ActorMesh *mesh)
{
	if (mesh->active)
	{
		glDeleteBuffers(1, &(mesh->ebo));
		glDeleteBuffers(1, &(mesh->vbo));
	}
	BG_FREE(mesh);
}

void B_free_model(ActorModel *model)
{
	B_free_mesh(model->mesh);
	for (int i = 0; i < model->num_bones; ++i)
	{
		free_bone(model->bone_array[i]);
	}
	BG_FREE(model->bone_array);
	for (int i = 0; i < model->num_children; ++i)
	{
		B_free_model(model->children[i]);
	}
	if (model->num_children)
	{
		BG_FREE(model->children);
	}
	
	BG_FREE(model);
}

void free_animation_node(AnimationNode *node)
{
	BG_FREE(node->position_times);
	BG_FREE(node->scale_times);
	BG_FREE(node->rotation_times);

	BG_FREE(node->position_keys);
	BG_FREE(node->rotation_keys);
	BG_FREE(node->scale_keys);

	BG_FREE(node->children);
	BG_FREE(node);
}

void free_animation(Animation *animation)
{
	for (int i = 0; i < animation->num_nodes; ++i)
	{
		free_animation_node(animation->node_array[i]);
	}
	BG_FREE(animation->node_array);
	BG_FREE(animation);
}

void free_bone(Bone *bone)
{
	BG_FREE(bone->children);
	BG_FREE(bone);
}

