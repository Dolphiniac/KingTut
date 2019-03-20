#include "CommandCoAntext.h"
#include <vector>

struct renderPassDescription_t {
	uint32_t colorTargetCount;
	imageFormat_t colorFormats[ RENDER_TARGET_COUNT ];
	imageFormat_t depthFormat;
	bool useDepth;
	VkRenderPass renderPass = VK_NULL_HANDLE;

	bool operator ==( const renderPassDescription_t & other ) {
		if ( colorTargetCount != other.colorTargetCount ) {
			return false;
		}
		if ( useDepth != other.useDepth ) {
			return false;
		}
		for ( uint32_t i = 0; i < colorTargetCount + ( useDepth ? 1 : 0 ); ++i ) {
			if ( formats[ i ] != other.formats[ i ] ) {
				return false;
			}
		}
		return true;
	}
	bool operator !=( const renderPassDescription_t & other ) {
		return !( *this == other );
	}
};

static std::vector< renderPassDescription_t > createdPasses;

static VkRenderPass CreateRenderPass( const renderPassDescription_t & description ) {
	renderPassDescription_t newDesc = description;
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = newDesc.colorTargetCount + ( newDesc.useDepth ? 1 : 0 );
	std::vector< VkAttachmentDescription > attachmentDescriptions;
	std::vector< VkAttachmentReference > attachmentReferences;
	attachmentDescriptions.resize( renderPassCreateInfo.attachmentCount );
	attachmentReferences.resize( renderPassCreateInfo.attachmentCount );
	extern VkFormat TranslateFormat( imageFormat_t format );
	for ( uint32_t i = 0; i < newDesc.colorTargetCount; ++i ) {
		VkAttachmentDescription & attachmentDescription = attachmentDescriptions[ i ];
		attachmentDescription = {};
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescription.format = TranslateFormat( description.colorFormats[ i ] );
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentReference & attachmentReference = attachmentReferences[ i ];
		attachmentReference.attachment = i;
		attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if ( newDesc.useDepth == true ) {
		VkAttachmentDescription & attachmentDescription = attachmentDescriptions[ newDesc.colorTargetCount ];
		attachmentDescription = {};
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescription.format = TranslateFormat( description.depthFormat );
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentReference & attachmentReference = attachmentReferences[ newDesc.colorTargetCount ];
		attachmentReference.attachment = newDesc.colorTargetCount;
		attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	VkSubpassDescription subpassDescription = {};
	subpassDescription.colorAttachmentCount = newDesc.colorTargetCount;
	subpassDescription.pColorAttachments = attachmentReferences.data();
	if ( newDesc.useDepth == true ) {
		subpassDescription.pDepthStencilAttachment = &attachmentReferences[ newDesc.colorTargetCount ];
	}
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	
	VK_CHECK( vkCreateRenderPass( renderObjects.device, &renderPassCreateInfo, NULL, &newDesc.renderPass ) );

	createdPasses.push_back( newDesc );
	return newDesc.renderPass;
}

struct framebufferDescription_t {
	uint32_t width;
	uint32_t height;
	uint32_t colorTargetCount;
	VkImageView colorViews[ RENDER_TARGET_COUNT ];
	VkImageView depthStencilView;

	VkFramebuffer framebuffer;

	bool operator ==( const framebufferDescription_t & other ) {
		if ( width != other.width ) {
			return false;
		}
		if ( height != other.height ) {
			return false;
		}
		if ( colorTargetCount != other.colorTargetCount ) {
			return false;
		}
		for ( uint32_t i = 0; i < colorTargetCount; ++i ) {
			if ( colorViews != other.colorViews[ i ] ) {
				return false;
			}
		}
		if ( depthStencilView != other.depthStencilView ) {
			return false;
		}
		return true;
	}

	bool operator !=( const framebufferDescription_t & other ) {
		return !( *this == other );
	}
};

std::vector< framebufferDescription_t > createdFramebuffers;

static VkFramebuffer CreateFramebuffer( const framebufferDescription_t & description, VkRenderPass renderPass ) {
	framebufferDescription_t newDesc = description;

	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.width = newDesc.width;
	framebufferCreateInfo.height = newDesc.height;
	framebufferCreateInfo.layers = 1;
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = newDesc.colorTargetCount + ( newDesc.depthStencilView != VK_NULL_HANDLE ? 1 : 0 );

	std::vector< VkImageView > attachments;
	attachments.resize( newDesc.colorTargetCount );
	memcpy( attachments.data(), newDesc.colorViews, sizeof( VkImageView ) * newDesc.colorTargetCount );
	if ( newDesc.depthStencilView != VK_NULL_HANDLE ) {
		attachments.push_back( newDesc.depthStencilView );
	}
	framebufferCreateInfo.pAttachments = attachments.data();
	VK_CHECK( vkCreateFramebuffer( renderObjects.device, &framebufferCreateInfo, NULL, &newDesc.framebuffer ) );

	return newDesc.framebuffer;
}

CommandContext * CommandContext::Create() {
	CommandContext * result = new CommandContext;
	VkCommandPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolCreateInfo.queueFamilyIndex = renderObjects.queueFamilyIndex;
	VK_CHECK( vkCreateCommandPool( renderObjects.device, &poolCreateInfo, NULL, &m_commandPool ) );

	VkCommandBufferAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocateInfo.commandPool = m_commandPool;
	bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferAllocateInfo.commandBufferCount = COMMAND_BUFFER_COUNT;
	VK_CHECK( vkAllocateCommandBuffers( renderObjects.device, &bufferAllocateInfo, m_commandBuffers ) );

	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;	// We'll create the first fence
	VK_CHECK( vkCreateFence( renderObjects.device, &fenceCreateInfo, NULL, &m_fences[ 0 ] ) );
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for ( uint32_t i = 1; i < COMMAND_BUFFER_COUNT; ++i ) {
		VK_CHECK( vkCreateFence( renderObjects.device, &fenceCreateInfo, NULL, &m_fences[ i ] ) );
	}

	m_commandBuffer = m_commandBuffers[ 0 ];
	m_fence = m_fences[ 0 ];
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK( vkBeginCommandBuffer( m_commandBuffer, &beginInfo ) );
}

void CommandContext::SetRenderTargets( uint32_t colorTargetCount, Image * colorTargets[ RENDER_TARGET_COUNT ], Image * depthStencilTarget ) {
	renderPassDescription_t renderPassDescription;
	renderPassDescription.colorTargetCount = colorTargetCount;
	renderPassDescription.useDepth = depthStencilTarget != NULL;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	for ( size_t i = 0; i < createdPasses.size(); ++i ) {
		if ( renderPassDescription == createdPasses[ i ] ) {
			renderPass = createdPasses[ i ].renderPass;
			break;
		}
	}
	if ( renderPass == VK_NULL_HANDLE ) {
		renderPass = CreateRenderPass( renderPassDescription );
	}

	framebufferDescription_t framebufferDescription = {};	// Important, so that depthStencilView is defaulted to VK_NULL_HANDLE
	if ( colorTargetCount > 0 ) {
		framebufferDescription.width = colorTargets[ 0 ]->GetWidth();
		framebufferDescription.height = colorTargets[ 0 ]->GetHeight();
	} else {
		// We will expect the depth attachment to be valid here
		framebufferDescription.width = depthStencilTarget->GetWidth();
		framebufferDescription.height = depthStencilTarget->GetHeight();
	}
	framebufferDescription.colorTargetCount = colorTargetCount;
	for ( uint32_t i = 0; i < colorTargetCount; ++i ) {
		framebufferDescription.colorViews[ i ] = colorTargets[ i ]->GetView();
	}
	if ( depthStencilTarget != NULL ) {
		framebufferDescription.depthStencilView = depthStencilTarget->GetView();
	}
	VkFramebuffer framebuffer = VK_NULL_HANDLE;
	for ( size_t i = 0; i < createdFramebuffers.size(); ++i ) {
		if ( framebufferDescription == createdFramebuffers[ i ] ) {
			framebuffer = createdFramebuffers[ i ].framebuffer;
			break;
		}
	}
	if ( framebuffer == VK_NULL_HANDLE ) {
		framebuffer = CreateFramebuffer( framebufferDescription, renderPass );
	}

	if ( m_inRenderPass == true ) {
		vkCmdEndRenderPass( m_commandBuffer );
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = framebufferDescription.width;
	renderPassBeginInfo.renderArea.extent.height = framebufferDescription.height;

	vkCmdBeginRenderPass( m_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
	m_inRenderPass = true;
}