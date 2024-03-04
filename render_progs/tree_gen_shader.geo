#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection_view;

in VS_OUT
{
	vec3 g_group_offset;
	vec4 g_branch_plane;
} gs_in[];


out vec3 f_normal;
out vec3 f_position;

void main()
{
	f_normal = vec3(gs_in[0].g_branch_plane);
	for (int i = 0; i < gl_in.length(); ++i)
	{
		f_position = gl_in[i].gl_Position.xyz;
		gl_Position = projection_view * gl_in[i].gl_Position;
		EmitVertex();

	}
	EndPrimitive();
}
