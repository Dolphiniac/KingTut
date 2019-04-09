#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Renderer.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "CommandContext.h"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {
	// Like for any engine subsystem, an init function should do all of the work to validate access, so
	// it's sufficient just to call this before doing anything else render-y.
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
		// All rendering code will be executed between BeginFrame and EndFrame.  The programmer writing techniques
		// doesn't need to worry about how to set up the frame, just how to set up passes and draws.
		Renderer_BeginFrame();
		renderObjects.commandContext->SetRenderTargets( renderObjects.colorImage, renderObjects.depthImage );
		renderObjects.commandContext->Clear( true, true, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f );	// Clear is stateful.  It depends on the set render targets
		renderObjects.commandContext->SetViewportAndScissor( renderObjects.colorImage->GetWidth(), renderObjects.colorImage->GetHeight() );
		renderObjects.commandContext->Draw( fullscreenTri, shader );
		Renderer_AcquireSwapchainImage();	// Best practice is to wait as long as possible to call this, but it must be before the first use of the swapchain image
		renderObjects.commandContext->Blit( renderObjects.colorImage, renderObjects.swapchainImage );
		Renderer_EndFrame();
	}
}