#version 430 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	int  instance_id;
} gs_in[];

out vec3 f_normal;
out vec3 f_position;

uniform mat4 projection_view;

void main()
{
	vec3 a = vec3(gl_in[0].gl_Position);
	vec3 b = vec3(gl_in[1].gl_Position);
	vec3 c = vec3(gl_in[2].gl_Position);

	f_normal = normalize(cross(b-a, c-a));
	for (int i = 0; i < gl_in.length(); ++i)
	{
		f_position = vec3(gl_in[i].gl_Position);
		vec3 pos = vec3(gl_in[i].gl_Position);
		gl_Position = projection_view * vec4(pos, 1.0);
		EmitVertex();	
	}
	EndPrimitive();
}
