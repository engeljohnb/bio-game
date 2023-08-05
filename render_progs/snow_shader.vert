#version 430 core

layout (location = 0) in vec3 pos;

uniform float num_instances;
uniform float time;
uniform vec3 player_pos;
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
void main()
{
	vs_out.instance_id = gl_InstanceID;
	int size = int(sqrt(num_instances));
	int x_index =  gl_InstanceID % size;
	int z_index =  gl_InstanceID / size;
	float x_percent = float(x_index)/float(size);
	float z_percent = float(z_index)/float(size);
	vec3 player_pos_xz = vec3(player_pos.x, 0.0, player_pos.z);
	vec3 left = vec3(-100.0, 0.0, 0.0);
	vec3 right = vec3(100.0, 0.0, 0.0);
	vec3 forward = vec3(0.0, 0.0, 100.0);
	vec3 backward = vec3(0.0, 0.0, -100.0);

	float rand_num = fract(100000*sin(gl_InstanceID));
	float velocity = 0.5;
	float init_y_position = player_pos.y + 200.0 + (rand_num*800.0);
	float final_y_position = player_pos.y - 100.0;
	float total_time = (init_y_position - final_y_position)/velocity;
	float current_time = mod(time, total_time);
	float x_offset = sin(time/10.0)*rand_num;
	vec3 x = mix(left, right, x_percent) + x_offset;
	vec3 z = mix(backward, forward, z_percent);
	float height = mix(init_y_position, final_y_position, current_time/total_time);

	mat4 translation = translate(player_pos_xz) + translate(z) + translate(x) + translate(vec3(0.0, height, 0.0));
	mat4 rotation = rotate(vec3(0.0, 1.0, 0.0), rand_num);
	gl_Position = translation * rotation * scale * vec4(pos, 1.0);
}
