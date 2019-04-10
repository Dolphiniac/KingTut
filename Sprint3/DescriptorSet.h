#pragma once

#include "Renderer.h"

// The scope determines for how long the descriptor set is to be bound.  It plays directly into which slots are valid for a set.
enum descriptorScope_t {
	DESCRIPTOR_SCOPE_FRAME,
	DESCRIPTOR_SCOPE_VIEW,
	DESCRIPTOR_SCOPE_MESH,
	DESCRIPTOR_SCOPE_COUNT
};

// Slots defined as named indices per scope type so it's relatively clear where a resource goes.
// This is a fairly clear way to describe where descriptors go without automation and metadata of some sort.
enum frameDescriptorUniformBufferSlot_t {
	FRAME_DESCRIPTOR_UNIFORM_BUFFER_SLOT_0,
	FRAME_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND
};

enum viewDescriptorUniformBufferSlot_t {
	VIEW_DESCRIPTOR_UNIFORM_BUFFER_SLOT_0,
	VIEW_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND
};

enum meshDescriptorUniformBufferSlot_t {
	MESH_DESCRIPTOR_UNIFORM_BUFFER_SLOT_0,
	MESH_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND
};

enum meshDescriptorSamplerSlot_t {
	MESH_DESCRIPTOR_SAMPLER_SLOT_0 = MESH_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND,
	MESH_DESCRIPTOR_SAMPLER_SLOT_BOUND
};

typedef uint32_t descriptorSlot_t;

class Buffer;
class Image;

class DescriptorSet {
public:
	static DescriptorSet * Allocate( descriptorScope_t scope );
	void SetUniformBuffer( descriptorSlot_t slot, const Buffer * buffer );
	void SetImageSampler( descriptorSlot_t slot, samplerType_t samplerType, const Image * image );
	VkDescriptorSet GetDescriptorSet() const { return m_descriptorSet; }
	descriptorScope_t GetScope() const { return m_scope; }

private:
	VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
	descriptorScope_t m_scope = DESCRIPTOR_SCOPE_COUNT;

private:
	DescriptorSet() = default;
};