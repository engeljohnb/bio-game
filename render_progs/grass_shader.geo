#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec3 g_player_pos;
	vec2 g_offset;
	vec2 g_base_offset;
	int  instance_id;
} gs_in[];

mat4 translate(vec3 delta)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(delta, 1.0));
}

//uniform mat4 scale;
//uniform float scale_factor;
uniform mat4 projection_view;
uniform sampler2D heightmap;
uniform float terrain_chunk_size;
uniform float view_distance;
uniform vec3 frustum_corners[8];
uniform float max_distance;
uniform float sea_level;
uniform int terrain_chunk_dimension;
uniform float xz_scale;
uniform int draw_debug;

out vec3 f_position;
out vec3 f_normal;
out float f_max_distance;
out vec3 f_center;
out flat int f_draw_debug;

vec3 get_frustum_normal(int i)
{
	switch (i)
	{
		case 0:
		{
			return normalize(cross(frustum_corners[1] - frustum_corners[0], frustum_corners[2] - frustum_corners[0]));
		}
		case 1:
		{
			return normalize(cross(frustum_corners[5] - frustum_corners[4], frustum_corners[1] - frustum_corners[4]));
		}
		case 2:
		{
			return normalize(cross(frustum_corners[2] - frustum_corners[3], frustum_corners[6] - frustum_corners[3]));
		}
		case 3:
		{
			return -normalize(cross(frustum_corners[5] - frustum_corners[7], frustum_corners[6] - frustum_corners[7]));
		}
	}
}

float get_d(int i)
{
	switch (i)
	{
		case 0:
		{
			return dot(frustum_corners[1], -get_frustum_normal(i));
		}
		case 1:
		{
			return dot(frustum_corners[1], -get_frustum_normal(i));
		}
		case 2:
		{
			return dot(frustum_corners[6], -get_frustum_normal(i));
		}
		case 3:
		{
			return dot(frustum_corners[6], -get_frustum_normal(i));
		}
	}
}

bool is_inside(vec3 normal, vec3 point, float d)
{
	return (dot(normal, point) + d > 0.0);
}

void main()
{
	vec3 a = vec3(gl_in[0].gl_Position);
	vec3 b = vec3(gl_in[1].gl_Position);
	vec3 c = vec3(gl_in[2].gl_Position);

	f_normal = normalize(cross((b-a), (c-a)));

	vec2 g_base_offset = gs_in[0].g_base_offset;
	vec2 g_offset = gs_in[0].g_offset;

	bool render = true;

	int half_dimension = int(terrain_chunk_dimension)/2;
	float min_xz = -float(half_dimension)*4.0;
	float max_xz = float(half_dimension+1) * 4.0;
	min_xz -= 0.03;
	max_xz -= 0.03;

	/* Position of this grass. */
	vec2 tex_coords = ((g_offset/xz_scale) - min_xz)/(max_xz-min_xz);
	vec4 height_color = texture(heightmap, tex_coords);
	float height = height_color.r * (height_color.g * 2500);
	vec3 position = vec3(g_offset.x, height, g_offset.y);

	/* Position of the center of the grass patch. */
	vec2 base_tex_coords = ((g_base_offset/xz_scale) - min_xz)/(max_xz-min_xz);
	vec4 base_height_color = texture(heightmap, base_tex_coords);
	float base_height = base_height_color.r * (base_height_color.g * 2500);
	vec3 base_position = vec3(g_base_offset.x, base_height, g_base_offset.y);

	f_center = base_position;
	f_max_distance = max_distance;
	f_draw_debug = draw_debug;

	height += 0.5;

	if (height < sea_level)
	{
		render = false;
	}
	if (distance(base_position, position) > max_distance)
	{
		render = false;
	}
	for (int i = 0; i < 4; ++i)
	{
		vec3 normal = get_frustum_normal(i);
		if ((dot(normal, position) + get_d(i)) < -10)
		{
			render = false;
			break;
		}
	}

	if (draw_debug != 0)
	{
		if (distance(base_position, position) < 10.0)
		{
			render = true;
		}
	}

	if (render)
	{
		for (int i = 0; i < gl_in.length(); i++)
		{ 
			vec3 trans = vec3(gl_in[i].gl_Position);
			trans += vec3(g_offset.x, height, g_offset.y);

			vec4 pos = projection_view * vec4(trans, 1.0);
			gl_Position = pos;
			f_position = trans;
			EmitVertex();
		}
	}

	EndPrimitive();
}
