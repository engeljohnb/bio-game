#version 430 core

layout (location = 0) in vec3 pos;

uniform vec3 frustum_corners[8];
uniform float num_instances;
uniform float time;
uniform vec3 player_pos;
uniform vec3 player_facing;
uniform mat4 scale;

out VS_OUT
{
	int instance_id;
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
#define RIGHT_TOP_NEAR 2
#define LEFT_TOP_FAR 5
#define RIGHT_TOP_FAR 6
#define RIGHT_BOTTOM_FAR 7

float get_min_z(vec3 frustum_corners[8])
{
	float min_z = frustum_corners[0].z;
	for (int i = 0; i < 8; ++i)
	{
		if (frustum_corners[i].z < min_z)
		{
			min_z = frustum_corners[i].z;
		}
	}
	return min_z;
}

float get_min_y(vec3 frustum_corners[8])
{
	float min_y = frustum_corners[0].y;
	for (int i = 0; i < 8; ++i)
	{
		if (frustum_corners[i].y < min_y)
		{
			min_y = frustum_corners[i].y;
		}
	}
	return min_y;
}


float get_min_x(vec3 frustum_corners[8])
{
	float min_x = frustum_corners[0].x;
	for (int i = 0; i < 8; ++i)
	{
		if (frustum_corners[i].x < min_x)
		{
			min_x = frustum_corners[i].x;
		}
	}
	return min_x;
}


float get_max_z(vec3 frustum_corners[8])
{
	float max_z = frustum_corners[0].z;
	for (int i = 0; i < 8; ++i)
	{
		if (frustum_corners[i].z > max_z)
		{
			max_z = frustum_corners[i].z;
		}
	}
	return max_z;
}

float get_max_x(vec3 frustum_corners[8])
{
	float max_x = frustum_corners[0].x;
	for (int i = 0; i < 8; ++i)
	{
		if (frustum_corners[i].x > max_x)
		{
			max_x = frustum_corners[i].x;
		}
	}
	return max_x;
}

float get_max_y(vec3 frustum_corners[8])
{
	float max_y = frustum_corners[0].y;
	for (int i = 0; i < 8; ++i)
	{
		if (frustum_corners[i].y > max_y)
		{
			max_y = frustum_corners[i].y;
		}
	}
	return max_y;
}
void main()
{
	vs_out.instance_id = gl_InstanceID;
	float size = round(sqrt(num_instances));
	int x_index =  gl_InstanceID % int(size);
	int z_index =  gl_InstanceID / int(size);
	float x_percent = float(x_index)/size;
	float z_percent = float(z_index)/size;
	vec3 player_facing_xz = normalize(vec3(player_facing.x, 0.0, player_facing.z));
	vec3 player_pos_xz = vec3(player_pos.x, 0.0, player_pos.z);
	vec3 player_right = normalize(cross(vec3(0.0, 1.0, 0.0), player_facing_xz));
	vec3 left = (player_right * 500.0);
	vec3 right = (-player_right * 500.0);
	vec3 front = (-player_facing_xz * 500.0);
	vec3 back = (player_facing_xz * 50.0);
	vec3 para = mix(front, back, z_percent);
	vec3 perp = mix(left, right, x_percent);
	float rand_num = fract(100000*sin(gl_InstanceID));
	float velocity = 25.0;
	float init_y_position = get_max_y(frustum_corners) + rand_num*800.0;
	float final_y_position = get_min_y(frustum_corners);
	float total_time = (init_y_position - final_y_position)/velocity;
	float current_time = mod(time, total_time);
	float height = mix(init_y_position, final_y_position, current_time/total_time);

	mat4 translation = translate(player_pos_xz) + translate(para) + translate(perp) + translate(vec3(0.0, height, 1.0));
	mat4 rotation = rotate(vec3(0.0, 1.0, 0.0), rand_num);
	gl_Position = translation * rotation * scale * vec4(pos, 1.0);
}
