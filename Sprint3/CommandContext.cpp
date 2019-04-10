#include "CommandContext.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "DescriptorSet.h"
#include "Buffer.h"
#include <vector>

static std::vector< renderPassDescription_t > createdPasses;

static VkRenderPass CreateRenderPass( const renderPassDescription_t & description ) {
	renderPassDescription_t newDesc = description;
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = ( newDesc.color != NULL ? 1 : 0 ) + ( newDesc.depth != NULL ? 1 : 0 );
	std::vector< VkAttachmentDescription > attachmentDescriptions;
	std::vector< VkAttachmentReference > attachmentReferences;
	attachmentDescriptions.resize( renderPassCreateInfo.attachmentCount );
	attachmentReferences.resize( renderPassCreateInfo.attachmentCount );
	extern VkFormat TranslateFormat( imageFormat_t format );
	if ( newDesc.color != NULL ) {
		VkAttachmentDescription & attachmentDescription = attachmentDescriptions[ 0 ];
		attachmentDescription = {};
		// With the initialLayout matching the subpass layout and the finalLayout, the render pass will do nothing to the layout,
		// so it's paramount that the image is ALREADY in the correct layout before setting it as a render target.
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDescription.format = TranslateFormat( description.color->GetFormat() );
		attachmentDescription.loadOp = newDesc.clearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentReference & attachmentReference = attachmentReferences[ 0 ];
		attachmentReference.attachment = 0;
		attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if ( newDesc.depth != NULL ) {
		VkAttachmentDescription & attachmentDescription = attachmentDescriptions[ newDesc.color != NULL ? 1 : 0 ];
		attachmentDescription = {};
		// See the comment for initialLayout and finalLayout for color images above.
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachmentDescription.format = TranslateFormat( description.depth->GetFormat() );
		attachmentDescription.loadOp = newDesc.clearDepth ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		VkAttachmentReference & attachmentReference = attachmentReferences[ newDesc.color != NULL ? 1 : 0 ];
		attachmentReference.attachment = newDesc.color != NULL ? 1 : 0;
		attachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
	VkSubpassDescription subpassDescription = {};
	if ( newDesc.color != NULL ) {
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = attachmentReferences.data();
	}
	if ( newDesc.depth != NULL ) {
		subpassDescription.pDepthStencilAttachment = &attachmentReferences[ newDesc.color != NULL ? 1 : 0 ];
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
	VkImageView colorView;
	VkImageView depthStencilView;

	VkFramebuffer framebuffer;

	bool operator ==( const framebufferDescription_t & other ) const {
		if ( width != other.width ) {
			return false;
		}
		if ( height != other.height ) {
			return false;
		}
		if ( colorView != other.colorView ) {
			return false;
		}
		if ( depthStencilView != other.depthStencilView ) {
			return false;
		}
		return true;
	}

	bool operator !=( const framebufferDescription_t & other ) const {
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
	framebufferCreateInfo.attachmentCount = ( newDesc.colorView != VK_NULL_HANDLE ? 1 : 0 ) + ( newDesc.depthStencilView != VK_NULL_HANDLE ? 1 : 0 );

	std::vector< VkImageView > attachments;
	if ( newDesc.colorView != VK_NULL_HANDLE ) {
		attachments.push_back( newDesc.colorView );
	}
	if ( newDesc.depthStencilView != VK_NULL_HANDLE ) {
		attachments.push_back( newDesc.depthStencilView );
	}
	framebufferCreateInfo.pAttachments = attachments.data();
	VK_CHECK( vkCreateFramebuffer( renderObjects.device, &framebufferCreateInfo, NULL, &newDesc.framebuffer ) );

	return newDesc.framebuffer;
}

CommandContext * CommandContext::Create() {
	CommandContext * result = new CommandContext;

	VkCommandBufferAllocateInfo bufferAllocateInfo = {};
	bufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	bufferAllocateInfo.commandPool = renderObjects.commandPool;
	bufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	bufferAllocateInfo.commandBufferCount = 1;
	VK_CHECK( vkAllocateCommandBuffers( renderObjects.device, &bufferAllocateInfo, &result->m_commandBuffer ) );

	return result;
}

void CommandContext::Begin() {
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK( vkBeginCommandBuffer( m_commandBuffer, &beginInfo ) );

	m_pipelineState = {};
	m_inRenderPass = false;
}

void CommandContext::End() {
	if ( m_inRenderPass == true ) {
		vkCmdEndRenderPass( m_commandBuffer );
	}
	VK_CHECK( vkEndCommandBuffer( m_commandBuffer ) );
}

void CommandContext::SetRenderTargets( Image * colorTarget, Image * depthStencilTarget ) {
	renderPassDescription_t renderPassDescription;
	renderPassDescription.clearColor = false;
	renderPassDescription.clearDepth = false;
	renderPassDescription.color = colorTarget;
	renderPassDescription.depth = depthStencilTarget;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	for ( size_t i = 0; i < createdPasses.size(); ++i ) {
		if ( renderPassDescription == createdPasses[ i ] ) {
			renderPass = createdPasses[ i ].renderPass;
			break;
		}
	}
	m_pipelineState.renderPassState = renderPassDescription;
	if ( renderPass == VK_NULL_HANDLE ) {
		renderPass = CreateRenderPass( renderPassDescription );
	}
	m_pipelineState.renderPassState.renderPass = renderPass;

	framebufferDescription_t framebufferDescription = {};	// Important, so that depthStencilView is defaulted to VK_NULL_HANDLE
	if ( colorTarget != NULL ) {
		framebufferDescription.width = colorTarget->GetWidth();
		framebufferDescription.height = colorTarget->GetHeight();
	} else {
		// We will expect the depth attachment to be valid here
		framebufferDescription.width = depthStencilTarget->GetWidth();
		framebufferDescription.height = depthStencilTarget->GetHeight();
	}
	if ( colorTarget != NULL ) {
		framebufferDescription.colorView = colorTarget->GetView();
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
	m_framebuffer = framebuffer;	// Caching this so render passes can be re-begun

	if ( m_inRenderPass == true ) {
		vkCmdEndRenderPass( m_commandBuffer );
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.framebuffer = framebuffer;
	renderPassBeginInfo.renderPass = renderPass;
	renderPassBeginInfo.renderArea.extent.width = framebufferDescription.width;
	renderPassBeginInfo.renderArea.extent.height = framebufferDescription.height;
	m_renderArea = renderPassBeginInfo.renderArea;

	vkCmdBeginRenderPass( m_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
	m_inRenderPass = true;
	if ( colorTarget != NULL ) {
		colorTarget->SetLayout( IMAGE_LAYOUT_COLOR_ATTACHMENT );
	}
	if ( depthStencilTarget != NULL ) {
		depthStencilTarget->SetLayout( IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT );
	}
}

void CommandContext::SetViewportAndScissor( uint32_t width, uint32_t height ) {
	m_viewportAndScissorWidth = width;
	m_viewportAndScissorHeight = height;
}

static std::vector< pipelineDescription_t > createdPipelines;

static VkPipeline CreatePipeline( const pipelineDescription_t & pipelineState ) {
	pipelineDescription_t description = pipelineState;

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.renderPass = description.renderPassState.renderPass;
	pipelineCreateInfo.subpass = 0;
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	if ( description.renderPassState.depth != NULL ) {
		depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	}
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	if ( description.renderPassState.color != NULL ) {
		colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
	}
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VkVertexInputBindingDescription vertexInputBinding = {};
	vertexInputBinding.binding = 0;
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBinding.stride = sizeof( vertex_t );
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
	VkVertexInputAttributeDescription vertexInputAttributes[ 3 ] = {};
	vertexInputAttributes[ 0 ].binding = 0;
	vertexInputAttributes[ 0 ].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributes[ 0 ].location = 0;
	vertexInputAttributes[ 0 ].offset = offsetof( vertex_t, position );
	vertexInputAttributes[ 1 ].binding = 0;
	vertexInputAttributes[ 1 ].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributes[ 1 ].location = 1;
	vertexInputAttributes[ 1 ].offset = offsetof( vertex_t, uv );
	vertexInputAttributes[ 2 ].binding = 0;
	vertexInputAttributes[ 2 ].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributes[ 2 ].location = 2;
	vertexInputAttributes[ 2 ].offset = offsetof( vertex_t, color );
	vertexInputState.vertexAttributeDescriptionCount = 3;
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes;
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.minSampleShading = 1.0f;
	VkSampleMask sampleMask = 0xFFFFFFFF;
	multisampleState.pSampleMask = &sampleMask;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.lineWidth = 1.0f;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pTessellationState = NULL;
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	// Even though the spec says that dynamic viewport and scissor means these members are ignored in the viewport state,
	// validation will scream if there are aren't exactly one scissor and viewport here.
	VkViewport stubViewport = {};
	VkRect2D stubScissor = {};
	viewportState.scissorCount = 1;
	viewportState.pScissors = &stubScissor;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &stubViewport;
	pipelineCreateInfo.pViewportState = &viewportState;
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	dynamicState.dynamicStateCount = ARRAY_COUNT( dynamicStates );
	dynamicState.pDynamicStates = dynamicStates;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	VkPipelineShaderStageCreateInfo shaderStages[ 2 ];
	shaderStages[ 0 ] = {};
	shaderStages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[ 0 ].module = description.shader->GetVertexModule();
	shaderStages[ 0 ].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[ 0 ].pName = "main";
	shaderStages[ 1 ] = {};
	shaderStages[ 1 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[ 1 ].module = description.shader->GetFragmentModule();
	shaderStages[ 1 ].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[ 1 ].pName = "main";
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.layout = renderObjects.unifiedPipelineLayout;

	VK_CHECK( vkCreateGraphicsPipelines( renderObjects.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, NULL, &description.pipeline ) );

	createdPipelines.push_back( description );

	return description.pipeline;
}

void CommandContext::Draw( const Mesh * mesh, const ShaderProgram * shader ) {
	m_pipelineState.shader = shader;
	VkPipeline pipeline = VK_NULL_HANDLE;
	for ( size_t i = 0; i < createdPipelines.size(); ++i ) {
		if ( m_pipelineState == createdPipelines[ i ] ) {
			pipeline = createdPipelines[ i ].pipeline;
			break;
		}
	}
	if ( pipeline == VK_NULL_HANDLE ) {
		pipeline = CreatePipeline( m_pipelineState );
	}

	vkCmdBindPipeline( m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );
	VkViewport viewport = {};
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.width = ( float )m_viewportAndScissorWidth;
	viewport.height = ( float )m_viewportAndScissorHeight;
	vkCmdSetViewport( m_commandBuffer, 0, 1, &viewport );

	VkRect2D scissor = {};
	scissor.extent.width = m_viewportAndScissorWidth;
	scissor.extent.height = m_viewportAndScissorHeight;
	vkCmdSetScissor( m_commandBuffer, 0, 1, &scissor );
	VkBuffer vertexBuffer = mesh->GetVertexBuffer()->GetBuffer();
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers( m_commandBuffer, 0, 1, &vertexBuffer, &offset );
	VkBuffer indexBuffer = mesh->GetIndexBuffer()->GetBuffer();
	vkCmdBindIndexBuffer( m_commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16 );
	vkCmdDrawIndexed( m_commandBuffer, mesh->GetIndexCount(), 1, 0, 0, 0 );
}

void CommandContext::Clear( bool doClearColor, bool doClearDepth, float clearR, float clearG, float clearB, float clearA, float clearDepth ) {
	m_pipelineState.renderPassState.clearColor = doClearColor;
	m_pipelineState.renderPassState.clearDepth = doClearDepth;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	for ( size_t i = 0; i < createdPasses.size(); ++i ) {
		if ( createdPasses[ i ] == m_pipelineState.renderPassState ) {
			renderPass = createdPasses[ i ].renderPass;
			break;
		}
	}
	if ( renderPass == VK_NULL_HANDLE ) {
		renderPass = CreateRenderPass( m_pipelineState.renderPassState );
	}
	m_pipelineState.renderPassState.renderPass = renderPass;

	// We're expected to be in a render pass at this point (SetRenderTargets should have been called before this)
	vkCmdEndRenderPass( m_commandBuffer );

	VkRenderPassBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.framebuffer = m_framebuffer;
	beginInfo.renderPass = renderPass;
	beginInfo.renderArea = m_renderArea;
	VkClearValue clearValues[ 2 ] = {};
	uint32_t clearValueCount = 0;
	if ( doClearColor == true ) {
		VkClearColorValue & clearColor = clearValues[ clearValueCount ].color;
		clearColor.float32[ 0 ] = clearR;
		clearColor.float32[ 1 ] = clearG;
		clearColor.float32[ 2 ] = clearB;
		clearColor.float32[ 3 ] = clearA;
		++clearValueCount;
	}
	if ( doClearDepth == true ) {
		VkClearDepthStencilValue & clearDepthValue = clearValues[ clearValueCount ].depthStencil;
		clearDepthValue.depth = clearDepth;
		++clearValueCount;
	}
	beginInfo.clearValueCount = clearValueCount;
	beginInfo.pClearValues = clearValues;
	
	vkCmdBeginRenderPass( m_commandBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE );
}

void CommandContext::BindDescriptorSet( const DescriptorSet * descriptorSet ) {
	VkDescriptorSet set = descriptorSet->GetDescriptorSet();
	vkCmdBindDescriptorSets( m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderObjects.unifiedPipelineLayout, descriptorSet->GetScope(), 1, &set, 0, NULL );
}

void CommandContext::EndRenderPass() {
	if ( m_inRenderPass == true ) {
		vkCmdEndRenderPass( m_commandBuffer );
		m_inRenderPass = false;
		m_pipelineState.renderPassState = {};
	}
}

void CommandContext::Blit( const Image * src, const Image * dst ) {
	EndRenderPass();

	VkImageBlit blit = {};
	blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blit.srcSubresource.baseArrayLayer = 0;
	blit.srcSubresource.mipLevel = 0;
	blit.srcSubresource.layerCount = 1;
	blit.dstSubresource = blit.srcSubresource;
	blit.srcOffsets[ 1 ].x = src->GetWidth();
	blit.srcOffsets[ 1 ].y = src->GetHeight();
	blit.srcOffsets[ 1 ].z = 1;
	blit.dstOffsets[ 1 ].x = dst->GetWidth();
	blit.dstOffsets[ 1 ].y = dst->GetHeight();
	blit.dstOffsets[ 1 ].z = 1;

	vkCmdBlitImage( m_commandBuffer, src->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR );
}

static void TranslateImageLayout( imageLayout_t layout, VkImageLayout & vulkanLayout, VkAccessFlags & vulkanAccessFlags, VkPipelineStageFlags & pipelineStageFlags ) {
	switch ( layout ) {
		case IMAGE_LAYOUT_FRAGMENT_SHADER_READ: {
			vulkanLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			vulkanAccessFlags = VK_ACCESS_SHADER_READ_BIT;	
			pipelineStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}
		case IMAGE_LAYOUT_COLOR_ATTACHMENT: {
			vulkanLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			vulkanAccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			pipelineStageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			break;
		}
		case IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT: {
			vulkanLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			vulkanAccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			pipelineStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			break;
		}
		case IMAGE_LAYOUT_PRESENT: {
			vulkanLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			vulkanAccessFlags = VK_ACCESS_MEMORY_READ_BIT;
			pipelineStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			break;
		}
	}
}

void CommandContext::PipelineBarrier( Image * image, imageLayout_t newLayout, barrierFlags_t flags ) {
	if ( m_pipelineState.renderPassState.color == image || m_pipelineState.renderPassState.depth == image ) {
		EndRenderPass();
	}
	// We use layout tracking to make the interface easier.  It's generally more useful to know in what state you need
	// to be, than to have to keep track of in what state you already are.
	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	const bool discard = ( flags & BARRIER_DISCARD_AND_IGNORE_OLD_LAYOUT ) != 0;
	VkPipelineStageFlags srcPipelineStage;
	if ( discard == true ) {
		// Setting the old layout to undefined means it doesn't matter what the current layout ACTUALLY is.  It also means
		// that whatever is in the image at the time of the barrier is fair game for garbage.  This is a potential optimization
		// as well as a convenience.  Generally, attachments should be discarded the first time they're used in a frame.
		// In addition, render passes won't do any of this for us, so before setting a render target, it should be transitioned
		// to the proper attachment layout.
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.srcAccessMask = 0;
		srcPipelineStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	} else {
		if ( image->GetLayout() == newLayout ) {
			return;	// Layouts that don't discard nor transition between two different layouts become a NOP.
		}
		TranslateImageLayout( image->GetLayout(), barrier.oldLayout, barrier.srcAccessMask, srcPipelineStage );
	}
	VkPipelineStageFlags dstPipelineStage;
	TranslateImageLayout( newLayout, barrier.newLayout, barrier.dstAccessMask, dstPipelineStage );
	barrier.subresourceRange.aspectMask = TranslateFormatToAspect( image->GetFormat() );
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	barrier.image = image->GetImage();
	vkCmdPipelineBarrier( m_commandBuffer, srcPipelineStage, dstPipelineStage, 0, 0, NULL, 0, NULL, 1, &barrier );
	image->SetLayout( newLayout );
}