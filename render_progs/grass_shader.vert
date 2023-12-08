#version 430 core

layout (location = 0) in vec3 v_pos;
uniform float patch_size;
uniform vec2 base_offset;
uniform vec3 player_facing;
uniform vec3 player_position;
uniform float time;

out VS_OUT
{
	vec3 	g_player_pos;
	vec2 	g_offset;
	vec2 	g_base_offset;
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
	int x_index = gl_InstanceID % int(patch_size);
	int z_index = gl_InstanceID / int(patch_size);
	float rand_num = fract(100000*sin(gl_InstanceID));
	float patch_size_factor = 0.4;
	float offx = (x_index-patch_size/2)*patch_size*patch_size_factor*rand_num;
	float offz = (z_index-patch_size/2)*patch_size*patch_size_factor*(rand_num*rand_num/2);
	vec2 final_xz_offset = vec2(base_offset.x + offx, base_offset.y + offz);

	mat4 rotation = rotate(vec3(0, 1, 0), rand_num);
	mat4 displacement = mat4(1.0);
	mat4 recenter = translate(vec3(-0.5, -0.5, -0.5));
	mat4 inv_recenter = inverse(recenter);
	float player_distance = distance(player_position.xz, final_xz_offset);
	if (player_distance < 8)
	{
		if (player_distance == 0)
		{
			player_distance = 0.01;
		}
		vec3 axis = normalize(-player_facing);
		displacement = translate(vec3(-1.0, -1.0, 0.5));
		float angle = min(1/(player_distance*2), 30);
		displacement = recenter * rotate(axis, angle);
		displacement = displacement * inv_recenter;
	}

	float wind_distance = (time * rand_num);
	float angle = radians(sin(wind_distance*2))*2;

	vec3 wind_rotation_axis = vec3(1, 0, 1);
	mat4 wind_displacement =  recenter * rotate(wind_rotation_axis, angle);
	wind_displacement = wind_displacement * inv_recenter;

	vs_out.g_offset = final_xz_offset;
	vs_out.g_player_pos = player_position;
	vs_out.instance_id = gl_InstanceID;
	vs_out.g_base_offset = base_offset;
	gl_Position = wind_displacement * rotation * displacement * vec4(v_pos, 1.0);
}
