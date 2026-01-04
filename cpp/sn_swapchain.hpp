struct snCamera
{
	glm::vec3 v3_position = glm::vec3(0.0f);
	glm::vec3 v3_forward;
	glm::vec3 v3_right;
	glm::vec3 v3_up;
	
	glm::mat4 m4_projection = glm::mat4(0.0);
	glm::quat qt_orientation = glm::quat(glm::vec3(0.0f));
	glm::mat4 m4_Rotation = glm::mat4(1.f);
	
	float focale = 50.f; //mm
	float fov = 2.f * glm::atan(18.f/focale);
	
	float z_near = 0.1f;
	float z_far = 50.0f;

	float m_yaw = glm::pi<float>() * 0;
	float m_pitch = glm::pi<float>() * 0;
	float m_limitpitch = glm::pi<float>()*0.5f-0.0001f;

	void Proceed_Keys()
	{
		if(sn_interface.Keys[17].pressed)
		{
			v3_position -= v3_forward * (float)sn_interface.delta_time;
		}
		if(sn_interface.Keys[31].pressed)
		{
			v3_position += v3_forward * (float)sn_interface.delta_time;
		}
		if(sn_interface.Keys[32].pressed)
		{
			v3_position += v3_right * (float)sn_interface.delta_time;
		}
		if(sn_interface.Keys[30].pressed)
		{
			v3_position -= v3_right * (float)sn_interface.delta_time;
		}
		if(sn_interface.Keys[46].pressed)
		{
			v3_position += v3_up * (float)sn_interface.delta_time;
		}
		if(sn_interface.Keys[18].pressed)
		{
			v3_position -= v3_up * (float)sn_interface.delta_time;
		}
		if(sn_interface.Keys[74].pressed)
		{
			Change_fov(-sn_interface.delta_time);
		}
		if(sn_interface.Keys[78].pressed)
		{
			Change_fov(sn_interface.delta_time);
		}
	}
	void Change_fov(float move)
	{
		focale+=move * 4.f;
		fov = 2.f * glm::atan(18.f/focale);
	}
	void Corrige_Assiette()
	{
		glm::vec3 v3_corr = glm::cross(v3_up,glm::vec3(0,0,1));
		float lenght = glm::dot(v3_corr,v3_corr);
		if(lenght > 1.17549e-38)
		{
			
			float cor_sin = glm::sin(lenght * sn_interface.delta_time);
			float cor_cos = glm::cos(lenght * sn_interface.delta_time);
			v3_corr = glm::normalize(v3_corr);

			qt_orientation = glm::normalize(
			qt_orientation * glm::quat(cor_cos,
				  cor_sin*v3_corr.x,
				  cor_sin*v3_corr.y,
				  cor_sin*v3_corr.z));
		}
	}
	void Orientation1(float yaw = .0f,float pitch =.0f,float roll=.0f)
	{
		qt_orientation = glm::lookAtLH(v3_position,v3_position+v3_forward,v3_up);
		
		float yaw_sin = glm::sin(yaw);
		float yaw_cos = glm::cos(yaw);
		float pitch_sin = glm::sin(pitch);
		float pitch_cos = glm::cos(pitch);
		
		qt_orientation = glm::normalize(
		qt_orientation *
		glm::quat(yaw_cos,
				  yaw_sin*v3_up.x,
				  yaw_sin*v3_up.y,
				  yaw_sin*v3_up.z) *
		glm::quat(pitch_cos,
				  pitch_sin*v3_right.x,
				  pitch_sin*v3_right.y,
				  pitch_sin*v3_right.z));
				  
		Corrige_Assiette();
		
		//Get Rotation matrix
		m4_Rotation = glm::mat4_cast(qt_orientation);
		//Get Axis
		v3_right = glm::normalize(glm::vec3(m4_Rotation[0][0],m4_Rotation[1][0],m4_Rotation[2][0]));
		v3_up = glm::normalize(glm::vec3(m4_Rotation[0][1],m4_Rotation[1][1],m4_Rotation[2][1]));
		v3_forward =glm::normalize(glm::vec3(m4_Rotation[0][2],m4_Rotation[1][2],m4_Rotation[2][2]));
	}
	void Orientation2(float yaw = .0f,float pitch =.0f,float roll=.0f)
	{
		m_yaw -= yaw * 2.f;
		m_pitch += pitch * 2.f;

		if(m_pitch > m_limitpitch)
		m_pitch = m_limitpitch;
		if(m_pitch < -m_limitpitch)
		m_pitch = -m_limitpitch;
		
		float yaw_sin = glm::sin(m_yaw);
		float yaw_cos = glm::cos(m_yaw);
		float pitch_sin = glm::sin(m_pitch);
		float pitch_cos = glm::cos(m_pitch);
		
		v3_forward.x = yaw_sin * pitch_cos;
       	v3_forward.z = pitch_sin;
		v3_forward.y = yaw_cos * pitch_cos;
        
		v3_right.x = glm::sin(m_yaw + 1.5707963263216916398f);
       	v3_right.z = 0.f;
		v3_right.y = glm::cos(m_yaw + 1.5707963263216916398f);
        
		v3_up = glm::normalize(glm::cross(v3_forward,v3_right));
		qt_orientation = glm::lookAtLH(v3_position,v3_position+v3_forward,v3_up);
		//Get Rotation matrix
		m4_Rotation = glm::mat4_cast(qt_orientation);
		//Get Axis
		v3_right = glm::normalize(glm::vec3(m4_Rotation[0][0],m4_Rotation[1][0],m4_Rotation[2][0]));
		v3_up = glm::normalize(glm::vec3(m4_Rotation[0][1],m4_Rotation[1][1],m4_Rotation[2][1]));
		v3_forward =glm::normalize(glm::vec3(m4_Rotation[0][2],m4_Rotation[1][2],m4_Rotation[2][2]));
	}
	glm::mat4 LookAtFirstPerson()
	{
		m4_projection = glm::perspectiveLH(fov,
										sn_interface.aspectratio,
										z_near,
										z_far);
		m4_projection = glm::perspective(fov, sn_interface.aspectratio, z_near, z_far);

		glm::mat4 Translation = glm::translate(glm::mat4(1.0), -v3_position);
		
		glm::mat4 World = m4_Rotation * Translation;
		return World;
	}
}camera;
struct snSwapChain
{
	VkExtent2D window_size;
	VkSwapchainKHR swapChain{NULL};     
    
	//MEMORY
	
	std::vector<VkImage>		SN_ARRAY_BY_FRAME swapChainImages;
    std::vector<VkImageView> 	SN_ARRAY_BY_FRAME swapChainImageViews;
    std::vector<VkFramebuffer> 	SN_ARRAY_BY_FRAME swapChainFramebuffers;
	struct {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	} Zbuffer;

	//DRAWING
	VkRenderPass renderPass;
		
	uint32_t currentFrame = 0;
	std::vector<snPipeline>	pipelines;
	
	VkCommandBuffer SN_ARRAY_BY_FRAME 		*commandBuffers = nullptr; 		
	VkCommandBuffer SN_ARRAY_BY_FRAME 		*imgui_commandBuffers = nullptr; 
	
	std::vector<LWCMODEL> models;
	snBuffer Array;	 				
	
	//SYNCHRO
	VkSemaphore* SN_ARRAY_BY_FRAME renderSema{VK_NULL_HANDLE};
    VkSemaphore presentSema{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};
		
	UBOVS uboVS;

void LoadAssets()
	{
		LWCMODEL* pModel = nullptr;
		uint32_t num_vertices = readLwcFile("assets/objets/monsters/ikmommy_low90.lwc",&pModel);
		if(pModel)
		{
			snBuffer vertex;
			vertex.CreateVertexBuffer(pModel->Vertices.mem_size);
			vertex.CopyFrom(pModel->Vertices);
			pModel->Vertices.Release();
			pModel->Vertices = vertex;
			if(pModel->Indexes.vkbuffer)
				{
				snBuffer index;
				index.CreateIndexBuffer(pModel->Indexes.mem_size);	
				index.CopyFrom(pModel->Indexes);
				pModel->Indexes.Release();
				pModel->Indexes = index;
				}
			models.push_back(*pModel);
		}

		Vertex* pVertex;
		uint32_t num_vertex = Read_Obj((void**)&pVertex,nullptr);
		if(pVertex)
		{
			snBuffer stage;
			stage.CreateTransferBuffer((void*)pVertex,
										num_vertex*sizeof(Vertex));
			
			LWCMODEL* pModel2 = new LWCMODEL;
			pModel2->Vertices.CreateVertexBuffer(num_vertex*sizeof(Vertex));
			pModel2->Vertices.CopyFrom(stage);
			pModel2->dwNumVertex = num_vertex;
			stage.Release();
			delete [] pVertex;
			models.push_back(*pModel2);
		}

		const int total_textures = 8;
		int w,h,c;
		snBuffer textures[total_textures];
		snTextureArray TextureArray(total_textures);
		textures[0].AddArrayElm("assets/textures/tex_sol0.bmp",&w,&h,&c);TextureArray.Set(0,w,h);
		textures[1].AddArrayElm("assets/textures/tex_sol1.bmp",&w,&h,&c);TextureArray.Set(1,w,h);
		textures[2].AddArrayElm("assets/textures/tex_mur0.bmp",&w,&h,&c);TextureArray.Set(2,w,h);
		textures[3].AddArrayElm("assets/textures/tex_mur1.bmp",&w,&h,&c);TextureArray.Set(3,w,h);
		textures[4].AddArrayElm("assets/textures/tex_por.bmp",&w,&h,&c);TextureArray.Set(4,w,h);
		textures[5].AddArrayElm("assets/textures/tex_pla.bmp",&w,&h,&c);TextureArray.Set(5,w,h);
		textures[6].AddArrayElm("assets/textures/tex_mou.bmp",&w,&h,&c);TextureArray.Set(6,w,h);
		textures[7].AddArrayElm("assets/textures/tex_obj.bmp",&w,&h,&c);TextureArray.Set(7,w,h);
		Array.CreateTextureArray(textures,8,w,h,c,&TextureArray);
		for(int i=0;i<total_textures;i++)
		{
			textures[i].Release();
		}
	
	
	}

void Pipelines_create()
	{
	snPipeline pm;
	
	pm.Reset();
	pm.Prepare();
	pm.ReserveShaders(2);
	pm.LoadShader(0,"shaders/vert.spv","main",VK_SHADER_STAGE_VERTEX_BIT);
	pm.LoadShader(1,"shaders/frag.spv","main",VK_SHADER_STAGE_FRAGMENT_BIT);

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

	pm.Descriptor.bindings.push_back(bind);

	bind.Binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bind.Binding.binding = 1;
	bind.Binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bind.Binding.descriptorCount = 1;
	bind.Binding.pImmutableSamplers = nullptr;
	bind.count = 1;
	bind.buffers = new snBuffer;

	uint32_t tex_width;
	uint32_t tex_height;
	
	loadTexturelunarG("pipo", NULL, &tex_width, &tex_height);
	uint8_t* data = new uint8_t[tex_width*tex_height*4];
	loadTexturelunarG("pipo", data, &tex_width, &tex_height);
	
	/*bind.buffers[0].CreateTextureFrom(
		tex_width,
		tex_height,
		data,
		tex_width*tex_height*sizeof(uint32_t));*/

		bind.buffers[0].CreateTexture("assets/textures/tex_mur3.bmp");

	pm.Descriptor.bindings.push_back(bind);
	pm.GfxPipelineInfo.subpass = 1;
	pm.Descriptor.DescriptorsCreate();
	pm.Create(renderPass);
	
	pipelines.push_back(pm);

	//Second pipeline***********************************************************************************************************************

	snPipeline pm2;
	pm2.Reset();
	pm2.Prepare();
	pm2.ReserveShaders(2);
	pm2.LoadShader(0,"shaders/instancing.vert.spv","main",VK_SHADER_STAGE_VERTEX_BIT);
	pm2.LoadShader(1,"shaders/instancing.frag.spv","main",VK_SHADER_STAGE_FRAGMENT_BIT);
	pm2.vertexInputInfo = Vertex::VertexInputState();

	bind.count = 1;
	bind.buffers = new snBuffer;
	bind.buffers->CreateUniformBuffer(&uboVS,sizeof(UBOVS));
	bind.Binding = bind.buffers->BindLayout(0);
	pm2.Descriptor.bindings.push_back(bind);
	
	bind.count = 1;
	bind.buffers = new snBuffer;
	bind.buffers->CreateTextureInstance(Array);
	bind.Binding = bind.buffers->BindLayout(1);
	pm2.Descriptor.bindings.push_back(bind);
	
	pm2.Descriptor.DescriptorsCreate();
	
	pm2.PushConstants(1);
	pm2.PushConstantsInfo(0,sizeof(MeshPushConstants),VK_SHADER_STAGE_VERTEX_BIT);

	//Rasterizer
	pm2.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	pm2.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	pm2.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;//VK_FRONT_FACE_CLOCKWISE VK_FRONT_FACE_COUNTER_CLOCKWISE
	//Z Buffer
	VkPipelineDepthStencilStateCreateInfo Dinfo = {};
	Dinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	Dinfo.depthTestEnable = VK_TRUE;
	Dinfo.depthWriteEnable = VK_TRUE;
	Dinfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	Dinfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	pm2.GfxPipelineInfo.pDepthStencilState = &Dinfo;
	
	pm2.Create(renderPass);
	pipelines.push_back(pm2);
	}

void Pipelines_destroy()
    {
		for(volatile int c=0;c<pipelines.size();c++)
		{
			pipelines[c].Release();
		}
		pipelines.clear();
    }
//FREE_DESCRIPTOR_SET_BIT


//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------	
void commandbuffers_create()
{
		// STEP 11 POOL BUFFER SYNC
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = pdevice->commandPool;
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
			
		for (size_t i = 0; i < swap_imageCount; i++) 
		{	
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		
		VkClearValue clearValues[2];
		clearValues[0].color = {{0.3f, 0.3f, 0.3f, 1.0f}};
		clearValues[1].depthStencil = { 1.0f, 0 };
		
		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapChainFramebuffers[i];
		renderPassBeginInfo.renderArea.offset = {0, 0};
		renderPassBeginInfo.renderArea.extent = window_size;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		
		/*VkClearValue* pClearColor = &clearColor;
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
		RenderingInfo.pDepthAttachment = &Depth;*/

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) 
			throw std::runtime_error("failed to begin recording command buffer!");

		//SWAPCHAIN_MemoryBarrier(commandBuffers[i],swapChainImages[i],VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);	
		vkCmdBeginRenderPass(commandBuffers[i],&renderPassBeginInfo,VK_SUBPASS_CONTENTS_INLINE);
		
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
				
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[1].Pipe);
		vkCmdBindDescriptorSets(commandBuffers[i], 
								VK_PIPELINE_BIND_POINT_GRAPHICS, 
								pipelines[1].PipeLayout, 0, 1, &pipelines[1].Descriptor.descriptorsets[i], 0, nullptr);
		
		MeshPushConstants constants;
		constants.data.x = 0;
		constants.model = glm::mat4{ 1.0f };
		
		vkCmdPushConstants(commandBuffers[i], pipelines[1].PipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		models[0].Draw(commandBuffers[i]);
		
		constants.data.x = 2;
		constants.model = glm::rotate(glm::mat4{ 1.0f }, 0.2f, glm::vec3(0, 1, 0));
		
		vkCmdPushConstants(commandBuffers[i], pipelines[1].PipeLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		models[1].Draw(commandBuffers[i]);
				
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0].Pipe);
		vkCmdNextSubpass(commandBuffers[i],VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindDescriptorSets(commandBuffers[i], 
								VK_PIPELINE_BIND_POINT_GRAPHICS, 
								pipelines[0].PipeLayout, 0, 1, &pipelines[0].Descriptor.descriptorsets[i], 0, nullptr);

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
void commandbuffers_destroy()
{
		if(commandBuffers)
			{
			vkFreeCommandBuffers(device,pdevice->commandPool,swap_imageCount,commandBuffers);
			delete [] commandBuffers;
			commandBuffers = nullptr;
			}
		if(imgui_commandBuffers)
			{
			vkFreeCommandBuffers(device,pdevice->commandPool,swap_imageCount,imgui_commandBuffers);
			delete [] imgui_commandBuffers;
			imgui_commandBuffers = nullptr;
			}

		
}
//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------		
//-------------------------------------------------------------------------------------------------------------------------------	

void DepthStencil_Create()
{
	VkFormat depthFormat = findDepthFormat(carte_graphique);
    
	VkImageCreateInfo imageCI{};
	imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCI.imageType = VK_IMAGE_TYPE_2D;
	imageCI.format = depthFormat;
	imageCI.extent = { sn_interface.destWidth, sn_interface.destHeight, 1 };
	imageCI.mipLevels = 1;
	imageCI.arrayLayers = 1;
	imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	err = vkCreateImage(device, &imageCI, nullptr, &Zbuffer.image);
	VkMemoryRequirements memReqs{};
	vkGetImageMemoryRequirements(device, Zbuffer.image, &memReqs);

	VkMemoryAllocateInfo memAllloc{};
	memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllloc.allocationSize = memReqs.size;
	memAllloc.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); 
	err = vkAllocateMemory(device, &memAllloc, nullptr, &Zbuffer.mem);
	err = vkBindImageMemory(device, Zbuffer.image, Zbuffer.mem, 0);

	VkImageViewCreateInfo imageViewCI{};
	imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCI.image = Zbuffer.image;
	imageViewCI.format = depthFormat;
	imageViewCI.subresourceRange.baseMipLevel = 0;
	imageViewCI.subresourceRange.levelCount = 1;
	imageViewCI.subresourceRange.baseArrayLayer = 0;
	imageViewCI.subresourceRange.layerCount = 1;
	imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	// Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
	if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
		imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	err = vkCreateImageView(device, &imageViewCI, nullptr, &Zbuffer.view);
}
void DepthStencil_Destroy()
{
	vkDestroyImageView(device, Zbuffer.view, nullptr);
	vkDestroyImage(device, Zbuffer.image, nullptr);
	vkFreeMemory(device, Zbuffer.mem, nullptr);
}
void renderpasses_Create()
{

			VkSurfaceFormatKHR surfaceFormat = {
			VK_FORMAT_R8G8B8A8_UNORM,//VK_FORMAT_B8G8R8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
			};
					
			// STEP 8 RENDER PASS
			VkPhysicalDeviceMemoryProperties memProperties;
			vkGetPhysicalDeviceMemoryProperties(carte_graphique, &memProperties);

			VkAttachmentDescription colorAttachment{};
			colorAttachment.format = surfaceFormat.format;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD; //VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			//Mode clear activé
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			VkAttachmentDescription Zbuffer{};
			Zbuffer.format = findDepthFormat(carte_graphique);
			Zbuffer.samples = VK_SAMPLE_COUNT_1_BIT;
			Zbuffer.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			Zbuffer.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Zbuffer.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Zbuffer.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			Zbuffer.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			Zbuffer.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentDescription attachs[2];
			attachs[0] = colorAttachment;
			attachs[1] = Zbuffer;

			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass[2] = {};
			subpass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass[0].colorAttachmentCount = 1;
			subpass[0].pColorAttachments = &colorReference;
			subpass[0].pDepthStencilAttachment = &depthReference;
			subpass[0].inputAttachmentCount = 0;
			subpass[0].pInputAttachments = nullptr;
			subpass[0].preserveAttachmentCount = 0;
			subpass[0].pPreserveAttachments = nullptr;
			subpass[0].pResolveAttachments = nullptr;
			
			subpass[1] = subpass[0];
			subpass[1].pDepthStencilAttachment = 0;

			VkSubpassDependency dependencies[2];
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].dstSubpass = 0;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].srcAccessMask = 0;
			dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = 2;
			renderPassInfo.pAttachments = attachs;
			renderPassInfo.subpassCount = 2;
			renderPassInfo.pSubpasses = subpass;
			renderPassInfo.dependencyCount = 2;
			renderPassInfo.pDependencies = dependencies;
			
			err = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
			if( err!= VK_SUCCESS){
					throw std::runtime_error("failed to create render pass!!");
				}

}
void renderpasses_Destroy()
{
 	vkDestroyRenderPass(device, renderPass, nullptr);
}
void SyncObjects_Create() {
		VkSemaphoreCreateInfo semaphoreInfo{};
        	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
			fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if(!renderSema)renderSema = new VkSemaphore[swap_imageCount];
		
		for(int i=0;i<swap_imageCount;i++)
		{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderSema[i]) != VK_SUCCESS)
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
		
		if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &presentSema) != VK_SUCCESS)
			throw std::runtime_error("failed to create synchronization objects for a frame!");

		if(vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		
        }
    }
void SyncObjects_Destroy() {
       
	for(int i=0;i<swap_imageCount;i++)
		{
          
            if(renderSema)
			if(renderSema[i])
				{
					vkDestroySemaphore(device, renderSema[i], nullptr);
					renderSema[i] = VK_NULL_HANDLE;
				}
		}
         
			if(presentSema)
				{
					vkDestroySemaphore(device, presentSema, nullptr);
					presentSema = VK_NULL_HANDLE;
				}
	    if(inFlightFence)
				{
					vkDestroyFence(device, inFlightFence, nullptr);
					inFlightFence = VK_NULL_HANDLE;
				}
       
    }
void SWAPCHAIN_Create(){
	

		
	// Store the current swap chain handle so we can use it later on to ease up recreation
	VkSwapchainKHR oldSwapchain = swapChain;

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps;
	err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(carte_graphique, surface, &surfCaps);

	// Get available present modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(carte_graphique, surface, &presentModeCount, NULL);
	assert(presentModeCount > 0);

	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(carte_graphique, surface, &presentModeCount, presentModes.data());
	
	VkExtent2D swapchainExtent = {};
	// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
	if (surfCaps.currentExtent.width == (uint32_t)-1)
	{
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = sn_interface.destWidth;
		swapchainExtent.height = sn_interface.destHeight;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCaps.currentExtent;
		sn_interface.destWidth = surfCaps.currentExtent.width;
		sn_interface.destHeight = surfCaps.currentExtent.height;
	}

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	bool vsync = false;
	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!vsync)
	{
		for (size_t i = 0; i < presentModeCount; i++)
		{
			if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
	}

		// Determine the number of images
	uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
	if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
	{
		desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
	}

	// Find the transformation of the surface
	VkSurfaceTransformFlagsKHR preTransform;
	if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		// We prefer a non-rotated transform
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfCaps.currentTransform;
	}

	// Find a supported composite alpha format (not all devices support alpha opaque)
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Simply select the first composite alpha format available
	std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	for (auto& compositeAlphaFlag : compositeAlphaFlags) {
		if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag) {
			compositeAlpha = compositeAlphaFlag;
			break;
		};
	}

	VkSurfaceFormatKHR surfaceFormat = {
			VK_FORMAT_R8G8B8A8_UNORM,//VK_FORMAT_B8G8R8A8_SRGB,
			VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
		};
		
	VkSwapchainCreateInfoKHR swapchainCI = {};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.surface = surface;
	swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCI.imageFormat = surfaceFormat.format;
	swapchainCI.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
	swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
	swapchainCI.imageArrayLayers = 1;
	swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCI.queueFamilyIndexCount = 0;
	swapchainCI.presentMode = swapchainPresentMode;
	// Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse and makes sure that we can still present already acquired images
	swapchainCI.oldSwapchain = oldSwapchain;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	swapchainCI.clipped = VK_TRUE;
	swapchainCI.compositeAlpha = compositeAlpha;

	// Enable transfer source on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
		swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Enable transfer destination on swap chain images if supported
	if (surfCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	swap_imageCount = desiredNumberOfSwapchainImages;

	err = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapChain);
	if(err != VK_SUCCESS) {
				throw std::runtime_error("failed to create swap chain!");
			}
	
	window_size.width = swapchainExtent.width;
	window_size.height = swapchainExtent.height;

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE) 
	{ 
		for (uint32_t i = 0; i < swap_imageCount; i++)
		{
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
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
	
	swapChainFramebuffers.resize(swapChainImageViews.size());

	if (oldSwapchain != VK_NULL_HANDLE)
	{ 
		for (auto framebuffer : swapChainFramebuffers) {
            	vkDestroyFramebuffer(device, framebuffer, nullptr);
            }   
		DepthStencil_Destroy();
	}
	DepthStencil_Create();

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		
		VkImageView attachments[] = {
			swapChainImageViews[i],
			Zbuffer.view
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 2;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchainExtent.width;
		framebufferInfo.height = swapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}
void SWAPCHAIN_Resize()
	{  	
		
		vkDeviceWaitIdle(device);
		
		SWAPCHAIN_Create();

		commandbuffers_destroy();
		commandbuffers_create();
		SyncObjects_Destroy();
		SyncObjects_Create();
		
		vkDeviceWaitIdle(device);
		
	}
void SWAPCHAIN_Destroy(){
					
		if(renderSema)delete [] renderSema;
		renderSema = nullptr;
		
		DepthStencil_Destroy();
		
		for (auto framebuffer : swapChainFramebuffers) {
            	vkDestroyFramebuffer(device, framebuffer, nullptr);
            }       

        for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            
        vkDestroySwapchainKHR(device, swapChain, nullptr);
    }
void LightUp()
	{
		snCommand::TransfertCommandPool_Create();

		LoadAssets();
		
		renderpasses_Create(); // II
		Pipelines_create(); // III
	
		SWAPCHAIN_Resize(); 
		
		camera.v3_position = glm::vec3(0,0.5f,0.5f);
	}
void LightDown()
	{
		vkDeviceWaitIdle(device);
		
		for(auto model : models)
		{
			model.Release();
		}
		models.clear();
		Array.Release();
		SyncObjects_Destroy();
		SWAPCHAIN_Destroy();
		commandbuffers_destroy();
		Pipelines_destroy(); // III
		renderpasses_Destroy(); // II

		snCommand::TransfertCommandPool_Destroy();
		
		vkDeviceWaitIdle(device);
	}
}ecran;
void EcranOn()
{
	ecran.LightUp();
}
void EcranOff()
{
	ecran.LightDown();
	
}
static float f = 0.0f;
bool show_demo_window = true;
bool show_another_window = false;
void sn_Updates(uint32_t imageIndex){     

		static double horloge = 0.0;
		horloge += sn_interface.delta_time * (double)f;// * glm::two_pi<float>();
				
		glm::mat4 Model = glm::mat4(1.0f);//glm::rotate(glm::mat4(1.0f), horloge, glm::vec3(0, 0, 1.f));
		glm::mat4 project = glm::scale(glm::mat4(1.0f), glm::vec3(sn_interface.aspectH, sn_interface.aspectV, 1.f));
		glm::mat4 trans = glm::translate(glm::mat4(1.0f),glm::vec3(sn_interface.wfX, sn_interface.wfY, 0.f));
		
		Model = trans * project * glm::rotate(glm::mat4(1.0f), (float)horloge, glm::vec3(0, 0, 1.f));

		memcpy(ecran.pipelines[0].Descriptor.bindings[0].buffers[imageIndex].mem_ptr,
		glm::value_ptr(Model),sizeof(glm::mat4));

		sn_interface.mdX * (float)sn_interface.delta_time;
		
		camera.Proceed_Keys();
		camera.Orientation2((float)sn_interface.mdX * (float)sn_interface.delta_time,(float)-sn_interface.mdY * (float)sn_interface.delta_time,0.f);
		sn_interface.mdX = 0;sn_interface.mdY = 0;
		glm::mat4 WorldView = camera.LookAtFirstPerson();
		ecran.uboVS.projection = 	camera.m4_projection;
		ecran.uboVS.view = 			WorldView;
		memcpy(ecran.pipelines[1].Descriptor.bindings[0].buffers[0].mem_ptr,
			   &ecran.uboVS,
			   ecran.pipelines[1].Descriptor.bindings[0].buffers[0].mem_size);
}
inline void sn_VulkandrawOverlay(uint32_t iIndex = 0)
{     
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
		ImGui::SliderFloat("Rotation", &f, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clr_color); // Edit 3 floats representing a color
		ImGui::Text("EYE : X %f Y %f Z %f", camera.v3_position.x, camera.v3_position.y, camera.v3_position.z);
		ImGui::Text("FRONT : X %f Y %f Z %f", camera.v3_forward.x, camera.v3_forward.y, camera.v3_forward.z);
		ImGui::Text("Focale : X %f mm",camera.focale);
		
		
		
		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);
		ImGui::SameLine();
		ImGui::Text("ECRAN.currentFrame = %d", ecran.currentFrame);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		static quat qrot1 = quat(1.0f,0.f,0.f,0.f);
		//ImGui::gizmo3D("##gizmo1",qrot1,200.f);

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

	vkResetCommandBuffer(ecran.imgui_commandBuffers[iIndex],VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	//err = vkResetCommandPool(device, ECRAN.commandPool, 0);
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	err = vkBeginCommandBuffer(ecran.imgui_commandBuffers[iIndex], &info);

	VkClearValue clear_color = {};
	VkClearValue* pClearColor = nullptr;
	VkClearValue* pDepthValue = nullptr;

	VkRenderingAttachmentInfoKHR Color = {};
	VkRenderingAttachmentInfo Depth = {};
	Color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
	Depth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	Color.imageView = ecran.swapChainImageViews[iIndex];
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

	VkRenderPassBeginInfo RPinfo = {};
	RPinfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	RPinfo.renderPass = ecran.renderPass;
	RPinfo.framebuffer = ecran.swapChainFramebuffers[iIndex];
	RPinfo.renderArea.extent.width = sn_interface.destWidth;
	RPinfo.renderArea.extent.height = sn_interface.destHeight;
	RPinfo.clearValueCount = 1;
	RPinfo.pClearValues = &clear_color;
	//vkCmdBeginRenderPass(ECRAN.imgui_commandBuffers[imageIndex], &RPinfo, VK_SUBPASS_CONTENTS_INLINE);
	//vkCmdBeginRendering(ECRAN.imgui_commandBuffers[imageIndex],&RenderingInfo);
	ImDrawData* draw_data = ImGui::GetDrawData();
	ImGui_ImplVulkan_RenderDrawData(draw_data, ecran.imgui_commandBuffers[iIndex],VK_NULL_HANDLE);
	//vkCmdEndRendering(ECRAN.imgui_commandBuffers[imageIndex]);

	/*ECRAN.SWAPCHAIN_MemoryBarrier(ECRAN.imgui_commandBuffers[imageIndex],
								ECRAN.swapChainImages[imageIndex],
								VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
								VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);*/

	//Submit command buffer
	//vkCmdEndRenderPass(ECRAN.imgui_commandBuffers[imageIndex]);

	err = vkEndCommandBuffer(ecran.imgui_commandBuffers[iIndex]);

	if(err != VK_SUCCESS)
	{
		
		if(err == VK_ERROR_OUT_OF_HOST_MEMORY)throw std::runtime_error("Marche pas le vkEndCommandBuffer de Imgui!");
	}
	#endif
}   
void sn_Vulkandraw(){        

	static uint32_t imageIndex = 0;
	//vkWaitForFences(device, 1, &ECRAN.inFlightFence, VK_TRUE, UINT64_MAX);
	err = vkAcquireNextImageKHR(device, 
								ecran.swapChain, 
								UINT64_MAX, 
								ecran.presentSema, 
								(VkFence)nullptr, 
								&imageIndex);
		if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR)) 
		{
			if (err == VK_ERROR_OUT_OF_DATE_KHR) {
			ecran.SWAPCHAIN_Resize();
			}
		return;
		}
		else if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
			
            throw std::runtime_error("failed to acquire swap chain image!");
        }
	//vkResetFences(device, 1, &ECRAN.inFlightFence);
	if(err == VK_ERROR_DEVICE_LOST)std::cout << "VK_ERROR_DEVICE_LOST" << std::endl;
	if(err == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)std::cout << "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT" << std::endl;
	if(err == VK_ERROR_OUT_OF_DATE_KHR)std::cout << "VK_ERROR_OUT_OF_DATE_KHR" << std::endl;
	if(err == VK_ERROR_OUT_OF_DEVICE_MEMORY)std::cout << "VK_ERROR_OUT_OF_DEVICE_MEMORY" << std::endl;
	if(err == VK_ERROR_OUT_OF_HOST_MEMORY)std::cout << "VK_ERROR_OUT_OF_HOST_MEMORY" << std::endl;
	if(err == VK_ERROR_SURFACE_LOST_KHR)std::cout << "VK_ERROR_SURFACE_LOST_KHR" << std::endl;
	if(err == VK_ERROR_UNKNOWN)std::cout << "VK_ERROR_UNKNOWN" << std::endl;
	
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
	sn_VulkandrawOverlay(imageIndex);
	VkCommandBuffer batch[2] = {ecran.commandBuffers[imageIndex],ecran.imgui_commandBuffers[imageIndex]};
	#else
	VkCommandBuffer batch[1] = {ECRAN.commandBuffers[imageIndex]};
    #endif


	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &ecran.presentSema;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &ecran.renderSema[imageIndex];

	submitInfo.pWaitDstStageMask = waitStages;
	#ifdef USE_IMGUI_PLEASE_IFYOUCAN
	submitInfo.commandBufferCount = 2;
	#else
	submitInfo.commandBufferCount = 1;
	#endif
	submitInfo.pCommandBuffers = batch;//&ECRAN.commandBuffers[imageIndex];
	
	sn_Updates(imageIndex);

	err = vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

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
		presentInfo.pWaitSemaphores = &ecran.renderSema[imageIndex];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &ecran.swapChain;
		presentInfo.pImageIndices = &imageIndex;

		err = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR) {
          
            sn_interface.resizing = false;
			ecran.SWAPCHAIN_Resize();
			if (err == VK_ERROR_OUT_OF_DATE_KHR) {
				return;
			}
		} else if (err != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
		vkQueueWaitIdle(presentQueue);
        ecran.currentFrame = (ecran.currentFrame + 1) % swap_imageCount;
			
}

