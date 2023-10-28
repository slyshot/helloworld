//taken from vkguide.dev for hello triangle
//glsl version 4.5
#version 450

//output write
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 uv_pos;
layout(binding = 1, set = 1) uniform sampler2D cube_texture;
layout(location = 2) in vec3 texture_selector;
void main()
{

	vec4 texColor = texture(cube_texture, vec2(uv_pos[0] + 1.0/texture_selector[2] * (texture_selector[0] - 1), uv_pos[1] + 1.0/texture_selector[2] * (texture_selector[1] - 1)));
	outColor = texColor;
	outColor[3] = 1.0;
}
