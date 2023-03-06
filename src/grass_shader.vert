#version 430 core

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_offset;
uniform vec3 player_position;

out VS_OUT
{
	vec2 	g_offset;
	int	instance_id;
} vs_out;

mat4 translate(vec3 delta)
{
    return mat4(
        vec4(1.0, 0.0, 0.0, 0.0),
        vec4(0.0, 1.0, 0.0, 0.0),
        vec4(0.0, 0.0, 1.0, 0.0),
        vec4(delta, 1.0));
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

void main()
{
	mat4 rotation = rotate(vec3(0, 1, 0), radians(15*gl_InstanceID));
	vec3 final_pos = vec3(rotation * vec4(v_pos, 1.0));
	mat4 displacement = mat4(1.0);
	float dist = distance(player_position.xz, v_offset);
	// TODO: Make this nicer
	if (dist < 5)
	{
		vec2 axis_xz = v_offset - player_position.xz;
		vec3 axis = normalize(vec3(axis_xz.x, 0, axis_xz.y));
		displacement = translate(vec3(-0.04, -0.5, 0));
		float angle = radians(10);
		if (dist < 2)
		{
			angle = radians(30);
		}
		displacement = displacement * rotate(axis, angle);
		displacement = displacement * inverse(translate(vec3(-0.04, -0.5, 0)));
	}

	vs_out.g_offset = v_offset;
	vs_out.instance_id = gl_InstanceID;
	gl_Position = rotation * displacement * vec4(v_pos, 1.0);
}
