#pragma once

#include "Renderer.h"

const uint32_t RENDER_TARGET_COUNT = 2;

class Image;
class Mesh;
class ShaderProgram;

struct renderPassDescription_t {
	uint32_t colorTargetCount;
	imageFormat_t colorFormats[ RENDER_TARGET_COUNT ];
	imageFormat_t depthFormat;
	bool useDepth;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	bool operator ==( const renderPassDescription_t & other ) const {
		if ( colorTargetCount != other.colorTargetCount ) {
			return false;
		}
		if ( useDepth != other.useDepth ) {
			return false;
		}
		for ( uint32_t i = 0; i < colorTargetCount + ( useDepth ? 1 : 0 ); ++i ) {
			if ( colorFormats[ i ] != other.colorFormats[ i ] ) {
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
	void SetRenderTargets( uint32_t colorTargetCount, const Image * colorTargets[ RENDER_TARGET_COUNT ], const Image * depthStencilTarget );
	void Draw( const Mesh * mesh, const ShaderProgram * shader );

private:
	VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	pipelineDescription_t m_pipelineState = {};

	bool m_inRenderPass = false;

private:
	CommandContext() = default;
};