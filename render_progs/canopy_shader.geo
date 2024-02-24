#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 projection_view;
uniform mat4 scale;

in VS_OUT
{
	vec3 g_base_position;
	vec3 g_position;
} gs_in[];


out vec3 f_normal;
out vec3 f_position;

void main()
{
	vec3 a = vec3(gl_in[0].gl_Position);
	vec3 b = vec3(gl_in[1].gl_Position);
	vec3 c = vec3(gl_in[2].gl_Position);

	f_normal = normalize(cross((b-a), (c-a)));


	for (int i = 0; i < gl_in.length(); i++)
	{
		if (distance(gs_in[i].g_position, gs_in[i].g_base_position) > 50.0)
		{
			continue;
		}
		vec3 trans = vec3(scale * gl_in[i].gl_Position);
		trans += gs_in[i].g_position;

		vec4 pos = projection_view * vec4(trans, 1.0);
		gl_Position = pos;
		f_position = trans;
		EmitVertex();
	}
	EndPrimitive();
}
