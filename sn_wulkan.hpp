void EcranOn();
void EcranOff();
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}
void sn_Wulkaninit(HINSTANCE hinstance,HWND hwnd)
{
    std::vector<const char*> extensions_present_names;
    std::vector<const char*> layers;
    std::vector<const char*> extension_names;

    VkLayerProperties *instance_layers{NULL};
    VkExtensionProperties *instance_extensions{};
	const char **required_extensions{NULL};
	
	uint32_t queueFamilyCount{0};
    uint32_t deviceCount{0};
    uint32_t instance_layer_count{0};
    uint32_t required_extension_count{0};
 
	uint32_t vkversion;
	err = vkEnumerateInstanceVersion(&vkversion);

	uint32_t vkversion_variant = VK_API_VERSION_VARIANT(vkversion);
	uint32_t vkversion_major = VK_VERSION_MAJOR(vkversion);
	uint32_t vkversion_minor = VK_API_VERSION_MINOR(vkversion);
	uint32_t vkversion_patch = VK_VERSION_PATCH(vkversion);

	auto t_start = std::chrono::high_resolution_clock::now();
    auto t_now = std::chrono::high_resolution_clock::now();
        
    const char *instance_validation_layers_alt1[] = {"VK_LAYER_LUNARG_standard_validation"};
    vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
    
	const char *instance_needed_layers[] = {"VK_LAYER_LUNARG_standard_validation"};

	char **instance_validation_layers = (char**) instance_validation_layers_alt1;
        if (instance_layer_count > 0) {
            instance_layers = (VkLayerProperties*)malloc(sizeof (VkLayerProperties) * instance_layer_count);
            err = vkEnumerateInstanceLayerProperties(&instance_layer_count,instance_layers);
            for(uint32_t i=0;i<instance_layer_count;i++)
            {
                layers.push_back(instance_layers[i].layerName);
            }
        }
		
	uint32_t instance_extension_count{0};
	vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, NULL);
	std::vector<VkExtensionProperties> ExtensionProperties(instance_extension_count);
	vkEnumerateInstanceExtensionProperties(NULL, &instance_extension_count, ExtensionProperties.data());
            
	for(uint32_t i=0;i<instance_extension_count;i++)
            {
				std::cout << "Extension: " << ExtensionProperties[i].extensionName << std::endl;
			}
	
    for(uint32_t i=0;i<instance_layer_count;i++)
        {
			
			if (required_extension_count > 0) {
            instance_extensions = (VkExtensionProperties*)malloc(sizeof (VkExtensionProperties) * instance_extension_count);
            err = vkEnumerateInstanceExtensionProperties(layers[i], &instance_extension_count, NULL);
            for(uint32_t i=0;i<instance_extension_count;i++)
            {
                extensions_present_names.push_back(instance_extensions[i].extensionName);
				std::cout << "Extension: " << instance_extensions[i].extensionName << std::endl;
			}
			free(instance_extensions);
        	}
		}
	
	//extensions utilisés
	extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	//extension_names.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "WinVulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Spacenet";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_API_VERSION(vkversion_variant,
											vkversion_major,
											vkversion_minor,
											vkversion_patch);

	VkInstanceCreateInfo instancecreateInfo = {};
	instancecreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instancecreateInfo.pApplicationInfo = &appInfo;
	instancecreateInfo.enabledExtensionCount = static_cast<uint32_t>(extension_names.size());
    instancecreateInfo.ppEnabledExtensionNames = extension_names.data();
	//fossilize
	//instancecreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	//no fossilize
	instancecreateInfo.enabledLayerCount = 0;
	instancecreateInfo.ppEnabledLayerNames = layers.data();
	
	err = vkCreateInstance(&instancecreateInfo, nullptr, &instance);
	if (err != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance!" << std::endl;
		return;
	}
	std::cout << "VULKAN INSTANCE OK" << std::endl;

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hinstance;
	surfaceCreateInfo.hwnd = hwnd;
	err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
	
	//Combien de Gpus ?      
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	
	//Teste tous les gpus
	for (const VkPhysicalDevice& tested_device : devices) 
	{
		vkGetPhysicalDeviceQueueFamilyProperties(tested_device, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(tested_device, &queueFamilyCount, queueFamilies.data());
	
		//Teste toutes les familles de queue
		for (uint32_t n_family = 0;n_family<queueFamilies.size();n_family++) {
				
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(tested_device, n_family, surface, &presentSupport);
				
				if (queueFamilies[n_family].queueFlags & VK_QUEUE_GRAPHICS_BIT) 
				{
				graphicsFamily = n_family;
				if (presentSupport) {
					//Ce GPU est Bon pour cette surface
					carte_graphique = tested_device;
					presentFamily = n_family;
					}
				}
				if (queueFamilies[n_family].queueFlags & VK_QUEUE_TRANSFER_BIT &&
				!(queueFamilies[n_family].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
				!(queueFamilies[n_family].queueFlags & VK_QUEUE_COMPUTE_BIT)) 
				{
					transferFamily = n_family;
				}
			}
	}
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {graphicsFamily, presentFamily, transferFamily};

	float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

        // get extension names
        uint32_t _extensionCount{0};
		uint32_t _layer_count{0};

		vkEnumerateDeviceLayerProperties(carte_graphique , &_layer_count, NULL);
 		std::vector<VkLayerProperties> layerProps(_layer_count);
		vkEnumerateDeviceLayerProperties(carte_graphique , &_layer_count, layerProps.data());
		
		for (const VkLayerProperties& tested_layer : layerProps) {
			std::cout << tested_layer.layerName << std::endl;
			std::cout << tested_layer.description << std::endl;
		}

        vkEnumerateDeviceExtensionProperties( carte_graphique, NULL, &_extensionCount, NULL);
        std::vector<const char *> extNames;
        std::vector<VkExtensionProperties> extProps(_extensionCount);
        vkEnumerateDeviceExtensionProperties( carte_graphique, NULL, &_extensionCount, extProps.data());
        for (uint32_t i = 0; i < _extensionCount; i++) {
            extNames.push_back(extProps[i].extensionName);
			std::cout << "Extension: " << extProps[i].extensionName << std::endl;
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.pEnabledFeatures = &deviceFeatures;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size());
            createInfo.ppEnabledExtensionNames = extNames.data();
            //createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
            //createInfo.ppEnabledLayerNames = layers.data();
            
            err = vkCreateDevice(carte_graphique, &createInfo, nullptr, &device);
        if(err != VK_SUCCESS){
            std::runtime_error("failed to create logical device!");
        }
        std::cout << "VULKAN GPU OK" << std::endl;
		VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
		
		vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
   		vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
		vkGetDeviceQueue(device, transferFamily, 0, &transferQueue);
		
        
        VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = graphicsFamily;

			if (vkCreateCommandPool(device, &poolInfo, nullptr, &transfercommandPool) != VK_SUCCESS) {
				throw std::runtime_error("failed to create transfert command pool!");
			}


		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(carte_graphique, surface, &capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(carte_graphique, surface, &formatCount, nullptr);
        if (formatCount != 0) {
                formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(carte_graphique, surface, &formatCount, formats.data());
            }
        uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(carte_graphique, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
                presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(carte_graphique, surface, &presentModeCount, presentModes.data());
            }

		VkDescriptorPoolSize descriptorPoolSizes[] =
		{	{ VK_DESCRIPTOR_TYPE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 }
		};

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
			descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCI.maxSets = 10 * (sizeof(descriptorPoolSizes)/sizeof(*descriptorPoolSizes));
			descriptorPoolCI.poolSizeCount = (uint32_t)(sizeof(descriptorPoolSizes)/sizeof(*descriptorPoolSizes));
			descriptorPoolCI.pPoolSizes = descriptorPoolSizes;
	
		err = vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorPool);
		swap_imageCount = capabilities.minImageCount + 1;

		#ifdef USE_IMGUI_PLEASE_IFYOUCAN
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        ImGui::StyleColorsDark();
        //ImGui_ImplGlfw_InitForVulkan(window, true);
		// Setup scaling
    	// Make process DPI aware and obtain main monitor scale
   		ImGui_ImplWin32_EnableDpiAwareness();
    	float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
	
		ImGuiStyle& style = ImGui::GetStyle();
    	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    	style.FontScaleDpi = main_scale;    

        // Create Descriptor Pool
    	// If you wish to load e.g. additional textures you may need to alter pools sizes and maxSets.
		{
			static VkAllocationCallbacks*   g_Allocator = nullptr;
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE },
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 0;
			for (VkDescriptorPoolSize& pool_size : pool_sizes)
				pool_info.maxSets += pool_size.descriptorCount;
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			err = vkCreateDescriptorPool(device, &pool_info, g_Allocator, &imgui_pDescriptorPool);
			check_vk_result(err);
		}
		
		if (capabilities.maxImageCount > 0 && swap_imageCount > capabilities.maxImageCount) {
				swap_imageCount = capabilities.maxImageCount;
			}

							
			VkAttachmentDescription Attachments{};

            Attachments.format = VK_FORMAT_B8G8R8A8_UNORM;
            Attachments.samples = VK_SAMPLE_COUNT_1_BIT;
            Attachments.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
            Attachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            Attachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            Attachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            Attachments.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            Attachments.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference color_Ref{};
            color_Ref.attachment = 0;
            color_Ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			
			VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &color_Ref;
			
			VkRenderPassCreateInfo renderPassInfo_imgui{};
            renderPassInfo_imgui.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo_imgui.attachmentCount = 1;
            renderPassInfo_imgui.pAttachments = &Attachments;
            renderPassInfo_imgui.subpassCount = 1;
            renderPassInfo_imgui.pSubpasses = &subpass;
            renderPassInfo_imgui.dependencyCount = 0;

            err = vkCreateRenderPass(device, &renderPassInfo_imgui, nullptr, &imgui_renderPass);
            if( err!= VK_SUCCESS){
                    throw std::runtime_error("failed to create render pass!!");
                }
				
			ImGui_ImplVulkan_InitInfo dearimgui{};
            dearimgui.Instance = instance;
            dearimgui.Device = device;
            dearimgui.PhysicalDevice = carte_graphique;  
            dearimgui.ImageCount = swap_imageCount;
            dearimgui.MinImageCount = swap_imageCount;
            dearimgui.Queue = graphicsQueue;
            dearimgui.QueueFamily = graphicsFamily;
            dearimgui.DescriptorPool = imgui_pDescriptorPool;
			dearimgui.PipelineInfoMain.RenderPass = imgui_renderPass;
			dearimgui.PipelineInfoMain.Subpass = 0;
			dearimgui.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
			dearimgui.CheckVkResultFn = check_vk_result;
        
        
        ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplVulkan_Init(&dearimgui);
		#endif
		
		
		EcranOn();
		rebuild(false);
}
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
void sn_Vulkandestroy()
{
	vkDeviceWaitIdle(device);

	#ifdef USE_IMGUI_PLEASE_IFYOUCAN           
	   	ImGui_ImplVulkan_Shutdown();
		vkFreeCommandBuffers(device,imgui_CommandPool,1,&imgui_CommandBuffer);
		vkDestroyCommandPool(device, imgui_CommandPool, nullptr);
		vkDestroyRenderPass(device, imgui_renderPass, nullptr);
	#endif

    EcranOff();
	vkDestroyDescriptorPool(device,descriptorPool,NULL);  
	vkDestroyDevice(device,NULL);
    vkDestroySurfaceKHR(instance,surface,NULL);
    vkDestroyInstance(instance, NULL);
}
