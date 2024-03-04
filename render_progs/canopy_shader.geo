#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection_view;
uniform float max_distance;
//uniform mat4 scale;

in VS_OUT
{
	vec3 g_base_position;
	vec3 g_position;
} gs_in[];


out vec3 f_normal;
out vec3 f_position;

vec3 get_leaf_normal(vec3 position)
{
	float offset_x = 0.1;
	float offset_y = 0.1;
	float offset_z = 0.1;
	vec3 pos_dx = vec3(position.x + offset_x, position.y, position.z);
	vec3 pos_dz = vec3(position.x, position.y, position.z+ offset_z);
	vec3 pos_dxdz = vec3(position.x+offset_x, position.y, position.z + offset_z);

	return normalize(cross(pos_dz - pos_dx, pos_dxdz - pos_dx));
}


void main()
{
	f_normal = get_leaf_normal(gs_in[0].g_position);

	for (int i = 0; i < gl_in.length(); i++)
	{
		if (distance(gs_in[i].g_position, gs_in[i].g_base_position) > max_distance)
		{
			continue;
		}
		vec3 trans = vec3(gl_in[i].gl_Position);
		trans += gs_in[i].g_position;

		vec4 pos = projection_view * vec4(trans, 1.0);
		gl_Position = pos;
		f_position = trans;
		EmitVertex();
	}
	EndPrimitive();
}
