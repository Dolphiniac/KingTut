#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "Renderer.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "CommandContext.h"
#include "Buffer.h"
#include "DescriptorSet.h"
#include <math.h>

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

	Vector3 ftl = {
		-0.5f,
		0.5f,
		-0.5f
	};
	Vector3 ftr = {
		0.5f,
		0.5f,
		-0.5f
	};
	Vector3 fbl = {
		-0.5f,
		-0.5f,
		-0.5f
	};
	Vector3 fbr = {
		0.5f,
		-0.5f,
		-0.5f
	};
	Vector3 btl = ftl;
	btl.z = 0.5f;
	Vector3 btr = ftr;
	btr.z = 0.5f;
	Vector3 bbl = fbl;
	bbl.z = 0.5f;
	Vector3 bbr = fbr;
	bbr.z = 0.5f;

	Vector2 tl = {
		0.0f,
		0.0f
	};
	Vector2 tr = {
		1.0f,
		0.0f
	};
	Vector2 bl = {
		0.0f,
		1.0f
	};
	Vector2 br = {
		1.0f,
		1.0f
	};

	Vector4 cftl = {
		0.0f,
		0.0f,
		0.0f,
		1.0f
	};
	Vector4 cftr = {
		1.0f,
		0.0f,
		0.0f,
		1.0f
	};
	Vector4 cfbl = {
		0.0f,
		1.0f,
		0.0f,
		1.0f
	};
	Vector4 cfbr = {
		1.0f,
		1.0f,
		0.0f,
		1.0f
	};
	Vector4 cbtl = cftl;
	cbtl.z = 1.0f;
	Vector4 cbtr = cftr;
	cbtr.z = 1.0f;
	Vector4 cbbl = cfbl;
	cbbl.z = 1.0f;
	Vector4 cbbr = cfbr;
	cbbr.z = 1.0f;

	vertex_t cubeVerts[] = {
		// Front face
		{
			ftl, tl, cftl
		},
		{
			ftr, tr, cftr
		},
		{
			fbl, bl, cfbl
		},
		{
			fbr, br, cfbr
		},
		// Right face
		{
			ftr, tl, cftr
		},
		{
			btr, tr, cbtr
		},
		{
			fbr, bl, cfbr
		},
		{
			bbr, br, cbbr
		},
		// Left face
		{
			btl, tl, cbtl
		},
		{
			ftl, tr, cftl
		},
		{
			bbl, bl, cbbl
		},
		{
			fbl, br, cfbl
		},
		// Back face
		{
			btr, tl, cbtr
		},
		{
			btl, tr, cbtl
		},
		{
			bbr, bl, cbbr
		},
		{
			bbl, br, cbbl
		},
		// Top face
		{
			btl, tl, cbtl
		},
		{
			btr, tr, cbtr
		},
		{
			ftl, bl, cftl
		},
		{
			ftr, br, cftr
		},
		// Bottom face
		{
			fbl, tl, cfbl
		},
		{
			fbr, tr, cfbr
		},
		{
			bbl, bl, cbbl
		},
		{
			bbr, br, cbbr
		}
	};

	uint16_t cubeIndices[] = {
		// Front
		0, 2, 3,
		0, 3, 1,
		// Right
		4, 6, 7,
		4, 7, 5,
		// Left
		8, 10, 11,
		8, 11, 9,
		// Back
		12, 14, 15,
		12, 15, 13,
		// Top
		16, 18, 19,
		16, 19, 17,
		// Bottom
		20, 22, 23,
		20, 23, 21
	};

	// Fullscreen triangle for alternate blit from color to swapchain.
	Mesh * tri = Mesh::Create( fullscreenTriVerts, sizeof( fullscreenTriVerts ), fullscreenTriIndices, sizeof( fullscreenTriIndices ), ARRAY_COUNT( fullscreenTriIndices ) );
	// Cube mesh for perspective draw.
	Mesh * cube = Mesh::Create( cubeVerts, sizeof( cubeVerts ), cubeIndices, sizeof( cubeIndices ), ARRAY_COUNT( cubeIndices ) );
	// Matching shaders for above meshes.
	ShaderProgram * triShader = ShaderProgram::Create( "simpleTri" );
	ShaderProgram * meshShader = ShaderProgram::Create( "simpleMesh" );

	// To test uniform buffers, we make MVP matrices and bind them to different descriptor scopes.
	float nearZ = 0.2f;
	float farZ = 100.0f;
	float f = 1.0f / tanf( 90.0f * 3.14159f / 360.0f );
	float aspect = ( float )renderObjects.colorImage->GetWidth() / ( float )renderObjects.colorImage->GetHeight();

	Matrix44 projection = {
		f / aspect, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, -( farZ + nearZ ) / ( nearZ - farZ ), 1.0f,
		0.0f, 0.0f, ( 2.0f * farZ * nearZ ) / ( nearZ - farZ ), 0.0f
	};

	Matrix44 view = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	Matrix44 model = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		1.5f, 0.0f, 3.0f, 1.0f
	};

	// The objects to hold the data.
	Buffer * projectionBuffer = Buffer::Create( &projection, sizeof( projection ), BUFFER_USAGE_UNIFORM_BUFFER );
	Buffer * viewBuffer = Buffer::Create( &view, sizeof( view ), BUFFER_USAGE_UNIFORM_BUFFER );
	Buffer * modelBuffer = Buffer::Create( &model, sizeof( model ), BUFFER_USAGE_UNIFORM_BUFFER );

	// The descriptor sets that bind the resources to slots in the shaders.
	DescriptorSet * frameSet = DescriptorSet::Allocate( DESCRIPTOR_SCOPE_FRAME );
	DescriptorSet * viewSet = DescriptorSet::Allocate( DESCRIPTOR_SCOPE_VIEW );
	DescriptorSet * meshSet = DescriptorSet::Allocate( DESCRIPTOR_SCOPE_MESH );

	// Setting the resources on the sets as an initialization step.
	frameSet->SetUniformBuffer( FRAME_DESCRIPTOR_UNIFORM_BUFFER_SLOT_0, projectionBuffer );
	viewSet->SetUniformBuffer( VIEW_DESCRIPTOR_UNIFORM_BUFFER_SLOT_0, viewBuffer );
	meshSet->SetUniformBuffer( MESH_DESCRIPTOR_UNIFORM_BUFFER_SLOT_0, modelBuffer );

	// Sampled image to test texture descriptor and staging pipeline.
	Image * vulkanImage = Image::CreateFromFile( "vulkanLogo.jpg" );
	meshSet->SetImageSampler( MESH_DESCRIPTOR_SAMPLER_SLOT_0, SAMPLER_TYPE_LINEAR, vulkanImage );

	// Sampled attachment to test mid-frame layout transition.
	DescriptorSet * triSet = DescriptorSet::Allocate( DESCRIPTOR_SCOPE_MESH );
	triSet->SetImageSampler( MESH_DESCRIPTOR_SAMPLER_SLOT_0, SAMPLER_TYPE_LINEAR, renderObjects.colorImage );

	while ( true ) {
		// Local pointer variables to make writing the render loop more succinct.
		CommandContext * context = renderObjects.commandContext;
		Image * colorImage = renderObjects.colorImage;
		Image * depthImage = renderObjects.depthImage;
		Image * swapchainImage = renderObjects.swapchainImage;

		Renderer_BeginFrame();
		// We bind descriptor sets at different frequencies based on scope.  In a single-view scene, frame and view
		// sets have to be bound exactly once per context.
		context->PipelineBarrier( colorImage, IMAGE_LAYOUT_COLOR_ATTACHMENT, BARRIER_DISCARD_AND_IGNORE_OLD_LAYOUT );
		context->PipelineBarrier( depthImage, IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT, BARRIER_DISCARD_AND_IGNORE_OLD_LAYOUT );
		context->BindDescriptorSet( frameSet );
		context->BindDescriptorSet( viewSet );
		context->SetRenderTargets( colorImage, depthImage );
		context->Clear( true, true, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f );
		context->SetViewportAndScissor( colorImage->GetWidth(), colorImage->GetHeight() );
		context->BindDescriptorSet( meshSet );
		context->Draw( cube, meshShader );
		Renderer_AcquireSwapchainImage();
		// Transition the color image to a readable state and swapchain image to writable.
		context->PipelineBarrier( colorImage, IMAGE_LAYOUT_FRAGMENT_SHADER_READ, BARRIER_NONE );
		context->PipelineBarrier( swapchainImage, IMAGE_LAYOUT_COLOR_ATTACHMENT, BARRIER_DISCARD_AND_IGNORE_OLD_LAYOUT );
		context->SetRenderTargets( swapchainImage, NULL );
		context->SetViewportAndScissor( swapchainImage->GetWidth(), swapchainImage->GetHeight() );
		context->BindDescriptorSet( triSet );
		context->Draw( tri, triShader );
		// Transition the swapchain image to presentable state.
		context->PipelineBarrier( swapchainImage, IMAGE_LAYOUT_PRESENT, BARRIER_NONE );
		Renderer_EndFrame();
	}
}