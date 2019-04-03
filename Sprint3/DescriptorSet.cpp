#include "DescriptorSet.h"
#include "Buffer.h"
#include "Image.h"

DescriptorSet * DescriptorSet::Allocate( descriptorScope_t scope ) {
	DescriptorSet * result = new DescriptorSet;
	result->m_scope = scope;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = renderObjects.descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	switch ( scope ) {
		case DESCRIPTOR_SCOPE_FRAME: {
			descriptorSetAllocateInfo.pSetLayouts = &renderObjects.frameDescriptorSetLayout;
			break;
		}
		case DESCRIPTOR_SCOPE_VIEW: {
			descriptorSetAllocateInfo.pSetLayouts = &renderObjects.viewDescriptorSetLayout;
			break;
		}
		case DESCRIPTOR_SCOPE_MESH: {
			descriptorSetAllocateInfo.pSetLayouts = &renderObjects.meshDescriptorSetLayout;
			break;
		}
	}
	VK_CHECK( vkAllocateDescriptorSets( renderObjects.device, &descriptorSetAllocateInfo, &result->m_descriptorSet ) );

	return result;
}

void DescriptorSet::SetUniformBuffer( descriptorSlot_t slot, const Buffer * buffer ) {
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = buffer->GetBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstBinding = slot;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets( renderObjects.device, 1, &writeDescriptorSet, 0, NULL );
}

void DescriptorSet::SetImageSampler( descriptorSlot_t slot, samplerType_t samplerType, const Image * image ) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = image->GetView();
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = renderObjects.samplers[ samplerType ];

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstBinding = slot;
	writeDescriptorSet.dstSet = m_descriptorSet;
	writeDescriptorSet.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets( renderObjects.device, 1, &writeDescriptorSet, 0, NULL );
}