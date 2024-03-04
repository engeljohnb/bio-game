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


#ifndef __ASSET_LOADING_H__
#define __ASSET_LOADING_H__
#include <stdio.h>
#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include "actor_rendering.h"

/* Since I'm not a good programmer, there are some kinks you need to work around to load a model properly.
 * Mostly, you need to make sure the root bone for any skinned mesh has translation (0,0,0), rotation (0,0,0), and scale (1,1,1). 
 * I'll fix it later if I can figure out what's wrong, but I've been stuck on this for a week and need to move on already.
 *
 * Also, only one skinned mesh per actor, and it must not be the child of another mesh. */

/* This function also loads all of the textures needed for the model. If the correct texture can't be found the texture is empty (zero'd) data.
 * For this function to load textures, the naming convention must be followed. The name of the texture must be the same as the name of the
 * corresponding node, but without any dots or numbers, and with "_texture.jpg" appended to the end of it.
 *
 * Examples:
 * 	if the node's name is "BlenderMonkey"    --->    "BlenderMonkey_texture.jpg"
 * 	if the node's name is "Cube.001" 	 ---> 	 "Cube_texture.jpg" 
 *
 * In addition, the texture and the model file must be in the same directory. */
ActorModel *B_load_model_from_file(const char *filename);
void B_send_mesh_to_gpu(ActorMesh *mesh, VertexData *vertex_data);
Animation **B_load_animations_from_file(const char *filename, int *num_animations);
void B_load_bone_array_iter(C_STRUCT aiNode *node, Bone **bone_array, Bone *current_bone, Bone *parent, C_STRUCT aiBone **bones, int num_bones);
Bone **B_load_bones(const C_STRUCT aiScene *scene, C_STRUCT aiMesh *mesh);

TerrainElementMesh load_plant_mesh_from_file(const char filename[], B_Framebuffer g_buffer, B_Texture heightmap);

B_Texture B_send_texture_to_gpu(const char *filename);
/* Assigns the color_texture of the mesh and all of it's children to the given texture */
void B_assign_all_color_textures(ActorModel *model, B_Texture texture);
void assimp_to_cglm_mat4(C_STRUCT aiMatrix4x4 source, mat4 dest);
void B_free_texture(B_Texture texture);
#endif
