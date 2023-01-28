#version 410 core

in vec3 f_position;
in vec3 f_color;
in vec3 f_normal;
out vec4 frag_color;

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;
};

vec4 calculate_point_light(PointLight point_light)
{
	vec3 direction = normalize(point_light.position - f_position);
	float angle = max(dot(direction, f_normal), 0.0f); 
	return vec4(point_light.color * point_light.intensity * angle, 1.0f);
}

void main()
{
	PointLight point_light;
	point_light.position = vec3(4.0f, 4.0f, 0.0f);
	point_light.color = vec3(1.0f, 1.0f, 1.0f);
	point_light.intensity = 10.0f;

	vec4 result = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	result += calculate_point_light(point_light);

	frag_color = result*vec4(f_color, 1.0);
	//frag_color = vec4(f_normal, 1.0);

}
