#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Renderer.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "CommandContext.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {
	Renderer_Init();

	vertex_t fullscreenTriVerts[ 3 ] = {
		{
			{
				-1.0f,
				3.0f,
				0.0f,
			},
			{
				0.0f,
				-1.0f,
			},
			{
				1.0f,
				0.0f,
				0.0f,
				1.0f
			},
		},
		{
			{
				-1.0f,
				-1.0f,
				0.0f,
			},
			{
				0.0f,
				1.0f,
			},
			{
				0.0f,
				1.0f,
				0.0f,
				1.0f,
			},
		},
		{
			{
				3.0f,
				-1.0f,
				0.0f,
			},
			{
				2.0f,
				1.0f,
			},
			{
				0.0f,
				0.0f,
				1.0f,
				1.0f,
			},
		},
	};

	uint16_t fullscreenTriIndices[ 3 ] = {
		0,
		1,
		2,
	};

	Mesh * fullscreenTri = Mesh::Create( fullscreenTriVerts, sizeof( fullscreenTriVerts ), fullscreenTriIndices, sizeof( fullscreenTriIndices ), 3 );
	ShaderProgram * shader = ShaderProgram::Create( "simpleTri" );

	while ( true ) {
		Renderer_BeginFrame();
		renderObjects.commandContext->SetRenderTargets( renderObjects.colorImage, renderObjects.depthImage );
		renderObjects.commandContext->Clear( true, true, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f );
		renderObjects.commandContext->SetViewportAndScissor( renderObjects.colorImage->GetWidth(), renderObjects.colorImage->GetHeight() );
		renderObjects.commandContext->Draw( fullscreenTri, shader );
		Renderer_AcquireSwapchainImage();
		renderObjects.commandContext->Blit( renderObjects.colorImage, renderObjects.swapchainImage );
		Renderer_EndFrame();
	}
}