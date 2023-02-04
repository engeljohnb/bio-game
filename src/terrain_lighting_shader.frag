#version 410 core

in vec2 f_tex_coords;
out vec4 frag_color;
uniform sampler2D f_normal_texture;
uniform sampler2D f_position_texture;

struct PointLight
{
	vec3 	position;
	vec3	color;
	float	intensity;
};


vec4 calculate_point_light(PointLight point_light)
{
	vec3 f_position = vec3(texture(f_position_texture, f_tex_coords));
	vec3 f_normal = vec3(texture(f_normal_texture, f_tex_coords));
	vec3 direction = normalize(point_light.position - f_position);
	float angle = max(dot(direction, f_normal), 0.0f); 
	float dist = distance(point_light.position, f_position);
	float intensity = point_light.intensity / (dist*dist);
	return vec4(point_light.color * intensity * angle, 1.0f);
}

void main()
{
	PointLight point_light;
	point_light.position = vec3(4.0f, 4.0f, 0.0f);
	point_light.color = vec3(1.0f, 1.0f, 1.0f);
	point_light.intensity = 10.0f;

	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	result += calculate_point_light(point_light);
	frag_color = result * vec4(1.0, 0.0, 0.0, 1.0);
}
