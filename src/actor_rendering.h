/*
    Bio-Game is a game for designing your own organism. 
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

#ifndef __GRAPHICS_H__
#define __GRAPHICS_H__
#include <cglm/cglm.h>
#include <glad/glad.h>
#include "camera.h"
#include "window.h"
#include "rendering.h"

#define MAX_BONES 25

typedef struct A_Vertex
{
	GLfloat	position[3];
	GLfloat	normal[3];
	GLfloat tex_coords[3];
	GLint	bone_ids[4];
	GLfloat	bone_weights[4];
} A_Vertex;

/* A Bone is the joint information for an animated ActorModel. The inverse_bind
 * transforms from the bone's local space to model space. Parent bone is not stored
 * because I never use it. Child bones are stored as indices:
 *
 * ActorModel *model = /// Load model ///;
 * Bone *root_bone = model->bone_array[0];
 * for (int i = 0; i < root_bone->num_children; ++i)
 * {
 * 	Bone *child_bone = model->bone_array[root_bone->children[i]]);
 * }
 *
 * Root bone should be the first bone in the ActorModel's bone_array. */

typedef struct Bone
{
	int		id;
	char		name[256];
	int		*children;
	int		num_children;
	mat4		inverse_bind;
	mat4		world_space;
	mat4		current_transform;	
} Bone;

/* In every Animation struct, there's a corresponding AnimationNode 
 * for each bone in the animated model. So obviously a single animation can only
 * be applied to different models if the models have the same bone hierarchy layout.
 * current_transform is the transformation to be applied to the animated model's corresponding
 * bone -- current_transform is in the bone's local space.
 *
 * AnimationNodes are arranged hierarchically like Bones. The children of each node are 
 * stored as indices. Example:
 *
 * Animation *animation = /// Load animation ///;
 * AnimationNode *root_node = animation->node_array[0];
 * for (int i = 0; i < root_node->num_children; ++i)
 * {
 * 	AnimationNode *child_node = animation->node_array[root_node->children[i]];
 * }
 * 
 * Root node should be the first node in the Animation struct's node_array. */
typedef struct AnimationNode
{
	int		id;
	char		name[256];
	int		num_position_keys;
	int		num_rotation_keys;
	int		num_scale_keys;
	float		*position_times;
	float		*rotation_times;
	float		*scale_times;
	vec3		*position_keys;
	vec4		*rotation_keys;
	vec3		*scale_keys;
	int		num_children;
	int		*children;
	mat4		inverse_bind;
	mat4		current_transform;
} AnimationNode;


/* An Animation struct stores the current time and duration of the animation, and everything else is stored in
 * the AnimationNode array. Each AnimationNode represents the transform to be applied to one
 * specific bone of the animated model.
 *
 * The root animation node should be the first in the node_array, unless I've done something wrong. */
typedef struct Animation
{
	float		time_reference;
	float		current_time;
	float		duration;
	int		num_nodes;
	AnimationNode	**node_array;
} Animation;


/* VertexData is created when a model is loaded, sent to the GPU, then discarded. See src/asset_loading.c: B_load_ai_mesh_iter */
typedef struct VertexData
{
	A_Vertex 	*vertices;
	unsigned int	*faces;
	int		num_vertices;
	int		num_faces;
} VertexData;

/* A ActorMesh is the vertex data needed to render a model. */

typedef struct ActorMesh
{
	int 		num_vertices;
	int		num_faces;	
	unsigned int 	vao;
	unsigned int	vbo;
	unsigned int	ebo;
} ActorMesh;

/* A ActorModel is all the graphical information required for an actor, including animations.
 * Models can be arranged hierarchically: each model can have one parent and any number of children.
 * The bone_array stores all the bones (joints) for an animated model. The root bone should be the first
 * in the array.
 *
 * Every model has one mesh. If you load a gltf file with multiple meshes in the same model, each mesh
 * will be divided up into its own model (unless I've done something wrong with the model loader). */

typedef struct ActorModel
{
	char			name[256];
	mat4			local_space;
	mat4			world_space;
	mat4			original_position;
	ActorMesh		*mesh;
	float			height;
	int			num_children;
	struct ActorModel 	**children;
	struct ActorModel 	*parent;
	Bone			**bone_array;
	int			num_bones;
	Animation		*current_animation;
	/* The texture containing the local colors of the model (what do you call it? Albedo?) */
	B_Texture		color_texture;
} ActorModel;

/* Calculates the transform to be applied to the corresponding bone in an animated ActorModel (ActorModel->bone_array[id]). The
 * transformation is actually applied to each bone in apply_animation. */
void advance_animation(AnimationNode *node, float current_time);

/* This is where the current_transform of each AnimationNode is applied to each Bone in the animated ActorModel. */
void apply_animation(AnimationNode **node_array, AnimationNode *node, 
		      Bone **bone_array, Bone *bone, 
		      mat4 parent_transform);

PointLight create_point_light(vec3 position, vec3 color, float intensity);
int B_check_shader(unsigned int id, const char *name, int status);
Renderer create_default_renderer(B_Window window);
void B_draw_actor_model(ActorModel *model, Camera camera, B_Shader shader);
void B_free_model(ActorModel *model);
void free_animation(Animation *animation);
void free_bone(Bone *bone);

#endif
