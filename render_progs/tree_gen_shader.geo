#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 102) out;

uniform mat4 projection_view;
uniform float scale_factor;
uniform vec4 frustum_planes[6];

in VS_OUT
{
	vec3 g_base_offset;
	vec3 g_group_offset;
} gs_in[];

out vec3 f_normal;
out vec3 f_position;

float rand(vec2 n) 
{ 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

vec3 get_middle(vec3 a, vec3 b, vec3 c)
{
	float dist_ab = distance(a, b)/2.0;
	float dist_ac = distance(a, c)/2.0;
	vec3 dir_ab = normalize(b-a);
	vec3 dir_ac = normalize(c-a);

	return a + (dir_ab*dist_ab) + (dir_ac*dist_ac);
}

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

vec3 get_normal(vec3 a, vec3 b, vec3 c)
{
	return normalize(cross((b-a), (c-a)));
}

float signed_distance_to_plane(vec3 p, vec4 plane)
{
        vec3 point_on_plane = plane.xyz * -plane.w;
        return dot(plane.xyz, p-point_on_plane);
}

bool on_or_beyond_plane(vec3 p, vec4 plane)
{
        return (signed_distance_to_plane(p, plane) > 0.0);
}

bool in_frustum(vec3 pos)
{
        for (int i = 0; i < 6; ++i)
        {
                if (!on_or_beyond_plane(pos, frustum_planes[i]))
                {
                        return true;
                }
        }
        return false;
}

void emit_position(vec3 pos)
{
	f_position = pos;
	gl_Position = projection_view*vec4(pos, 1.0);
	EmitVertex();
}

float angle(vec3 a, vec3 b)
{
	return dot(normalize(a), normalize(b));
}

void sort_clockwise3(vec3 src[3])
{
	vec3 middle = (src[0]+src[1]+src[2])/3.0;	
	bool swapped = false;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < 3 - i - 1; ++j)
		{
			if (angle(middle, src[j]) > angle(middle, src[j+1]))
			{
				vec3 tmp = src[j];
				src[j] = src[j+1];
				src[j+1] = tmp;
				swapped = true;
			}
		}
		if (!swapped)
		{
			break;
		}
	}
}

void sort_clockwise4(vec3 src[4])
{
	vec3 middle = (src[0]+src[1]+src[2]+src[3])/4.0;	
	bool swapped = false;
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 4 - i - 1; ++j)
		{
			if (angle(middle, src[j]) > angle(middle, src[j+1]))
			{
				vec3 tmp = src[j];
				src[j] = src[j+1];
				src[j+1] = tmp;
				swapped = true;
			}
		}
		if (!swapped)
		{
			break;
		}
	}
}

void emit_triangle(vec3 a, vec3 b, vec3 c)
{
	emit_position(a);
	emit_position(b);
	emit_position(c);
	EndPrimitive();
}

void connect_line_segments(vec3 a0, vec3 b0, vec3 a1, vec3 b1)
{
	vec3 corners[4];
	corners[0] = a0;
	corners[1] = a1;
	corners[2] = b0;
	corners[3] = b1;

	sort_clockwise4(corners);

	f_normal = get_normal(corners[1], corners[3], corners[0]);
	emit_position(corners[1]);
	emit_position(corners[3]);
	emit_position(corners[0]);
	EndPrimitive();

	f_normal = get_normal(corners[0], corners[2], corners[3]);
	emit_position(corners[0]);
	emit_position(corners[2]);
	emit_position(corners[3]);
	EndPrimitive();
}

vec3[2] interpolate_line_segments(vec3 a0, vec3 b0, vec3 a1, vec3 b1)
{
	vec3 dest[2];
	dest[0] = mix(a0, a1, 0.5);
	dest[1] = mix(b0, b1, 0.5);
	return dest;
}

vec3[3] interpolate_triangles(vec3 a0, vec3 b0, vec3 c0, vec3 a1, vec3 b1, vec3 c1)
{
	vec3 dest[3];
	dest[0] = mix(a0, a1, 0.5);
	dest[1] = mix(b0, b1, 0.5);
	dest[2] = mix(c0, c1, 0.5);
	return dest;
}

vec3 [2]connect_triangles(vec3 a0, vec3 b0, vec3 c0, vec3 a1, vec3 b1, vec3 c1, float inbt_offset)
{
	vec3 corners0[3];
	vec3 corners1[3];

	corners0[0] = a0;
	corners0[1] = b0;
	corners0[2] = c0;

	corners1[0] = a1;
	corners1[1] = b1;
	corners1[2] = c1;

	vec3 inbetween[2] = interpolate_line_segments(corners0[0], corners0[1], corners1[0], corners1[1]);
	inbetween[0] += inbt_offset;
	inbetween[1] += inbt_offset;

	connect_line_segments(corners0[0], corners0[1], inbetween[0], inbetween[1]);
	connect_line_segments(inbetween[0], inbetween[1], corners1[0], corners1[1]);
	//f_color = vec3(0.0, 1.0, 0.0);
	connect_line_segments(corners0[1], corners0[2], inbetween[0], inbetween[1]);
	connect_line_segments(inbetween[0], inbetween[1], corners1[1], corners1[2]);
	//f_color = vec3(0.0, 0.0, 1.0);
	connect_line_segments(corners0[2], corners0[0], inbetween[0], inbetween[1]);
	connect_line_segments(inbetween[0], inbetween[1], corners1[2], corners1[2]);
	return inbetween;
}

void main()
{
	vec3 base_position = gs_in[0].g_base_offset;
	vec3 group_position = gs_in[0].g_group_offset;
	vec3 branch_end_corners[3];

	//f_color = vec3(0.3, 0.2, 0.1);
	for (int i = 0; i < gl_in.length(); i++)
	{
		f_normal = -normalize((base_position + group_position) - base_position);

		mat4 scaled = scale(vec3(scale_factor/5.0));
		mat4 rotated = rotate(vec3(cross(vec3(0.0, 1.0, 0.0), f_normal)), acos(dot(vec3(0.0, 1.0, 0.0), f_normal)));
		mat4 translated = translate(gs_in[i].g_base_offset + gs_in[i].g_group_offset);

		vec3 trans = vec3(translated * rotated * scaled * gl_in[i].gl_Position);
		vec4 pos = projection_view * vec4(trans, 1.0);
		gl_Position = pos;
		f_position = trans;
		branch_end_corners[i] = trans;
		EmitVertex();
	}
	EndPrimitive();

	vec3 trunk_corners[3];
	trunk_corners[0] = base_position;
	trunk_corners[1] = base_position;
	trunk_corners[2] = base_position;

	trunk_corners[0].x += 2.0*scale_factor;
	trunk_corners[1].z -= 2.0*scale_factor;

	sort_clockwise3(trunk_corners);
	sort_clockwise3(branch_end_corners);

	vec3 inbetween[3] = interpolate_triangles(trunk_corners[0], trunk_corners[1], trunk_corners[2],
					       branch_end_corners[0], branch_end_corners[1], branch_end_corners[2]);	

	float randval = rand(vec2(15.0)) * 25.0;

	vec3 con_points[2] = connect_triangles(trunk_corners[0], trunk_corners[1], trunk_corners[2],
			  		    inbetween[0], inbetween[1], inbetween[2], randval);

	connect_triangles(inbetween[0], inbetween[1], inbetween[2],
			  branch_end_corners[0], branch_end_corners[1], branch_end_corners[2], randval);

	vec3 trunk_bottom[3];
	trunk_bottom[0] = trunk_corners[0];
	trunk_bottom[1] = trunk_corners[1];
	trunk_bottom[2] = trunk_corners[2];

	trunk_bottom[0].y -= 300.0;
	trunk_bottom[1].y -= 300.0;
	trunk_bottom[2].y -= 300.0;

	connect_line_segments(trunk_corners[0], trunk_corners[1], trunk_bottom[0], trunk_bottom[1]);
	connect_line_segments(trunk_corners[1], trunk_corners[2], trunk_bottom[1], trunk_bottom[2]);
	connect_line_segments(trunk_corners[2], trunk_corners[0], trunk_bottom[2], trunk_bottom[0]);

	emit_triangle(trunk_corners[2], trunk_corners[0], con_points[0]);
}
