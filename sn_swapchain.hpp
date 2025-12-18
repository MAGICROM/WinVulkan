struct Vertex
{
	glm::vec3 position;
	glm::vec3 normale;
	glm::vec2 uv;
};

struct Camera
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 up = glm::vec3(0.0f,0.1f,0.0f);
	glm::vec3 forward;
	glm::vec3 right;
	
	glm::mat4 projection = glm::mat4(0.0);
	glm::quat orientation = glm::quat(glm::vec3(0.0f));
	
	float focale = 50.f; //mm
	float fov = 2.f * glm::atan(50.f/2*focale);
	
	float z_near = 0.0f;
	float z_far = 10.0f;
	
	glm::mat4 LookAtLH(const glm::vec3& to)
	{
		orientation = glm::lookAtLH(position,to,up);
		projection = glm::perspectiveLH(fov,
										sn_interface.aspectratio,
										z_near,
										z_far);

		glm::mat4 Translation = glm::translate(glm::mat4(1.0), -position);
		
		glm::mat4 Rotation = glm::mat4_cast(orientation);
		right = glm::vec3(Rotation[0][0],Rotation[1][0],Rotation[2][0]);
		forward = glm::vec3(Rotation[0][2],Rotation[1][2],Rotation[2][2]);
		
		glm::mat4 World = Rotation * Translation;
		
		return World;
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
bool loadTexture(const char *filename, uint8_t *rgba_data, uint32_t *width, uint32_t *height) {
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

void Read_Obj()
{
	std::vector<char> object = readFile("suzan.obj");
	if(!object.size())return; //PAS DE fiCHIER
	std::vector<char> line;
	bool newline = true;
	
	std::vector<float> uvs;
	std::vector<float> vertex;
	std::vector<float> normales;
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
			if(faces.size() == 4){std::cout << "QUAD";face_size+=2;}// 2 triangles
			if(faces.size() == 3){std::cout << "TRI" << std::endl;face_size++;}// 1 triangles
			std::vector<std::vector<char>> indices;
			std::vector<int> indices_int;
			for(int l=0;l<faces.size();l++)
			{
				std::vector<char> n;
				n.clear();
				for(int m=0;m<faces[l].size();m++)
				{
					char v=faces[l][m];
					if(v == '/' || v == ' ')
						{
							n.push_back(0);
							indices.push_back(n);
							n.clear();
							continue;
						}
					n.push_back(v);
				}
				
				indices.push_back(n);
				for(int m=0;m<indices.size();m++)
				{
					indices_int.push_back(std::atoi(reinterpret_cast<char*>(indices[m].data())));
					
				}
				for(int m=0;m<indices_int.size();m++)
				{
					std::cout << "/" << indices_int[m];
					index.push_back(indices_int[m]);
				}
				std::cout << ",";
				indices_int.clear();
				indices.clear();
			}
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
				return;//Ok les chiffres correspondent
			}
				
	
	newline = true; //Erreur les chiffres correspondent pas
	//Tout a été lu

}
//is a [] BY FRAMES
#define SN_ARRAY_BY_FRAME
struct snBinding
{
	VkDescriptorSetLayoutBinding	Binding;
	uint32_t 						count;
	snBuffer*	SN_ARRAY_BY_FRAME	buffers = nullptr; 				
};
struct snSwapChain
{
	VkExtent2D window_size;
	
	VkSwapchainKHR swapChain{NULL};     
    
	std::vector<VkImage>		swapChainImages;
    std::vector<VkImageView> 	swapChainImageViews;
    std::vector<VkFramebuffer> 	swapChainFramebuffers;

	std::vector<snBinding> bindings;
	
	VkCommandPool commandPool;
	VkCommandBuffer SN_ARRAY_BY_FRAME 		*commandBuffers = nullptr; 		
	VkCommandBuffer SN_ARRAY_BY_FRAME 		*imgui_commandBuffers = nullptr; 				
	
	VkDescriptorSetLayout 					descriptorSetLayout; 
	VkDescriptorSet SN_ARRAY_BY_FRAME 		*descriptorsets = nullptr; 			
	
	//snBuffer SN_ARRAY_BY_FRAME 				*uniforms = nullptr; 							

	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	//SYNCHRO
	
	std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

	//Memory
	
	bool pipe = true;
	void CreatePipeline()
	{
		PipeModel pm;
		
		pm.PipeModel_Clear();
		pm.PipeModel_PreFill();
		
		pm.PipeModel_CreateShaders(2);

		auto vertShaderCode = readFile("shaders/vert.spv");
		pm.Shaders[0].pCode = reinterpret_cast<const uint32_t*>(vertShaderCode.data());
		pm.Shaders[0].codeSize = vertShaderCode.size();
		pm.ShaderStageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		pm.ShaderStageInfo[0].pName = "main";
		
		auto fragShaderCode = readFile("shaders/frag.spv");
		pm.Shaders[1].pCode = reinterpret_cast<const uint32_t*>(fragShaderCode.data());
		pm.Shaders[1].codeSize = fragShaderCode.size();
		pm.ShaderStageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		pm.ShaderStageInfo[1].pName = "main";

		pm.pipelineLayoutInfo.setLayoutCount = 1; //un par frame
		pm.pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout; //<---- Ils servent ici un
		
		pm.PipeModel_Proceed(renderPass);

		pipelineLayout = pm.PipeLayout;
		graphicsPipeline = pm.Pipe;
	}
	void DestroyPipeline()
    {
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    }

	void DescriptorsCreate2()
    {
		snBinding bind{};
		bind.Binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bind.Binding.binding = 0;
		bind.Binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bind.Binding.descriptorCount = 1;
		bind.Binding.pImmutableSamplers = nullptr;
		bind.count = swap_imageCount;
		bind.buffers = new snBuffer[bind.count];

		for (size_t i = 0; i < swap_imageCount; i++) 
			{
				glm::mat4 matrice = glm::mat4(1.f);
				bind.buffers[i].CreateUniformBuffer(glm::value_ptr(matrice),sizeof(glm::mat4));
			}	

		bindings.push_back(bind);

		bind.Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		bind.Binding.binding = 1;
		bind.Binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		bind.Binding.descriptorCount = 1;
		bind.Binding.pImmutableSamplers = nullptr;
		bind.count = 1;
		bind.buffers = new snBuffer;

		uint32_t tex_width;
		uint32_t tex_height;
		loadTexture("pipo", NULL, &tex_width, &tex_height);
		uint8_t* data = new uint8_t[tex_width*tex_height*4];
		loadTexture("pipo", data, &tex_width, &tex_height);
		
		bind.buffers[0].CreateTextureFrom(
			tex_width,
			tex_height,
			data,
			tex_width*tex_height*sizeof(uint32_t));

		bindings.push_back(bind);

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
			
			if(bindings[j].Binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				{
				VkDescriptorBufferInfo Buffer_info_UBO{};
				Buffer_info_UBO.buffer = bindings[j].buffers[i].vkbuffer;
				Buffer_info_UBO.offset = 0;
				Buffer_info_UBO.range = bindings[j].buffers[i].mem_size;
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
	void DescriptorsDestroy2()
    {
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
    }
	void SWAPCHAIN_Create(){
		
		VkSurfaceFormatKHR surfaceFormat = {
			VK_FORMAT_R8G8B8A8_UNORM,//VK_FORMAT_B8G8R8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		window_size ={
					static_cast<uint32_t>(sn_interface.destWidth),
					static_cast<uint32_t>(sn_interface.destHeight)
				};

		VkSwapchainCreateInfoKHR createInfoSC{};

			createInfoSC.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			createInfoSC.surface = surface;
			createInfoSC.minImageCount = swap_imageCount;
			createInfoSC.imageFormat = surfaceFormat.format;
			createInfoSC.imageColorSpace = surfaceFormat.colorSpace;
			createInfoSC.imageExtent = window_size;
			createInfoSC.imageArrayLayers = 1;
			createInfoSC.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

		if (graphicsFamily != presentFamily) {
				createInfoSC.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
				createInfoSC.queueFamilyIndexCount = 2;
				createInfoSC.pQueueFamilyIndices = queueFamilyIndices;
			} else {
				createInfoSC.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			}
			createInfoSC.preTransform = capabilities.currentTransform;
			createInfoSC.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
			createInfoSC.presentMode = presentMode;
			createInfoSC.clipped = VK_TRUE;
			createInfoSC.oldSwapchain = VK_NULL_HANDLE;

			err = vkCreateSwapchainKHR(device, &createInfoSC, nullptr, &swapChain);
			if(err != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}

		// STEP 7 IMAGES VIEWS
			
			vkGetSwapchainImagesKHR(device, swapChain, &swap_imageCount, nullptr);
			swapChainImages.resize(swap_imageCount);
			vkGetSwapchainImagesKHR(device, swapChain, &swap_imageCount, swapChainImages.data());
			
			VkFormat swapChainImageFormat = surfaceFormat.format;
			
			swapChainImageViews.resize(swapChainImages.size());

			for (size_t i = 0; i < swapChainImages.size(); i++) {
				VkImageViewCreateInfo createInfo{};
				createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				createInfo.image = swapChainImages[i];
				createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				createInfo.format = swapChainImageFormat;
				createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
				createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				createInfo.subresourceRange.baseMipLevel = 0;
				createInfo.subresourceRange.levelCount = 1;
				createInfo.subresourceRange.baseArrayLayer = 0;
				createInfo.subresourceRange.layerCount = 1;

				err = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);  
				if( err!= VK_SUCCESS){
					throw std::runtime_error("failed to create image views!");
				}
			}

		// STEP 8 RENDER PASS
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(carte_graphique, &memProperties);

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = swapChainImageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			//Mode clear activé
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format = findDepthFormat(carte_graphique);
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef{};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 1;
			renderPassInfo.pAttachments = &colorAttachment;
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 1;
			renderPassInfo.pDependencies = &dependency;

			err = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
			if( err!= VK_SUCCESS){
					throw std::runtime_error("failed to create render pass!!");
				}

			// STEP 9 PIPELINE
			
			CreatePipeline();
			
			// STEP 10 FRAME BUFFERS
			
			swapChainFramebuffers.resize(swapChainImageViews.size());

			for (size_t i = 0; i < swapChainImageViews.size(); i++) {
				VkImageView attachments[] = {
					swapChainImageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = window_size.width;
				framebufferInfo.height = window_size.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create framebuffer!");
				}
			}

			// STEP 11 POOL BUFFER SYNC

			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = graphicsFamily;

			if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create command pool!");
			}

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = swap_imageCount;

			commandBuffers = new VkCommandBuffer[swap_imageCount];
			
			
			if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}

			imgui_commandBuffers = new VkCommandBuffer[swap_imageCount];
			
			if (vkAllocateCommandBuffers(device, &allocInfo, imgui_commandBuffers) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate command buffers!");
			}
		
	//------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//------------------------------------------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
	//-------------------------------------------------------------------------------------------------------------------------------		
		
	for (size_t i = 0; i < swap_imageCount; i++) 
		{	
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapChainFramebuffers[i];
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = window_size;

		VkClearValue clearColor = {{0.3f, 0.3f, 0.3f, 1.0f}};
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;
		
		VkClearValue* pClearColor = &clearColor;
		VkClearValue* pDepthValue = nullptr;

		VkRenderingAttachmentInfoKHR Color = {};
		VkRenderingAttachmentInfo Depth = {};

		Color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		Color.imageView = swapChainImageViews[i];
		Color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Color.resolveMode = VK_RESOLVE_MODE_NONE;
		Color.imageView = VK_NULL_HANDLE;
		Color.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Color.loadOp = pClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pClearColor)
		{
			Color.clearValue = *pClearColor;
		}
		
		Depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		Depth.imageView = VK_NULL_HANDLE;
		Depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		Depth.resolveMode = VK_RESOLVE_MODE_NONE;
		Depth.resolveImageView = VK_NULL_HANDLE;
		Depth.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Depth.loadOp = pDepthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pDepthValue)
		{
			Depth.clearValue = *pDepthValue;
		}

		VkRenderingInfoKHR RenderingInfo = {};
		RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		RenderingInfo.renderArea = {{0,0},{sn_interface.destWidth,sn_interface.destHeight}};
		RenderingInfo.layerCount = 0;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &Color;
		RenderingInfo.pDepthAttachment = &Depth;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) 
			throw std::runtime_error("failed to begin recording command buffer!");

		//SWAPCHAIN_MemoryBarrier(commandBuffers[i],swapChainImages[i],VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);	
		vkCmdBeginRenderPass(commandBuffers[i],&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
			
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float) window_size.width;
		viewport.height = (float) window_size.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = window_size;
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

		vkCmdBindDescriptorSets(commandBuffers[i], 
								VK_PIPELINE_BIND_POINT_GRAPHICS, 
								pipelineLayout, 0, 1, &descriptorsets[i], 0, nullptr);

		//vkCmdBeginRendering(commandBuffers[i],&RenderingInfo);
		vkCmdDraw(commandBuffers[i], 6, 1, 0, 0);
		//vkCmdEndRendering(commandBuffers[i]);
	
		//SWAPCHAIN_MemoryBarrier(commandBuffers[i],swapChainImages[i],VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		vkCmdEndRenderPass(commandBuffers[i]);

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
		}
	}
//------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------	
void SWAPCHAIN_Destroy(){
		
        
		vkFreeCommandBuffers(device,commandPool,swap_imageCount,commandBuffers);
		delete [] commandBuffers;
		vkFreeCommandBuffers(device,commandPool,swap_imageCount,imgui_commandBuffers);
		delete [] imgui_commandBuffers;
		        
		vkDestroyCommandPool(device, commandPool, nullptr);
	
        for (auto framebuffer : swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
            }

        DestroyPipeline();
        vkDestroyRenderPass(device, renderPass, nullptr);

        for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
	
	void SyncObjectsCreate() {
        imageAvailableSemaphores.resize(swap_imageCount);
        renderFinishedSemaphores.resize(swap_imageCount);
        inFlightFences.resize(swap_imageCount);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < swap_imageCount; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
	
	void SyncObjectsDestroy() {
        for (size_t i = 0; i < swap_imageCount; i++)
		{
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }
    }
	void LightUp()
	{
		DescriptorsCreate2();
		SyncObjectsCreate();
	}
}ECRAN;
void EcranOn()
{
	ECRAN.LightUp();
}
void EcranOff()
{
	ECRAN.SWAPCHAIN_Destroy();
	ECRAN.DescriptorsDestroy2();
	
}
void rebuild(bool rebuild)
	{  	
		vkDeviceWaitIdle(device);
		if(rebuild)ECRAN.SWAPCHAIN_Destroy();
		ECRAN.SWAPCHAIN_Create();
	}
static float f = 0.0f;
void sn_Updates(uint32_t imageIndex){     

		static double horloge = 0.0;
		horloge += sn_interface.delta_time * (double)f;// * glm::two_pi<float>();
				
		glm::mat4 Model = glm::mat4(1.0f);//glm::rotate(glm::mat4(1.0f), horloge, glm::vec3(0, 0, 1.f));
		glm::mat4 project = glm::scale(glm::mat4(1.0f), glm::vec3(sn_interface.aspectH, sn_interface.aspectV, 1.f));
		glm::mat4 trans = glm::translate(glm::mat4(1.0f),glm::vec3(sn_interface.wfX, sn_interface.wfY, 0.f));
		
		Model = trans * project * glm::rotate(glm::mat4(1.0f), (float)horloge, glm::vec3(0, 0, 1.f));

		memcpy(ECRAN.bindings[0].buffers[imageIndex].mem_ptr,glm::value_ptr(Model),sizeof(glm::mat4));
}
bool show_demo_window = true;
bool show_another_window = false;
void sn_Vulkandraw(){        

	static uint32_t imageIndex = 0;
	vkWaitForFences(device, 1, &ECRAN.inFlightFences[ECRAN.currentFrame], VK_TRUE, UINT64_MAX);
	err = vkAcquireNextImageKHR(device, 
								ECRAN.swapChain, 
								UINT64_MAX, 
								ECRAN.imageAvailableSemaphores[ECRAN.currentFrame], 
								VK_NULL_HANDLE, 
								&imageIndex);
		if(err == VK_ERROR_OUT_OF_DATE_KHR)
			{
				std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
				sn_interface.resizing = false;
				rebuild(true);
				//return;
			}else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
			
            throw std::runtime_error("failed to acquire swap chain image!");
        }
	vkResetFences(device, 1, &ECRAN.inFlightFences[ECRAN.currentFrame]);
	if(err == VK_ERROR_DEVICE_LOST)std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
	if(err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)std::cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << std::endl;
	if(err == VK_ERROR_OUT_OF_DATE_KHR)std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
	if(err == VK_ERROR_OUT_OF_DEVICE_MEMORY)std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	if(err == VK_ERROR_OUT_OF_HOST_MEMORY)std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	if(err == VK_ERROR_SURFACE_LOST_KHR)std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
	if(err == VK_ERROR_UNKNOWN)std::cout << "VK_ERROR_UNKNOWN" << std::endl;
	
	
	
	
	//if(err == VK_ERROR_VALIDATION_FAILED)std::cout << "VK_ERROR_VALIDATION_FAILED" << std::endl;
	// OFFLINE (1340)
		//vkBeginCommandBuffer
		//vkCmdPipelineBarrier
		
		//vkCmdBindPipeline
		//vkCmdBindDescriptorSets
		//vkCmdDraw
		
		//vkCmdPipelineBarrier
		//vkEndCommandBuffer
			
	// ONLINE (1429)
		//vkAcquireNextImageKHR
		//Update uniforms
		//vkQueueSubmit
		//vkQueuePresentKHR

	
#ifdef USE_IMGUI_PLEASE_IFYOUCAN
		ImGuiIO& io = ImGui::GetIO();
		
		// Start the Dear ImGui frame
        ImGui_ImplVulkan_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		
		
        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
           static int counter = 0;

			io.MouseDrawCursor = true;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

			static float clr_color[3] = {0.1f,0.6f,0.8f};
            ImGui::SliderFloat("float", &f, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clr_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
			ImGui::SameLine();
            ImGui::Text("ECRAN.currentFrame = %d", ECRAN.currentFrame);
			ImGui::SameLine();
            ImGui::Text("imageIndex = %d", imageIndex);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
		
        ImDrawData* draw_data = ImGui::GetDrawData();
		
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
      
     	vkResetCommandBuffer(ECRAN.imgui_commandBuffers[imageIndex],VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		//err = vkResetCommandPool(device, ECRAN.commandPool, 0);
        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        err = vkBeginCommandBuffer(ECRAN.imgui_commandBuffers[imageIndex], &info);
      
    	VkClearValue clear_color = {};

        VkRenderPassBeginInfo RPinfo = {};
        
		RPinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        RPinfo.renderPass = imgui_renderPass;
        RPinfo.framebuffer = ECRAN.swapChainFramebuffers[imageIndex];
        RPinfo.renderArea.extent.width = sn_interface.destWidth;
        RPinfo.renderArea.extent.height = sn_interface.destHeight;
        RPinfo.clearValueCount = 1;
        RPinfo.pClearValues = &clear_color;
		
        //vkCmdBeginRenderPass(ECRAN.imgui_commandBuffers[imageIndex], &RPinfo, VK_SUBPASS_CONTENTS_INLINE);
		
		VkClearValue* pClearColor = nullptr;
		VkClearValue* pDepthValue = nullptr;

		VkRenderingAttachmentInfoKHR Color = {};
		VkRenderingAttachmentInfo Depth = {};
		Color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
		Depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		Color.imageView = ECRAN.swapChainImageViews[imageIndex];
		Color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		Color.resolveMode = VK_RESOLVE_MODE_NONE;
		Color.imageView = VK_NULL_HANDLE;
		Color.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Color.loadOp = pClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pClearColor)
		{
			Color.clearValue = *pClearColor;
		}
		
		Depth.imageView = VK_NULL_HANDLE;
		Depth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		Depth.resolveMode = VK_RESOLVE_MODE_NONE;
		Depth.resolveImageView = VK_NULL_HANDLE;
		Depth.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		Depth.loadOp = pDepthValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
		Depth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		if(pDepthValue)
		{
			Depth.clearValue = *pDepthValue;
		}
		
	
		VkRenderingInfoKHR RenderingInfo = {};
		RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		RenderingInfo.renderArea = {{0,0},{sn_interface.destWidth,sn_interface.destHeight}};
		RenderingInfo.layerCount = 0;
		RenderingInfo.viewMask = 0;
		RenderingInfo.colorAttachmentCount = 1;
		RenderingInfo.pColorAttachments = &Color;
		RenderingInfo.pDepthAttachment = &Depth;
		
		//vkCmdBeginRendering(ECRAN.imgui_commandBuffers[imageIndex],&RenderingInfo);
		ImGui_ImplVulkan_RenderDrawData(draw_data, ECRAN.imgui_commandBuffers[imageIndex],VK_NULL_HANDLE);
		//vkCmdEndRendering(ECRAN.imgui_commandBuffers[imageIndex]);
		
  		/*ECRAN.SWAPCHAIN_MemoryBarrier(ECRAN.imgui_commandBuffers[imageIndex],
									ECRAN.swapChainImages[imageIndex],
									VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
									VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);*/
		
		//Submit command buffer
		//vkCmdEndRenderPass(ECRAN.imgui_commandBuffers[imageIndex]);
		
		err = vkEndCommandBuffer(ECRAN.imgui_commandBuffers[imageIndex]);

		if(err != VK_SUCCESS)
		{
			
			if(err == VK_ERROR_OUT_OF_HOST_MEMORY)throw std::runtime_error("Marche pas le vkEndCommandBuffer de Imgui!");
		}
	VkCommandBuffer batch[2] = {ECRAN.commandBuffers[imageIndex],ECRAN.imgui_commandBuffers[imageIndex]};
	#else
	VkCommandBuffer batch[1] = {ECRAN.commandBuffers[imageIndex]};
    #endif


	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &ECRAN.imageAvailableSemaphores[ECRAN.currentFrame];
	submitInfo.pWaitDstStageMask = waitStages;
	
	#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	submitInfo.commandBufferCount = 2;
	#else
	submitInfo.commandBufferCount = 1;
	#endif

	submitInfo.pCommandBuffers = batch;//&ECRAN.commandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &ECRAN.renderFinishedSemaphores[ECRAN.currentFrame];

	sn_Updates(imageIndex);

	err = vkQueueSubmit(graphicsQueue, 1, &submitInfo, ECRAN.inFlightFences[ECRAN.currentFrame]);

	if (err != VK_SUCCESS) {
			if(err == VK_ERROR_OUT_OF_HOST_MEMORY)throw std::runtime_error("VK_ERROR_OUT_OF_HOST_MEMORY");
			if(err == VK_ERROR_OUT_OF_DEVICE_MEMORY)throw std::runtime_error("VK_ERROR_OUT_OF_DEVICE_MEMORY");
			if(err == VK_ERROR_DEVICE_LOST)throw std::runtime_error("VK_ERROR_DEVICE_LOST");
			throw std::runtime_error("failed to submit draw command buffer!");
        }

		//Tu vas attendre sur ce semaphore pour présenter l'image
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &ECRAN.imageAvailableSemaphores[ECRAN.currentFrame];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &ECRAN.swapChain;
		presentInfo.pImageIndices = &imageIndex;

		err = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
          
            sn_interface.resizing = false;
			rebuild(true);
        } else if (err != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
		vkQueueWaitIdle(presentQueue);
        ECRAN.currentFrame = (ECRAN.currentFrame + 1) % swap_imageCount;
			
}

