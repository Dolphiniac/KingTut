#version 450 core

#include "global.glslh"

layout( set = SCOPE_FRAME, binding = FRAME_UNIFORM_BUFFER_SLOT_0 ) uniform FrameData {
	layout( row_major ) mat4 gProjection;
};

layout( set = SCOPE_VIEW, binding = VIEW_UNIFORM_BUFFER_SLOT_0 ) uniform ViewData {
	layout( row_major ) mat4 gView;
};

layout( set = SCOPE_MESH, binding = MESH_UNIFORM_BUFFER_SLOT_0 ) uniform MeshData {
	layout( row_major ) mat4 gModel;
};

layout( location = LOC_POSITION ) in vec3 inPosition;
layout( location = LOC_UV ) in vec2 inUV0;
layout( location = LOC_COLOR ) in vec4 inColor;

layout( location = 0 ) out vec2 outUV0;
layout( location = 1 ) out vec4 outColor;

void main() {
	vec4 position = vec4( inPosition, 1.0f );
	position *= gModel * gView * gProjection;
	gl_Position = position;
	gl_Position.y *= -1.0f;

	outUV0 = inUV0;
	outColor = inColor;
}