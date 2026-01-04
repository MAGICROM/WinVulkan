
struct MeshPushConstants {
	glm::vec4 data;
	glm::mat4 model;
};

struct UBOVS{
		glm::mat4 projection;
		glm::mat4 view;
		glm::vec4 lightPos = glm::vec4(0.0f, -5.0f, 0.0f, 1.0f);
		float locSpeed = 0.0f;
		float globSpeed = 0.0f;
static VkDescriptorSetLayoutBinding BindLayout(uint32_t binding)
	{
	VkDescriptorSetLayoutBinding Binding;
	Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	Binding.binding = binding;
	Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	Binding.descriptorCount = 1;
	Binding.pImmutableSamplers = nullptr;
	return Binding;
	}
};
//is a [] BY FRAMES
#define SN_ARRAY_BY_FRAME
struct snBinding
{
	VkDescriptorSetLayoutBinding	Binding;
	uint32_t 						count;
	snBuffer*	SN_ARRAY_BY_FRAME	buffers = nullptr; 				
};
struct snDescriptor
{
	std::vector<snBinding> 					bindings;
	
	VkDescriptorPool 						descriptorPool;
	VkDescriptorSet* SN_ARRAY_BY_FRAME 		descriptorsets = nullptr; 			
	VkDescriptorSetLayout 					descriptorSetLayout;

void DescriptorsCreate(){
	VkDescriptorPoolSize descriptorPoolSizes[] =
	{	{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 }
	};

	VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCI.maxSets = 10 * (sizeof(descriptorPoolSizes)/sizeof(*descriptorPoolSizes));
		descriptorPoolCI.poolSizeCount = (uint32_t)(sizeof(descriptorPoolSizes)/sizeof(*descriptorPoolSizes));
		descriptorPoolCI.pPoolSizes = descriptorPoolSizes;

	err = vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorPool);

	VkDescriptorSetLayoutBinding* setLayoutBindings = new VkDescriptorSetLayoutBinding[bindings.size()];
	for(volatile int i=0;i<bindings.size();i++)
	{
		setLayoutBindings[i] = bindings[i].Binding;
	}
	
	VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
	descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutCI.bindingCount = bindings.size(); //Le nombre d'entrées de la liste
	descriptorLayoutCI.pBindings = setLayoutBindings;

	err = vkCreateDescriptorSetLayout(device, &descriptorLayoutCI, nullptr, &descriptorSetLayout);
	if( err!= VK_SUCCESS){throw std::runtime_error("failed to create descriptor set layout!");}

	delete [] setLayoutBindings;

	descriptorsets = new VkDescriptorSet[swap_imageCount];
	//Les sets de ce modéle un par frame
	
	for (size_t i = 0; i < swap_imageCount; i++) 
	{	
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = descriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &descriptorSetLayout;
		
		err = vkAllocateDescriptorSets(device, &allocateInfo, &descriptorsets[i]);
		if( err!= VK_SUCCESS){throw std::runtime_error("failed to allocate descriptor sets!");}
		
		for (size_t j = 0; j < bindings.size(); j++) 
		{	
		VkWriteDescriptorSet writeDescriptorSets{};
		
		writeDescriptorSets.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets.dstSet = descriptorsets[i];
		writeDescriptorSets.dstBinding = bindings[j].Binding.binding;
		writeDescriptorSets.dstArrayElement = 0;
		writeDescriptorSets.descriptorCount = 1;
		
		int bi=i; //Buffer index
		if(bindings[j].count == 1)bi=0; //Uniform Buffer unique

		if(bindings[j].Binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
			VkDescriptorBufferInfo Buffer_info_UBO{};
			Buffer_info_UBO.buffer = bindings[j].buffers[bi].vkbuffer;
			Buffer_info_UBO.offset = 0;
			Buffer_info_UBO.range = bindings[j].buffers[bi].mem_size;
			writeDescriptorSets.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets.pBufferInfo = &Buffer_info_UBO;
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSets, 0, nullptr);
			}
		
		if(bindings[j].Binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = bindings[j].buffers[0].pTexture->imageview;
			imageInfo.sampler = bindings[j].buffers[0].pTexture->sampler;
			writeDescriptorSets.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSets.pImageInfo = &imageInfo;
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSets, 0, nullptr);
			}
		}
	}
} 		
void DescriptorsDestroy(){
		for (size_t j = 0; j < bindings.size(); j++) 
		{
			for (size_t i = 0; i < bindings[j].count; i++) 
			{	
				bindings[j].buffers[i].Release();
			}
			if(bindings[j].count > 1)
				delete [] bindings[j].buffers;
			else
				delete bindings[j].buffers;
		}	
		
		vkDeviceWaitIdle(device);
		vkFreeDescriptorSets(device,descriptorPool,swap_imageCount,descriptorsets);
		delete [] descriptorsets;
		
		vkDestroyDescriptorSetLayout(device,descriptorSetLayout,NULL);
		vkDestroyDescriptorPool(device,descriptorPool,NULL);
	}
};
struct snPipeline
{
	VkPipelineLayout 							PipeLayout;
	VkPipeline 									Pipe;
	snDescriptor								Descriptor;

	VkShaderModuleCreateInfo*					Shaders = nullptr;			//[]		
	VkPipelineShaderStageCreateInfo*            ShaderStageInfo= nullptr;	//[]		
	VkDescriptorSetLayoutBinding*   			bindings= nullptr;			//[]
	VkDynamicState*                 			dynamicStates= nullptr;		//[]
	int 										push_constants_count;
	VkPushConstantRange* 						push_constants= nullptr; 	//[]
	
	VkPipelineVertexInputStateCreateInfo        vertexInputInfo{};
	VkPipelineInputAssemblyStateCreateInfo      inputAssembly{};
	VkPipelineViewportStateCreateInfo           viewportState{};
	VkPipelineRasterizationStateCreateInfo      rasterizer{};
	VkPipelineMultisampleStateCreateInfo        multisampling{};
	VkPipelineColorBlendAttachmentState         colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo         colorBlending{};
	VkPipelineDynamicStateCreateInfo            dynamicState{};
	
	VkDescriptorSetLayoutCreateInfo             set_layout{};
	VkPipelineLayoutCreateInfo                  pipelineLayoutInfo{};
	VkGraphicsPipelineCreateInfo                GfxPipelineInfo{};
	
void Reset(){
	memset(this,0,sizeof(snPipeline));
}
void PushConstants(int x){
	push_constants_count = x;
	push_constants = new VkPushConstantRange[x];
}
void PushConstantsInfo(int x,uint32_t size,VkShaderStageFlags stage){
	push_constants[x].offset = 0;
	push_constants[x].size = size;
	push_constants[x].stageFlags = stage;
}
void LoadShader(int x,const std::string &filename,const char* fun,VkShaderStageFlagBits stage)
{
	auto ShaderCode = readFile(filename);

	char* ptr = new char[ShaderCode.size()];
	Shaders[x].pCode = (uint32_t*)ptr;
	//Cree une copie jusque a la creation du pipeline
	for(volatile int i=0;i<ShaderCode.size();i++)
	{
		ptr[i] = ShaderCode[i];
	}
	Shaders[x].codeSize = ShaderCode.size();
	ShaderStageInfo[x].stage = stage;
	//nom de la fonction dans le shader
	ShaderStageInfo[x].pName = fun;
}
	
void ReserveShaders(size_t numbers){
			GfxPipelineInfo.stageCount = numbers;
			ShaderStageInfo = new VkPipelineShaderStageCreateInfo[GfxPipelineInfo.stageCount];
			Shaders = new VkShaderModuleCreateInfo[GfxPipelineInfo.stageCount];
			GfxPipelineInfo.pStages = ShaderStageInfo;

			for(volatile int i = 0; i < GfxPipelineInfo.stageCount; i++)
			{
				//Segmentation fault prevention 1
				memset(&ShaderStageInfo[i],0,sizeof(VkPipelineShaderStageCreateInfo));
				//Segmentation fault prevention 2
				memset(&Shaders[i],0,sizeof(VkShaderModuleCreateInfo));

				Shaders[i].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				Shaders[i].pCode = nullptr;  
				Shaders[i].codeSize = 0;
				
				ShaderStageInfo[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
       			ShaderStageInfo[i].stage = VK_SHADER_STAGE_VERTEX_BIT;
        		ShaderStageInfo[i].module = nullptr;
			}
}
void Prepare(){
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		set_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		GfxPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkVertexInputBindingDescription VertexBinds;
		VkVertexInputAttributeDescription VertexAttribs;

		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;

		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        dynamicStates = new VkDynamicState[2];
		dynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
       	dynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

        dynamicState.dynamicStateCount = 2;
        dynamicState.pDynamicStates = dynamicStates;

    	set_layout.bindingCount = 0;
        set_layout.pBindings = nullptr;

		pipelineLayoutInfo ={
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 0,
			.pSetLayouts = 0,
			.pushConstantRangeCount = 0,
			.pPushConstantRanges = 0
		};

        GfxPipelineInfo ={
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = 0,
			.pStages = 0,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = 0,
			.renderPass = 0,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE
		};
	}
void Create(VkRenderPass rp){
		
		pipelineLayoutInfo.setLayoutCount = 1; //un par frame
		pipelineLayoutInfo.pSetLayouts = &Descriptor.descriptorSetLayout; //<---- Ils servent ici un
		if(push_constants)
		{
			pipelineLayoutInfo.pushConstantRangeCount = push_constants_count;
			pipelineLayoutInfo.pPushConstantRanges = push_constants;
		}
		
		for(volatile int i = 0; i < GfxPipelineInfo.stageCount; i++)
		{
			VkShaderModuleCreateInfo createInfoSHD{};
			createInfoSHD.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfoSHD.codeSize = Shaders[i].codeSize;
			createInfoSHD.pCode = Shaders[i].pCode;
			createInfoSHD.flags = Shaders[i].flags;
			
			if (vkCreateShaderModule(device, &createInfoSHD, nullptr, &ShaderStageInfo[i].module) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader!");
			}
		}
		
		if(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &PipeLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
		}

		GfxPipelineInfo.layout = PipeLayout;
		GfxPipelineInfo.renderPass = rp;
		
		if(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &GfxPipelineInfo, nullptr, &Pipe) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
		}


		for(volatile int i = 0; i < GfxPipelineInfo.stageCount; i++)
		{
		vkDestroyShaderModule(device, ShaderStageInfo[i].module, nullptr);
		}
		
		if(ShaderStageInfo)delete [] ShaderStageInfo;
		if(Shaders)
		{
			for(volatile int i=0;i<GfxPipelineInfo.stageCount;i++)
			{
				char* ptr = (char*)Shaders[i].pCode;
				delete [] ptr;
			}

			
			delete [] Shaders;
		}
		if(dynamicStates)delete [] dynamicStates;
		if(bindings)delete [] bindings; 
		if(push_constants)delete [] push_constants;
	};
void Release(){
		Descriptor.DescriptorsDestroy();
		vkDestroyPipeline(device, Pipe, nullptr);
        vkDestroyPipelineLayout(device, PipeLayout, nullptr);
}
};