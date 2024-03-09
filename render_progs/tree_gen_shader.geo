#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 102) out;
uniform mat4 projection_view;

in VS_OUT
{
	float scale_factor;
	vec3 g_base_offset;
	vec3 g_group_offset;
} gs_in[];


out vec3 f_normal;
out vec3 f_position;

vec3 get_middle(vec3 a, vec3 b, vec3 c)
{
	float dist_ab = distance(a, b)/2.0;
	float dist_ac = distance(a, c)/2.0;
	vec3 dir_ab = normalize(b-a);
	vec3 dir_ac = normalize(c-a);

	return a + (dir_ab*dist_ab) + (dir_ac*dist_ac);
}

mat4 scale(vec3 axis)
{
	return mat4(
		vec4(axis.x, 0.0, 0.0, 0.0),
		vec4(0.0, axis.y, 0.0, 0.0),
		vec4(0.0, 0.0, axis.z, 0.0),
		vec4(0.0, 0.0, 0.0, 1.0));
}

mat4 rotate(vec3 axis, float angle)
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

mat4 translate(vec3 delta)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(delta, 1.0));
}

void main()
{
	vec3 base_position = gs_in[0].g_base_offset;
	vec3 branch_ends[3];
	for (int i = 0; i < gl_in.length(); i++)
	{
		vec3 group_position = gs_in[i].g_group_offset;
		f_normal = -normalize((base_position + group_position) - base_position);

		mat4 scaled = scale(vec3(gs_in[i].scale_factor));
		mat4 rotated = rotate(vec3(cross(vec3(0.0, 1.0, 0.0), f_normal)), acos(dot(vec3(0.0, 1.0, 0.0), f_normal)));
		mat4 translated = translate(gs_in[i].g_base_offset + gs_in[i].g_group_offset);

		vec3 trans = vec3(translated * rotated * scaled * gl_in[i].gl_Position);
		vec4 pos = projection_view * vec4(trans, 1.0);
		gl_Position = pos;
		f_position = trans;
		branch_ends[i] = trans;
		EmitVertex();
	}
	EndPrimitive();
}
