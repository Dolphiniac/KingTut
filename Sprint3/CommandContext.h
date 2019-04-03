#pragma once

#include "Renderer.h"
#include "Image.h"

class Mesh;
class ShaderProgram;
class DescriptorSet;

struct renderPassDescription_t {
	imageFormat_t colorFormat;
	bool useColor;
	bool clearColor;
	imageFormat_t depthFormat;
	bool useDepth;
	bool clearDepth;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	bool operator ==( const renderPassDescription_t & other ) const {
		if ( useColor != other.useColor ) {
			return false;
		}
		if ( clearColor != other.clearColor ) {
			return false;
		}
		if ( useDepth != other.useDepth ) {
			return false;
		}
		if ( clearDepth != other.clearDepth ) {
			return false;
		}
		if ( useColor == true ) {
			if ( colorFormat != other.colorFormat ) {
				return false;
			}
		}
		if ( useDepth == true ) {
			if ( depthFormat != other.depthFormat ) {
				return false;
			}
		}
		return true;
	}
	bool operator !=( const renderPassDescription_t & other ) const {
		return !( *this == other );
	}
};

struct pipelineDescription_t {
	bool operator ==( const pipelineDescription_t & other ) const {
		if ( renderPassState != other.renderPassState ) {
			return false;
		}
		if ( shader != other.shader ) {
			return false;
		}
		return true;
	}
	bool operator !=( const pipelineDescription_t & other ) const {
		return !( *this == other );
	}

	renderPassDescription_t renderPassState;
	const ShaderProgram * shader;
	VkPipeline pipeline = VK_NULL_HANDLE;
};

class CommandContext {
public:
	static CommandContext * Create();
	void Begin();
	void End();
	void SetRenderTargets( const Image * colorTarget, const Image * depthStencilTarget );
	void SetViewportAndScissor( uint32_t width, uint32_t height );
	void Draw( const Mesh * mesh, const ShaderProgram * shader );
	void Clear( bool doClearColor, bool doClearDepth, float clearR, float clearG, float clearB, float clearA, float clearDepth );
	void BindDescriptorSet( const DescriptorSet * descriptorSet );
	void Blit( const Image * src, const Image * dst );
	VkCommandBuffer GetCommandBuffer() const { return m_commandBuffer; }

private:
	VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	pipelineDescription_t m_pipelineState = {};
	VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
	VkRect2D m_renderArea = {};
	uint32_t m_viewportAndScissorWidth = 0;
	uint32_t m_viewportAndScissorHeight = 0;

	bool m_inRenderPass = false;

private:
	CommandContext() = default;
};