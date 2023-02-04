#version 410 core

in vec2 f_tex_coords;
out vec4 frag_color;
uniform sampler2D f_texture;

void main()
{
	frag_color = vec4(texture(f_texture, f_tex_coords).rgb, 1.0f);
}
