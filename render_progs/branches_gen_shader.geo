#version 430 core

#define MAX_TERRAIN_BLOCKS 100000

layout (triangles) in;
layout (triangle_strip, max_vertices = 102) out;

uniform mat4 projection_view;
uniform float scale_factor;
uniform float trunk_size;
uniform float branch_size;
uniform vec4 frustum_planes[6];

in VS_OUT
{
	vec3 g_base_offset;
	vec3 g_group_offset;
	float g_trunk_height;
	uint g_block;
	uint g_id;
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

void bend_branch(float num_joints,
		 float branch_size,
		 vec3 base_position,
		 vec3 e0, vec3 e1, vec3 e2, vec3 e3,
		 vec3 b0, vec3 b1, vec3 b2, vec3 b3)
{
	//vec3 base_position = gs_in[0].g_base_offset;
	vec3 group_position = gs_in[0].g_group_offset;

	vec3 h0 = e0;
	vec3 h1 = e1;
	vec3 h2 = e2;
	vec3 h3 = e3;
	for (int i = int(num_joints)-1; i > 0; --i)
	{
		vec3 i0 = base_position+group_position * ((1.0/num_joints) * i);
		i0.xz -= branch_size/2.0;
		vec3 i1 = i0;
		vec3 i2 = i0;
		vec3 i3 = i0;
		i1.z += branch_size;
		i2.xz += branch_size;
		i3.x += branch_size;

		vec3 perp = normalize(cross(vec3(0,1,0), group_position));

		i0 += 25.0*perp;
		i1 += 25.0*perp;
		i2 += 25.0*perp;
		i3 += 25.0*perp;

		connect_squares(h0, h1, i0, i1,
				h2, i2, h3, i3);
		h0 = i0;
		h1 = i1;
		h2 = i2;
		h3 = i3;
	}

	connect_squares(h0, h1, b0, b1,
			h2, b2, h3, b3);
}

void main()
{
	vec3 base_position = gs_in[0].g_base_offset;
	vec3 group_position = gs_in[0].g_group_offset;
	vec3 branch_end_corners[3];
	uint block = gs_in[0].g_block;

	float trunk_size = 40.0;
	float branch_size = 8.0;

	/* Intermediate bend point on the trunk */
	vec3 t = base_position;
	float x_block_fraction = float(block%MAX_TERRAIN_BLOCKS);
	float z_block_fraction = float(block/MAX_TERRAIN_BLOCKS);
	t.y -= rand(vec2(x_block_fraction, z_block_fraction)) * gs_in[0].g_trunk_height;
	t.xz -= trunk_size/2.0;
	t.xz -= (trunk_size*0.2);
	vec3 trunk_direction = t-base_position;
	vec3 trunk_intermed = base_position + (trunk_direction*0.6);

	/* Corners of the distal end of the branch. */
	vec3 e0 = base_position + group_position;
	e0.xz -= branch_size/2.0;
	vec3 e1 = e0;
	vec3 e2 = e0;
	vec3 e3 = e0;
	e1.z += branch_size;
	e2.xz += branch_size;
	e3.x += branch_size;

	/* Corners of the proximal base of the branch. */
	/* Every third branch branches off from lower on the trunk than odd branch instances-- which branch off the
	 * tip of the trunk. */
	vec3 b0 = trunk_intermed;
	if (gs_in[0].g_id % 3 != 0)
	{
		b0 = base_position;
	}

	b0.xz -= trunk_size/2.0;
	vec3 b1 = b0;
	vec3 b2 = b0;
	vec3 b3 = b0;
	b1.z += trunk_size;
	b2.xz += trunk_size;
	b3.x += trunk_size;

	if (gs_in[0].g_id % 3 != 0)
	{
		bend_branch(3.0,
			    20.0,
			    base_position,
			    e0, e1, e2, e3,
			    b0, b1, b2, b3);
	}
	else
	{
		bend_branch(3.0,
			    20.0,
			    trunk_intermed,
			    e0, e1, e2, e3,
			    b0, b1, b2, b3);
	}

}


