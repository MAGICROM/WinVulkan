struct Vertex
{
	glm::vec3 position;
	glm::vec3 normale;
	glm::vec2 uv;

	static VkVertexInputBindingDescription getBinding() {
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttribs() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, position);
	
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, normale);
	
	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, uv);

    return attributeDescriptions;
	}
};

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
extern unsigned char lunarg_ppm[];
extern unsigned int lunarg_ppm_len;
bool loadTexturelunarG(const char *filename, uint8_t *rgba_data, uint32_t *width, uint32_t *height) {
    (void)filename;
    char *cPtr;
    cPtr = (char *)lunarg_ppm;
    if ((unsigned char *)cPtr >= (lunarg_ppm + lunarg_ppm_len) || strncmp(cPtr, "P6\n", 3)) {
        return false;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    sscanf(cPtr, "%u %u", width, height);
    if (rgba_data == NULL) {
        return true;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    if ((unsigned char *)cPtr >= (lunarg_ppm + lunarg_ppm_len) || strncmp(cPtr, "255\n", 4)) {
        return false;
    }
    while (strncmp(cPtr++, "\n", 1))
        ;
    for (int y = 0; y < *height; y++) {
        uint8_t *rowPtr = rgba_data;
        for (int x = 0; x < *width; x++) {
            memcpy(rowPtr, cPtr, 3);
            rowPtr[3] = 255; /* Alpha of 1 */
            rowPtr += 4;
            cPtr += 3;
        }
        rgba_data += *width * 4;
    }
    return true;
}
#include <fstream>
static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
        }
uint32_t Read_Obj(void** ppVertex,void** ppIndex)
{
	std::vector<char> object = readFile("suzan.obj");
	if(!object.size())return 0; //PAS DE fiCHIER
	std::vector<char> line;
	bool newline = true;
	
	std::vector<float> uvs;
	std::vector<float> vertex;
	std::vector<float> normales;
	std::vector<int> indexthisline;
	std::vector<int> index;
	std::vector<std::vector<char>> faces;
	
	int uvs_vec_size = 0;
	int vertex_vec_size = 0;
	int normales_vec_size = 0;
	int faces_vec_size = 0;
	int vec_size = 0;
	
	//Total de chaque
	int uvs_size = 0;
	int vertex_size = 0;
	int normale_size = 0;
	int face_size = 0;

	int j = 0; //index dans la ligne
	std::vector<std::vector<char>> tokens;
	std::vector<char> token;
	for(volatile int i = 0;i < object.size();i++)//Jusqu'a la fin du fichier
	{
		if(object[i] != '\n'){
		line.push_back(object[i]); //Pousse chaque caracteres de la ligne
		//char* pligne = reinterpret_cast<char*>(line.data());
		
		if(line[j] != ' ')
			{
				token.push_back(line[j]);//Stocke chaque token
				j++;
				continue;
			}
		tokens.push_back(token);//Dans les tokens de la ligne
		j++;
		if(object[i] != '\n'){token.clear();
							  continue;}//Encore un token
		
		}
		//Fin de ligne
		tokens.push_back(token);
		token.clear();
    	int store = -1;
		vec_size = 0;
		//procede tous les tokens de cette ligne
		for(int k=0;k<tokens.size();k++)//Procede sur tous les tokens
		{
			if(k==0)//C'est le premier token c'est quoi?
			{
				store = -1;
				if(tokens[k].size() == 1)//Token de 1 lettre
				{
					if(tokens[0][0]=='v'){store = 0;vertex_size++;} //vertex
					if(tokens[0][0]=='f'){store = 3;} //face
				}
				if(tokens[k].size() == 2)//Token de 2 lettres
				{
					if(tokens[0][0]=='v' && tokens[0][1]=='n'){store = 1;normale_size++;} //normales
					if(tokens[0][0]=='v' && tokens[0][1]=='t'){store = 2;uvs_size++;} //uvs
				}
			}
			else //Pousse les suivants dans leur stockage respectif
			{
			vec_size++;
			if(store == 0){vertex.push_back(std::atof(reinterpret_cast<char*>(tokens[k].data())));
				if(vec_size > vertex_vec_size)vertex_vec_size=vec_size;
			}
			if(store == 1){normales.push_back(std::atof(reinterpret_cast<char*>(tokens[k].data())));
				if(vec_size > normales_vec_size)normales_vec_size=vec_size;
			}
			if(store == 2){uvs.push_back(std::atof(reinterpret_cast<char*>(tokens[k].data())));
				if(vec_size > uvs_vec_size)uvs_vec_size=vec_size;
			}
			if(store == 3){
				faces.push_back(tokens[k]);
				if(vec_size > faces_vec_size)faces_vec_size=vec_size;
			}
			}
		}
		if(store == 3)//Si c'est les indices de faces faut décoder
		{
			if(faces.size() == 4){face_size+=2;}// 2 triangles
			if(faces.size() == 3){face_size++;}// 1 triangles
			std::vector<std::vector<char>> indices;
			std::vector<int> indices_int;
			for(int l=0;l<faces.size();l++)
			{
				std::vector<char> n;
				n.clear();
				for(int m=0;m<faces[l].size();m++)
				{
					char v=faces[l][m];
					if(v == '/')
						{
							n.push_back('\n');
							indices.push_back(n);
							n.clear();
							continue;
						}
					n.push_back(v);
				}
				n.push_back('\n');
				indices.push_back(n);
				for(int m=0;m<indices.size();m++)
				{
					indices_int.push_back(std::atoi(reinterpret_cast<char*>(indices[m].data())));
				}
				for(int m=0;m<indices_int.size();m++)
				{
					indexthisline.push_back(indices_int[m]);
				}
				indices_int.clear();
				indices.clear();
			}
			if(faces.size() == 4)
			{
				if(indexthisline.size() == 12)
				{
					index.push_back(indexthisline[0]);
					index.push_back(indexthisline[1]);
					index.push_back(indexthisline[2]);
					index.push_back(indexthisline[3]);
					index.push_back(indexthisline[4]);
					index.push_back(indexthisline[5]);
					index.push_back(indexthisline[6]);
					index.push_back(indexthisline[7]);
					index.push_back(indexthisline[8]);
					
					index.push_back(indexthisline[0]);
					index.push_back(indexthisline[1]);
					index.push_back(indexthisline[2]);
					index.push_back(indexthisline[6]);
					index.push_back(indexthisline[7]);
					index.push_back(indexthisline[8]);
					index.push_back(indexthisline[9]);
					index.push_back(indexthisline[10]);
					index.push_back(indexthisline[11]);
				}
			}
			if(faces.size() == 3)
			{
				if(indexthisline.size() == 9)
				{
					for(int m=0;m<9;m++)
					{
						index.push_back(indexthisline[m]);
					}
				}		
			}
			indexthisline.clear();
			faces.clear();
		}
		line.clear();
		tokens.clear();
		newline = true;
		j = 0;
		
	}
	if(vertex_size == vertex.size()/vertex_vec_size) //3
		if(uvs_size == uvs.size()/uvs_vec_size) //2
			if(normale_size == normales.size()/normales_vec_size)//3
			{
				Vertex* pVertex = new Vertex[face_size*3];
				if(index.size()==face_size*3*3)
				{
					if(ppVertex)(*ppVertex) = pVertex;
					for(volatile int i=0;i<face_size*3;i++)
					{
						
						int iv = index[i*3]-1;
						int it = index[i*3+1]-1;
						int in = index[i*3+2]-1;

						if(iv*3+2 > vertex.size())
							{std::cerr << "TOO BIG INDEX FOR VERTEX" << std::endl;continue;}
						if(in*3+2 > normales.size())
							{std::cerr << "TOO BIG INDEX FOR NORMALE :" << std::endl;continue;}
						if(it*2+1 > uvs.size())
							{std::cerr << "TOO BIG INDEX FOR UVs" << std::endl;continue;}
						
						
						pVertex[i].position.x = vertex[iv*3]*.1f;
						pVertex[i].position.y = vertex[iv*3+1]*.1f;
						pVertex[i].position.z = vertex[iv*3+2]*.1f;

						pVertex[i].normale.x = normales[in*3];
						pVertex[i].normale.y = normales[in*3+1];
						pVertex[i].normale.z = normales[in*3+2];

						pVertex[i].uv.x = uvs[it*2];
						pVertex[i].uv.y = uvs[it*2+1];
					}
				
				}
				vertex.clear();
				uvs.clear();
				normales.clear();
				index.clear();
				
				return face_size*3;//Ok les chiffres correspondent
			}
				
	vertex.clear();
	uvs.clear();
	normales.clear();
	index.clear();
	newline = true; //Erreur les chiffres correspondent pas
	//Tout a été lu
	return 0;
}
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

	VkShaderModuleCreateInfo*					Shaders;		//[]		
	VkPipelineShaderStageCreateInfo*            ShaderStageInfo;//[]		
	VkDescriptorSetLayoutBinding*   			bindings;		//[]
	VkDynamicState*                 			dynamicStates;	//[]
	
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
	memset(this,0,sizeof(snPipeline));
}
void PipeModel_CreateShaders(size_t numbers){
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
void PipeModel_Prepare(){
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
void PipeModel_Create(VkRenderPass rp){
		
		pipelineLayoutInfo.setLayoutCount = 1; //un par frame
		pipelineLayoutInfo.pSetLayouts = &Descriptor.descriptorSetLayout; //<---- Ils servent ici un
		
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
void PipeModel_Release(){
		Descriptor.DescriptorsDestroy();
		vkDestroyPipeline(device, Pipe, nullptr);
        vkDestroyPipelineLayout(device, PipeLayout, nullptr);
}
};