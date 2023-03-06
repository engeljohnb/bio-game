#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec2 g_offset;
	int  instance_id;
} gs_in[];

uniform mat4 scale;
uniform mat4 projection_view;
uniform sampler2D heightmap;
uniform float terrain_chunk_size;

out vec3 f_position;
out vec3 f_normal;

mat4 rotation(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main()
{
	vec3 a = vec3(gl_in[0].gl_Position);
	vec3 b = vec3(gl_in[1].gl_Position);
	vec3 c = vec3(gl_in[2].gl_Position);

	f_normal = normalize(cross((b-a), (c-a)));

	for (int i = 0; i < gl_in.length(); i++)
	{ 
		vec2 g_offset = gs_in[i].g_offset;
		vec3 trans = vec3(scale * gl_in[i].gl_Position);
		vec2 tex_coords = g_offset/(terrain_chunk_size*3);
		tex_coords += 0.33;
		vec4 height_color = texture(heightmap, tex_coords);
		float height = height_color.r * (height_color.g * 2500);
		trans += vec3(g_offset.x, height, g_offset.y);

		vec4 pos = projection_view * vec4(trans, 1.0);
		gl_Position = pos;
		f_position = trans;
		EmitVertex();
	}
	//EndPrimitive();
}
