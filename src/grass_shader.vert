#version 430 core

layout (location = 0) in vec3 v_pos;
uniform vec2 base_offset;
uniform vec3 player_position;
uniform float time;

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
	int x_index = gl_InstanceID % 300;
	int z_index = gl_InstanceID / 300;
	float offx = (x_index-300/2)*3*fract(10000*sin(gl_InstanceID));
	float offz = (z_index-300/2)*3*fract(20000*sin(gl_InstanceID));
	//float offx = float(x_index - (300/2));
	//float offz = float(z_index - (300/2));
	vec2 final_xz_offset = vec2(base_offset.x + offx, base_offset.y + offz);

	mat4 rotation = rotate(vec3(0, 1, 0), gl_InstanceID*fract(10000*sin(gl_InstanceID)));
	mat4 displacement = mat4(1.0);
	float player_distance = distance(player_position.xz, final_xz_offset);
	if (player_distance < 8)
	{
		if (player_distance == 0)
		{
			player_distance = 0.01;
		}
		vec3 axis = normalize(vec3(1, 0, 1));
		displacement = translate(vec3(-0.04, -0.5, 0));
		float angle = min(1/(player_distance*2), 30);
		displacement = displacement * rotate(axis, angle);
		displacement = displacement * inverse(translate(vec3(-0.04, -0.5, 0)));
	}

	vec2 wind_direction = normalize(vec2(1.0, 1.0));
	vec2 wind_location = wind_direction*(time/1000);
	float wind_distance = distance(vec2(offx, offz), wind_location);
	float angle = radians(sin(wind_distance*2))*10;

	vec3 wind_rotation_axis = normalize(vec3(wind_direction.x, 0, wind_direction.y));
	mat4 wind_displacement = translate(vec3(-0.04, -0.5, 0));
	wind_displacement = wind_displacement * rotate(wind_rotation_axis, angle);
	wind_displacement = wind_displacement * inverse(translate(vec3(-0.04, -0.5, 0)));

	vs_out.g_offset = final_xz_offset;
	vs_out.instance_id = gl_InstanceID;
	gl_Position = wind_displacement * rotation * displacement * vec4(v_pos, 1.0);
}
