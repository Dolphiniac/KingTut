#version 450 core

#include "global.glslh"

layout( location = LOC_POSITION ) in vec3 inPosition;
layout( location = LOC_UV ) in vec2 inUV0;

layout( location = 0 ) out vec2 outUV0;

void main() {
	gl_Position = vec4( inPosition, 1.0f );
	gl_Position.y *= -1.0f;
	outUV0 = inUV0;
}