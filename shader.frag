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
	ivec2 texSize = textureSize(cube_texture, 0);
	//texture_selector[2] is size of atlas in terms of # of model textures on it.
	//texture_selector[0] and [1] are 'position', i.e, 1,1 represents the top left corner, 1,2 the bottom left, etc.
	vec2 pos = uv_pos;
	vec2 pos_out = vec2(pos[0] + 1.0/texture_selector[2] * (texture_selector[0] - 1), pos[1] + 1.0/texture_selector[2] * (texture_selector[1] - 1));
	//this didn't work, as 1/width would have been one pixel width, not 1/texture_selector[2]
	// pos[0] += (1.0/float(texSize[0]))*(texture_selector[0]);
	// pos[1] += (1.0/float(texSize[1]))*(texture_selector[1]);
	vec4 texColor = texture(cube_texture, pos_out);
	outColor = texColor;
	outColor[3] = 1.0;
}
