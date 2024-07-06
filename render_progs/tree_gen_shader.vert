#version 430 core

layout (location = 0) in vec3 v_position;

uniform vec3 base_offset;
uniform uint block;

out VS_OUT
{
	vec3 g_base_offset;
	vec3 g_group_offset;
	// Height from origin, not from ground
	float g_trunk_height;
	uint g_block;
} vs_out;


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

float rand(vec2 n) 
{ 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec3 get_group_offset(uint id)
{
	float x_id = float(id % block);
	float z_id = float(id / block);

	float rand_num0 = rand(vec2(x_id, z_id));
	float rand_num1 = rand(vec2(z_id, x_id));

	float coefficient = float(block*20)*1.50f;
	return vec3(rotate(vec3(rand_num0, 1.0, rand_num1), 10.0/rand_num0) * normalize(vec4(rand_num0, rand_num1, rand_num0*rand_num1, 1.0)) * coefficient);
}

void main()
{
	uint id = gl_InstanceID;

	vs_out.g_group_offset = get_group_offset(id);
	vs_out.g_base_offset = base_offset;

	vs_out.g_trunk_height = get_group_offset(block/2).y;
	vs_out.g_base_offset.y += get_group_offset(block/2).y;
	vs_out.g_block = block;

	gl_Position = vec4(v_position, 1.0);
}
