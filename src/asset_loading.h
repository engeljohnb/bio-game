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


#ifndef __ASSET_LOADING_H__
#define __ASSET_LOADING_H__
#include <stdio.h>
#include <stdlib.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>       
#include <assimp/postprocess.h> 
#include "graphics.h"

/* Since I'm not a good programmer, there are some kinks you need to work around to load a model properly.
 * Mostly, you need to make sure the root bone for any skinned mesh has translation (0,0,0), rotation (0,0,0), and scale (1,1,1). 
 * I'll fix it later if I can figure out what's wrong, but I've been stuck on this for a week and need to move on already.
 *
 * Also, only one skinned mesh, and it must not be the child of another mesh. */
B_Model *B_load_model_from_file(const char *filename);
void B_setup_mesh_gl(B_Mesh *mesh);
void print_hierarchy(C_STRUCT aiNode *root);
Animation **B_load_animations_from_file(const char *filename, int *num_animations);
#endif
