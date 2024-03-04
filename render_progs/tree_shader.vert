#version 430 core

layout (location = 0) in vec3 v_position;

uniform float scale_factor;
uniform vec3 base_offset;

out VS_OUT
{
	vec3 	g_player_pos;
	vec2 	g_offset;
	vec2 	g_base_offset;
	int	instance_id;
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

void main()
{
	vec3 pos = v_position;
	//pos += base_offset;
	//mat4 rotated = rotate(vec3(1.0, 0.0, 0.0), radians(9.0));
	//rotated *= rotate(vec3(1.0, 0.0, 0.0), radians(180.0));
	mat4 translated = translate(base_offset);
	gl_Position = translated * scale(vec3(scale_factor)) * vec4(pos, 1.0);
}
