#pragma once

#include "Renderer.h"
#include "Image.h"

class Mesh;
class ShaderProgram;
class DescriptorSet;

struct renderPassDescription_t {
	bool clearColor;
	bool clearDepth;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	const Image * color;
	const Image * depth;

	bool operator ==( const renderPassDescription_t & other ) const {
		if ( ( color == NULL ) != ( other.color == NULL ) ) {
			return false;
		}
		if ( clearColor != other.clearColor ) {
			return false;
		}
		if ( ( depth == NULL ) != ( other.depth == NULL ) ) {
			return false;
		}
		if ( clearDepth != other.clearDepth ) {
			return false;
		}
		if ( color != NULL ) {
			if ( color->GetFormat() != other.color->GetFormat() ) {
				return false;
			}
		}
		if ( depth != NULL ) {
			if ( depth->GetFormat() != other.depth->GetFormat() ) {
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

enum barrierFlags_t {
	BARRIER_NONE = BIT( 0 ),
	BARRIER_DISCARD_AND_IGNORE_OLD_LAYOUT = BIT( 1 )
};
inline barrierFlags_t operator |( barrierFlags_t left, barrierFlags_t right ) {
	return ( barrierFlags_t )( ( int )left | ( int )right );
}

class CommandContext {
public:
	static CommandContext * Create();
	void Begin();
	void End();
	void SetRenderTargets( Image * colorTarget, Image * depthStencilTarget );
	void SetViewportAndScissor( uint32_t width, uint32_t height );
	void Draw( const Mesh * mesh, const ShaderProgram * shader );
	void Clear( bool doClearColor, bool doClearDepth, float clearR, float clearG, float clearB, float clearA, float clearDepth );
	void BindDescriptorSet( const DescriptorSet * descriptorSet );
	void Blit( const Image * src, const Image * dst );
	void PipelineBarrier( Image * image, imageLayout_t newLayout, barrierFlags_t flags );
	void EndRenderPass();
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