#version 460 core

in vec3 v_normal;
in vec3 frag_position;
uniform vec4 color;
out vec4 frag_color;
void main()
{	
	frag_color = color;
}
