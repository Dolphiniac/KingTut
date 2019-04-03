#version 450 core

#include "global.glslh"

layout( set = SCOPE_MESH, binding = MESH_SAMPLER_SLOT_0 ) uniform sampler2D gTexture;

layout( location = 0 ) in vec2 inUV0;

layout( location = 0 ) out vec4 outColor;

void main() {
	outColor = texture( gTexture, inUV0 );
	//outColor = vec4( 1.0f );
}