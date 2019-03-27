#version 450 core

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec2 inUV0;
layout( location = 2 ) in vec4 inColor;

layout( location = 0 ) out vec4 outColor;

void main() {
	gl_Position = vec4( inPosition, 1.0f );
	gl_Position.y *= -1.0f;
	outColor = inColor;
}