struct sn_pipeline{

	VkPipelineLayout 							PipeLayout;
	VkPipeline 									Pipe;

	VkShaderModuleCreateInfo					*Shaders;		  //[]		
	VkPipelineShaderStageCreateInfo             *ShaderStageInfo; //[]		
	VkDescriptorSetLayoutBinding   				*bindings;		  //[]
	VkDynamicState                 				*dynamicStates;   //[]
	
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
	
	void PipeModel_Clear(){
			memset(this,0,sizeof(sn_pipeline));
		}

	void PipeModel_CreateBindings(size_t numbers)
		{
        	set_layout.bindingCount = numbers;
			pipelineLayoutInfo.setLayoutCount = 1;
			bindings = new VkDescriptorSetLayoutBinding[set_layout.bindingCount];
			for(volatile int i = 0; i < set_layout.bindingCount; i++)
				{
				//Segmentation fault prevention 1
				memset(&bindings[i],0,sizeof(VkDescriptorSetLayoutBinding));
				bindings[i].binding = 0;
				bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				bindings[i].descriptorCount = 1;
				bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
				bindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
				bindings[i].pImmutableSamplers = NULL;
			}
			set_layout.pBindings = bindings;
		}

	void PipeModel_CreateShaders(size_t numbers)
		{
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
	void PipeModel_PreFill()
	{
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
		vertexInputInfo.vertexAttributeDescriptionCount = 0;

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

        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

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
	void PipeModel_Proceed(VkRenderPass rp)
	{
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
		if(dynamicStates)delete [] dynamicStates;
		if(bindings)delete [] bindings; 
	};
	void PipeModel_Release()
	{
		vkDestroyPipeline(device, Pipe, nullptr);
        vkDestroyPipelineLayout(device, PipeLayout, nullptr);
	}
};