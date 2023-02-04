#version 410 core

layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

uniform mat4 projection_view_space;
out vec3 f_position;
out vec3 f_normal;

void main()
{
	for (int i = 0; i < gl_in.length(); ++i)
	{
		vec4 pos = projection_view_space * gl_in[i].gl_Position; 
		gl_Position = pos;
		vec3 a = vec3(gl_in[0].gl_Position);
		vec3 b = vec3(gl_in[1].gl_Position);
		vec3 c = vec3(gl_in[2].gl_Position);
		f_normal = normalize(cross((b-a), (c-a)));
		f_position = vec3(gl_in[i].gl_Position);	
		EmitVertex();
	}
	EndPrimitive();
}
