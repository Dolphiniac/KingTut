#pragma once

#include "Renderer.h"

class Image;
class Mesh;
class ShaderProgram;

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
	void SetRenderTargets( const Image * colorTarget, const Image * depthStencilTarget );
	void Draw( const Mesh * mesh, const ShaderProgram * shader );
	void Clear( bool doClearColor, bool doClearDepth, float clearR, float clearG, float clearB, float clearA, float clearDepth );

private:
	VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	pipelineDescription_t m_pipelineState = {};
	VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
	VkRect2D m_renderArea = {};

	bool m_inRenderPass = false;

private:
	CommandContext() = default;
};