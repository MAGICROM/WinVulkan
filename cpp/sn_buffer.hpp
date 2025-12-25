VkCommandPool transfercommandPool{VK_NULL_HANDLE};

struct snCommand
{
static void TransfertCommandPool_Create()
{	
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsFamily;

			if (vkCreateCommandPool(device, &poolInfo, nullptr, &transfercommandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create transfert command pool!");
			}
}
static void TransfertCommandPool_Destroy()
{	
	vkDestroyCommandPool(device,transfercommandPool,NULL);
}
	VkCommandBuffer m_cmd;
	VkFence fence;
	void Begin()
	{
		VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = transfercommandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;
				
		vkAllocateCommandBuffers(device, &allocInfo, &m_cmd);

		VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
		vkCreateFence(device, &fenceInfo, nullptr, &fence);
	}
	void End()
	{
		vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
		vkDestroyFence(device, fence, nullptr);
		vkFreeCommandBuffers(device, transfercommandPool, 1, &m_cmd);
	}
	operator VkCommandBuffer*(){return &m_cmd;};
	operator VkCommandBuffer&(){return m_cmd;};
	operator VkFence&(){return fence;};
};
struct snTexture
{
	VkImage image;
	VkImageView imageview;
	VkSampler sampler;
};
struct snBuffer
{ 

	alignas(16)
//If is set the buffer is a texture
	snTexture* 		pTexture = nullptr;
//Pointer set on a shared memory address
	void* 			mem_ptr = nullptr;
	uint32_t 		mem_size = 0l;
//Handles on memory
    VkBuffer 		vkbuffer	{VK_NULL_HANDLE};
    VkDeviceMemory 	vkdevicemem	{VK_NULL_HANDLE};
	
	VkMappedMemoryRange* range{nullptr};
	
//Cree un buffer sur la carte graphique uniquement
void CreateBuffer(VkBufferUsageFlagBits usage,uint32_t size){
	VkBufferCreateInfo ubo{};
            ubo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            ubo.size = size;
            ubo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            ubo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(device,&ubo,nullptr,&vkbuffer);
	
	VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device,vkbuffer,&mem_reqs);
    VkMemoryAllocateInfo mem_info{};

	mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.memoryTypeIndex = findMemoryType(mem_reqs.memoryTypeBits, 
                				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    mem_info.allocationSize = mem_reqs.size;
	err = vkAllocateMemory(device,&mem_info,nullptr,&vkdevicemem);
	if(err==VK_SUCCESS)mem_size = mem_reqs.size;
}
//Cree un buffer sur la carte graphique et visible par le cpu
void CreateAndCopyBuffer(VkBufferUsageFlagBits usage,void *Src,uint32_t size,bool unmap){
    
   //Buffer creation
	
	VkBufferCreateInfo ubo{};
            ubo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            ubo.size = size;
            ubo.usage = usage;
            ubo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    err = vkCreateBuffer(device,&ubo,nullptr,&vkbuffer);
    
	//Check for memory requirements
    
	VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(device,vkbuffer,&mem_reqs);
    VkMemoryAllocateInfo mem_info{};

	mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_info.memoryTypeIndex = findMemoryType(mem_reqs.memoryTypeBits, 
                				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    mem_info.allocationSize = mem_reqs.size;
    err = vkAllocateMemory(device,&mem_info,nullptr,&vkdevicemem);
	err = vkBindBufferMemory(device, vkbuffer, vkdevicemem, 0);
    
	//Buffer mapping and copy
    
	err = vkMapMemory(device,vkdevicemem,0,VK_WHOLE_SIZE,0,&mem_ptr);
    if(Src)memcpy(mem_ptr, Src, size);
    if(unmap)
        {
        VkMappedMemoryRange range = {
        .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
        .memory = vkdevicemem,
        .size = size
		};
        err = vkFlushMappedMemoryRanges(device, 1, &range);
        if(Src)vkUnmapMemory(device,vkdevicemem);
        }

    mem_size = size;
	}
//Cree un uniform buffer sur la carte graphique et visible par le cpu
void CreateUniformBuffer(void *Src,uint32_t size){
	//VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	CreateAndCopyBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,Src,size,false); //Reste Mappé
	}
//Cree un buffer sur la carte graphique et visible par le cpu pour transfert
void CreateTransferBuffer(void *Src,uint32_t size){
	CreateAndCopyBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,Src,size,true); //Démappe à a fin
	}
//Cree un buffer sur la carte graphique pour les points
void CreateVertexBuffer(uint32_t size){
	CreateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,size);
	}
//Cree un buffer sur la carte graphique pour les indices
void CreateIndexBuffer(uint32_t size){
	CreateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,size);
	}
//Copy sur le gpu des données depuis le cpu
void CopyFrom(snBuffer& buffer){
	err = vkBindBufferMemory(device, vkbuffer, vkdevicemem, 0);
	err = vkBindBufferMemory(device, buffer.vkbuffer, buffer.vkdevicemem, 0);
	//CREATE COMMAND 
	VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = transfercommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
			
	VkCommandBuffer copyCmd;
	vkAllocateCommandBuffers(device, &allocInfo, &copyCmd);
	
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = mem_size;
	
	//RECORD COMMAND 
	VkCommandBufferBeginInfo BeginInfo{};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	
	vkBeginCommandBuffer(copyCmd,&BeginInfo);
	vkCmdCopyBuffer(copyCmd, buffer.vkbuffer, vkbuffer, 1, &copyRegion);
	vkEndCommandBuffer(copyCmd);
	
	//SUBMIT AND FREE COMMAND 
	VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = 0;
	VkFence fence;
	vkCreateFence(device, &fenceInfo, nullptr, &fence);
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;
	err = vkQueueSubmit(transferQueue, 1, &submitInfo, fence);
	vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000);
	vkDestroyFence(device, fence, nullptr);
	vkFreeCommandBuffers(device, transfercommandPool, 1, &copyCmd);
	

	}
//Cree une texture 
void CreateTextureFrom(	uint32_t tex_width,uint32_t tex_height,void* data,uint32_t size)
{
	
	if(pTexture)return;
	pTexture = new snTexture;
	
	snBuffer stage;
	stage.CreateTransferBuffer(data,size);

	mem_size = size;
	
	VkImageCreateInfo text_img{};
    text_img.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    text_img.imageType = VK_IMAGE_TYPE_2D;
    text_img.format = VK_FORMAT_R8G8B8A8_UNORM;
    text_img.extent = {tex_width, tex_height, 1};
    text_img.mipLevels = 1;
    text_img.arrayLayers = 1;
    text_img.samples = VK_SAMPLE_COUNT_1_BIT;
    text_img.tiling = VK_IMAGE_TILING_LINEAR;
    text_img.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    text_img.flags = 0;
    text_img.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;   

	if (vkCreateImage(device, &text_img, nullptr, &pTexture->image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image!");
        }
	
	VkMemoryRequirements req;
    vkGetImageMemoryRequirements(device, pTexture->image, &req);
    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = req.size;
    alloc_info.memoryTypeIndex = findMemoryType(req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	err = vkAllocateMemory(device, &alloc_info, nullptr, &vkdevicemem);
    err = vkBindImageMemory(device, pTexture->image, vkdevicemem, 0);

	//SERT PAS POUR L'INSTANT
	VkImageSubresource subres{};
		subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subres.mipLevel = 0;
		subres.arrayLayer = 0;
		
	VkSubresourceLayout layout_data;
	vkGetImageSubresourceLayout(device, pTexture->image, &subres, &layout_data);
	//CREATE COMMAND 
	snCommand copyCmd;
	copyCmd.Begin();
	
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = mem_size;
	
	//RECORD COMMAND 
	VkCommandBufferBeginInfo BeginInfo{};
	BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
 
	vkBeginCommandBuffer(copyCmd,&BeginInfo);
	CommandImgMemBarrier(copyCmd,pTexture->image,VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	
	VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = tex_width;
        region.imageExtent.height = tex_height;
        region.imageExtent.depth = 1;
		
	vkCmdCopyBufferToImage(copyCmd,stage.vkbuffer,pTexture->image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	CommandImgMemBarrier(copyCmd,pTexture->image,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	err = vkEndCommandBuffer(copyCmd);
	
	//SUBMIT AND FREE COMMAND 

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd.m_cmd;
	err = vkQueueSubmit(graphicsQueue, 1, &submitInfo, copyCmd.fence);
	vkQueueWaitIdle(graphicsQueue);
	copyCmd.End();
	//err = vkDeviceWaitIdle(device);

	VkImageViewCreateInfo img_view{};
    img_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    img_view.image = pTexture->image;
    img_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    img_view.format = VK_FORMAT_R8G8B8A8_UNORM;
    img_view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};
    img_view.components = {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                };

    err = vkCreateImageView(device,&img_view,nullptr,&pTexture->imageview);   
    
    VkSamplerCreateInfo smp_info{};
    smp_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    smp_info.magFilter = VK_FILTER_NEAREST;
    smp_info.minFilter = VK_FILTER_NEAREST;
    smp_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    smp_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    smp_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    smp_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    smp_info.mipLodBias = 0.f;
    smp_info.anisotropyEnable = VK_FALSE;
    smp_info.maxAnisotropy = 1;
    smp_info.compareOp = VK_COMPARE_OP_NEVER;
    smp_info.minLod = 0.f;
    smp_info.maxLod = 0.f;
    smp_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    smp_info.unnormalizedCoordinates = VK_FALSE;
 
    err = vkCreateSampler(device,&smp_info,nullptr,&pTexture->sampler);
	stage.Release();
}
void Lock( uint32_t where, uint32_t size, void* *lptr, uint32_t flags = 0)
{
	range = new VkMappedMemoryRange;//VK_WHOLE_SIZE
	range->sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range->size = size;
	range->offset = where;
	range->memory = vkdevicemem;
	err = vkMapMemory(device,vkdevicemem,where,size,flags,lptr);
}
void UnLock()
{
    err = vkFlushMappedMemoryRanges(device, 1, range);
    vkUnmapMemory(device,vkdevicemem);
	delete range;
}
//Detruit le buffer
void Release()
	{
		if(pTexture)
		{
			vkDestroySampler(device,pTexture->sampler, nullptr);
			vkDestroyImageView(device, pTexture->imageview, nullptr);
            vkDestroyImage(device, pTexture->image, nullptr);
			delete pTexture;
			pTexture = nullptr;
		}			
		if(vkdevicemem)vkFreeMemory(device,vkdevicemem,nullptr);
		if(vkbuffer)vkDestroyBuffer(device,vkbuffer,nullptr);
	}
};