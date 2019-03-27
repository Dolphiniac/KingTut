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

	Mesh * fullscreenTri = Mesh::Create( fullscreenTriVerts, sizeof( fullscreenTriVerts ), fullscreenTriIndices, sizeof( fullscreenTriIndices ) );
	ShaderProgram * shader = ShaderProgram::Create( "simpleTri" );

	while ( true ) {
		renderObjects.commandContext->SetRenderTargets( renderObjects.colorImage, renderObjects.depthImage );
		renderObjects.commandContext->Clear( true, true, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f );
		// THE RENDER LOOP!!!!!!!!
		uint32_t imageIndex;
		result = vkAcquireNextImageKHR( barebonesRenderer.device, barebonesRenderer.swapchain, INFINITE, VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex );
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		result = vkBeginCommandBuffer( barebonesRenderer.commandBuffer, &commandBufferBeginInfo );
		VkClearValue clearColor = {};
		clearColor.color.float32[ 0 ] = 1.0f;
		clearColor.color.float32[ 1 ] = 1.0f;
		clearColor.color.float32[ 2 ] = 1.0f;
		clearColor.color.float32[ 3 ] = 1.0f;
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = barebonesRenderer.renderPass;
		renderPassBeginInfo.framebuffer = barebonesRenderer.framebuffers[ imageIndex ];
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = barebonesRenderer.swapchainExtent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass( barebonesRenderer.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( barebonesRenderer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, barebonesRenderer.pipeline );
		vkCmdDraw( barebonesRenderer.commandBuffer, 3, 1, 0, 0 );
		vkCmdEndRenderPass( barebonesRenderer.commandBuffer );
		result = vkEndCommandBuffer( barebonesRenderer.commandBuffer );
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &barebonesRenderer.commandBuffer;
		result = vkQueueSubmit( barebonesRenderer.queue, 1, &submitInfo, VK_NULL_HANDLE );
		result = vkQueueWaitIdle( barebonesRenderer.queue );
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &barebonesRenderer.swapchain;
		presentInfo.pImageIndices = &imageIndex;
		result = vkQueuePresentKHR( barebonesRenderer.queue, &presentInfo );
	}
}