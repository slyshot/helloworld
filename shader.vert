#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 5) in vec2 texPos;
layout(location = 1) in mat4 instanceM;


layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv_pos;

layout(binding = 0) uniform MVP {
	mat4 vp;
} ubo;
void main() {
	gl_Position =  ubo.vp * (instanceM * vec4(inPosition,1.0));
	fragColor = inPosition;
	uv_pos = texPos;
//    fragColor = vec3(sin(inPosition[0]), sin(inPosition[1]), sin(inPosition[2]) );
}


