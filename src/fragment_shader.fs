#version 460 core
#define MAX_LIGHTS 1

struct PointLight
{
	vec3 position;
	vec3 color;
	float intensity;
};

in vec3 v_normal;
in vec3 frag_position;

uniform vec4 color;
uniform PointLight point_lights[MAX_LIGHTS];

out vec4 frag_color;

vec4 calculate_point_light(PointLight point_light)
{
	vec3 direction = normalize(point_light.position - frag_position);
	float angle = dot(direction, v_normal);
	return vec4(point_light.color * point_light.intensity * angle, 1.0);
	/*vec3 norm = normalize(v_normal);
	vec3 direction = normalize(point_light.position - frag_position);
    	return vec4(point_light.color * point_light.intensity * max(dot(norm, direction), 0.0), 1.0);*/
}

void main()
{
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		result += calculate_point_light(point_lights[i]);
	}
	frag_color = color*result;
}
