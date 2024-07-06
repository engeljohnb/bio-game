#version 430 core

#define MAX_TERRAIN_BLOCKS 100000

layout (triangles) in;
layout (triangle_strip, max_vertices = 102) out;

uniform mat4 projection_view;
uniform float scale_factor;
uniform vec4 frustum_planes[6];

in VS_OUT
{
	vec3 g_base_offset;
	vec3 g_group_offset;
	float g_trunk_height;
	uint g_block;
} gs_in[];

out vec3 f_normal;
out vec3 f_position;

float rand(vec2 n) 
{ 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
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

void connect_squares(vec3 p0, vec3 p1, vec3 p2, vec3 p3,
		     vec3 p4, vec3 p5, vec3 p6, vec3 p7)
{
	connect_line_segments(p0, p1, p2, p3);
	connect_line_segments(p1, p4, p3, p5);
	connect_line_segments(p4, p6, p5, p7);
	connect_line_segments(p6, p0, p7, p2);
}

void make_trunk(vec3 base_pos, uint block)
{
	float height = gs_in[0].g_trunk_height;
	// Height from origin, not from ground
	float trunk_size = 40.0;
	float trunk_base_size = trunk_size + (trunk_size*0.2);
	vec3 tbl = base_pos;
	tbl.xz -= trunk_size/2.0;

	vec3 tbr = tbl;
	vec3 tfr = tbl;
	vec3 tfl = tbl;
	tfl.z += trunk_size;
	tfr.xz += trunk_size;
	tbr.x += trunk_size;

	vec3 bbl = tbl;
	bbl.y -= height;
	vec3 bfl = bbl;
	vec3 bfr = bbl;
	vec3 bbr = bbl;
	bfl.z += trunk_base_size;
	bfr.xz += trunk_base_size;
	bbr.x += trunk_base_size;

	vec3 ibl = tbl;
	float x_block_fraction = float(block%MAX_TERRAIN_BLOCKS);
	float z_block_fraction = float(block/MAX_TERRAIN_BLOCKS);
	ibl.y -= rand(vec2(x_block_fraction, z_block_fraction)) * height;
	ibl.xz -= trunk_size/2.0;
	ibl.xz -= (trunk_size*0.2);
	vec3 ifl = ibl;
	vec3 ifr = ibl;
	vec3 ibr = ibl;

	ifl.z += trunk_size;
	ifr.xz += trunk_size;
	ibr.x += trunk_size;

	connect_squares(tbl, tfl, ibl, ifl, 
			tfr, ifr, tbr, ibr);
	connect_squares(ibl, ifl, bbl, bfl, 
			ifr, bfr, ibr, bbr);
}

void main()
{
	vec3 base_position = gs_in[0].g_base_offset;

	make_trunk(base_position, gs_in[0].g_block);
}


