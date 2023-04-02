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

uniform mat4 scale;
uniform mat4 projection_view;
uniform sampler2D heightmap;
uniform float terrain_chunk_size;
uniform float view_distance;
uniform vec3 frustum_corners[8];
uniform float max_distance;

out vec3 f_position;
out vec3 f_normal;

vec3 get_frustum_normal(int i)
{
	switch (i)
	{
		case 0:
		{
			return normalize(cross(frustum_corners[1] - frustum_corners[0], frustum_corners[2] - frustum_corners[0]));
			break;
		}
		case 1:
		{
			return normalize(cross(frustum_corners[5] - frustum_corners[4], frustum_corners[1] - frustum_corners[4]));
			break;
		}
		case 2:
		{
			return normalize(cross(frustum_corners[2] - frustum_corners[3], frustum_corners[6] - frustum_corners[3]));
			break;
		}
		case 3:
		{
			return -normalize(cross(frustum_corners[5] - frustum_corners[7], frustum_corners[6] - frustum_corners[7]));
			break;
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
			break;
		}
		case 1:
		{
			return dot(frustum_corners[1], -get_frustum_normal(i));
			break;
		}
		case 2:
		{
			return dot(frustum_corners[6], -get_frustum_normal(i));
			break;
		}
		case 3:
		{
			return dot(frustum_corners[6], -get_frustum_normal(i));
			break;
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
	vec3 xz_location = vec3(g_offset.x, gs_in[0].g_player_pos.y , g_offset.y);

	bool render = true;
	for (int j = 0; j < 4; ++j)
	{
		vec3 normal = get_frustum_normal(j);
		if ((dot(normal, xz_location) + get_d(j)) < -10)
		{
			render = false;
			break;
		}
	}
	if (distance(g_base_offset, g_offset) > max_distance)
	{
		render = false;
	}
	if (render)
	{
		for (int i = 0; i < gl_in.length(); i++)
		{ 
			vec3 trans = vec3(scale * gl_in[i].gl_Position);
			vec2 tex_coords = g_offset/(terrain_chunk_size*3);
			tex_coords += 0.33;
			vec4 height_color = texture(heightmap, tex_coords);
			float height = height_color.r * (height_color.g * 2500)-0.75;
			trans += vec3(g_offset.x, height, g_offset.y);

			vec4 pos = projection_view * vec4(trans, 1.0);
			gl_Position = pos;
			f_position = trans;
			
			if (!render)
			{
				continue;
			}
			EmitVertex();
		}
	}

	EndPrimitive();
}
