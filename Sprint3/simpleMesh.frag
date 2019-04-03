#version 450 core

#include "global.glslh"

layout( set = SCOPE_MESH, binding = MESH_SAMPLER_SLOT_0 ) uniform sampler2D gTexture;

layout( location = 0 ) in vec2 inUV0;
layout( location = 1 ) in vec4 inColor;

layout( location = 0 ) out vec4 outColor;

void main() {
	outColor = inColor * texture( gTexture, inUV0 );
}