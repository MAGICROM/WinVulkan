uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(carte_graphique, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

	throw std::runtime_error("failed to find suitable memory type!");
	}
void CommandImgMemBarrier(VkCommandBuffer& buf,VkImage img,VkImageLayout de,VkImageLayout a)
	{
		VkImageMemoryBarrier memory_barrier = {};
		memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memory_barrier.srcAccessMask = 0;//VK_ACCESS_TRANSFER_WRITE_BIT;
		memory_barrier.dstAccessMask = 0;//VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		memory_barrier.oldLayout = de;//>>>>>>>>>>
		memory_barrier.newLayout = a;//>>>>>>>>>>
		memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		memory_barrier.image = img;//>>>>>>>>>>
		memory_barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

		VkPipelineStageFlagBits Source{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
		VkPipelineStageFlagBits Destination{VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
		

		if(de == VK_IMAGE_LAYOUT_UNDEFINED && a == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			memory_barrier.srcAccessMask = 0;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		//Pour les dynamics rendering
		if(de == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && a == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = 0;
			Source = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			Destination = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		//Pour les shaders
		if(de == VK_IMAGE_LAYOUT_UNDEFINED && a == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			memory_barrier.srcAccessMask = 0;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			Source = VK_PIPELINE_STAGE_HOST_BIT;
			Destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		if(de == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && a == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			Source = VK_PIPELINE_STAGE_TRANSFER_BIT;
			Destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		if(de == VK_IMAGE_LAYOUT_PREINITIALIZED && a == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			memory_barrier.srcAccessMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;//VK_ACCESS_TRANSFER_WRITE_BIT;
			memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			Source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			Destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		vkCmdPipelineBarrier(buf,
			Source,
			Destination,
			0,
			0,NULL,
			0,NULL,
			1,&memory_barrier);
		
	}
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features,VkPhysicalDevice physicalDevice) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }
VkFormat findDepthFormat(VkPhysicalDevice physicalDevice) {
        return findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
            physicalDevice
        );
    }