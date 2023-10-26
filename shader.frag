//taken from vkguide.dev for hello triangle
//glsl version 4.5
#version 450

//output write
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;
layout(location = 1) in vec2 uv_pos;
layout(binding = 1, set = 1) uniform sampler2D cube_texture;
layout(set = 1, binding = 2) uniform TextureSelectorBlock {
    vec3 texture_selector;
} selector;
void main()
{

//	outColor = vec4(fragColor,1.0);
/*
	outColor[0] = step(fragColor[0],0.5);
	outColor[1] = step(fragColor[1],0.5);
	outColor[2] = step(fragColor[2],0.5);
	outColor[3] = 1.0;
*/
/*
	outColor[0] = smoothstep(0.3,0.6,texColor[0]);
	outColor[1] = smoothstep(0.3,0.6,texColor[1]);
	outColor[2] = smoothstep(0.3,0.6,texColor[2]);
*/
	vec2 new_pos;
	vec4 texColor = texture(cube_texture, vec2(uv_pos[0] + 1.0/selector.texture_selector[2] * (selector.texture_selector[0] - 1), uv_pos[1] + 1.0/selector.texture_selector[2] * (selector.texture_selector[1] - 1)));
//	vec4 texColor = texture(cube_texture, vec2(uv_pos[0] * 1.0/selector.texture_selector[2] + 1.0/selector.texture_selector[0], uv_pos[1] * 1.0/selector.texture_selector[2] + 1.0/selector.texture_selector[1]));
//	vec4 texColor = texture(cube_texture, vec2(uv_pos[0] * 1.0/2.0 + 1.0/2.0, uv_pos[1] * 1.0/2 + 1.0/2.0));
	outColor = texColor;
	outColor[3] = 1.0;
}
