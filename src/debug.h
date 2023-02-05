/* Bio-Game is a game for designing your own microorganism. 
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

#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <glad/glad.h>
#include <cglm/cglm.h>
#include "utils.h"
unsigned int B_compile_vec_draw_shader(const char *vert_path, const char *frag_path);
void B_send_vec_draw_to_gpu(void);
void init_vec_draw(void);
void draw_vec(vec3 vec, unsigned int g_buffer);
void log_actor_state(ActorState actor_state);
#endif
