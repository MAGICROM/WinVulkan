void EcranOn();
void EcranOff();
/*int Platform_CreateVkSurface(ImGuiViewport *vp, ImU64 vk_inst, const void *vk_allocators, ImU64 *out_vk_surface)
{

}*/
struct snDevice
{
	VkDevice _device = VK_NULL_HANDLE;
	/** @brief Physical device representation */
	VkPhysicalDevice physicalDevice;
	/** @brief Logical device representation (application's view of the device) */
	VkDevice logicalDevice;
	/** @brief Properties of the physical device including limits that the application can check against */
	VkPhysicalDeviceProperties properties;
	/** @brief Features of the physical device that an application can use to check if a feature is supported */
	VkPhysicalDeviceFeatures features;
	/** @brief Features that have been enabled for use on the physical device */
	VkPhysicalDeviceFeatures enabledFeatures;
	/** @brief Memory types and heaps of the physical device */
	VkPhysicalDeviceMemoryProperties memoryProperties;
	/** @brief Queue family properties of the physical device */
	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	/** @brief List of extensions supported by the device */
	std::vector<std::string> supportedExtensions;
	/** @brief Default command pool for the graphics queue family index */
	VkCommandPool commandPool = VK_NULL_HANDLE;
	/** @brief Set to true when the debug marker extension is detected */
	bool enableDebugMarkers = false;
	/** @brief Contains queue family indices */
	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} queueFamilyIndices;
	explicit snDevice(VkPhysicalDevice physicalDevice);
	~snDevice();
	uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;
	VkResult createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, void *pNextChain, bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
	bool extensionSupported(std::string extension);
	VkCommandPool   createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	operator VkDevice()const {return _device;};
};

VkCommandPool snDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
	{
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
		cmdPoolInfo.flags = createFlags;
		VkCommandPool cmdPool;
		check_vk_result(vkCreateCommandPool(_device, &cmdPoolInfo, nullptr, &cmdPool));
		return cmdPool;
	}

bool snDevice::extensionSupported(std::string extension)
	{
		return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
	}
	
uint32_t snDevice::getQueueFamilyIndex(VkQueueFlags queueFlags) const
{
	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				return i;
			}
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				return i;
			}
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
	{
		if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
		{
			return i;
		}
	}

	throw std::runtime_error("Could not find a matching queue family index");
}

VkResult snDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char*> enabledExtensions, void* pNextChain, bool useSwapChain, VkQueueFlags requestedQueueTypes)
	{			
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Get queue family indices for the requested queue family types
		// Note that the indices may overlap depending on the implementation

		const float defaultQueuePriority(0.0f);

		// Graphics queue
		if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
			VkDeviceQueueCreateInfo queueInfo{};
			queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultQueuePriority;
			queueCreateInfos.push_back(queueInfo);
		}
		else
		{
			queueFamilyIndices.graphics = 0;
		}

		// Dedicated compute queue
		if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
		{
			queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
			if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
			{
				// If compute family index differs, we need an additional queue create info for the compute queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.compute = queueFamilyIndices.graphics;
		}

		// Dedicated transfer queue
		if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
		{
			queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
			if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
			{
				// If transfer family index differs, we need an additional queue create info for the transfer queue
				VkDeviceQueueCreateInfo queueInfo{};
				queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
				queueInfo.queueCount = 1;
				queueInfo.pQueuePriorities = &defaultQueuePriority;
				queueCreateInfos.push_back(queueInfo);
			}
		}
		else
		{
			// Else we use the same queue
			queueFamilyIndices.transfer = queueFamilyIndices.graphics;
		}

		// Create the logical device representation
		std::vector<const char*> deviceExtensions(enabledExtensions);
		if (useSwapChain)
		{
			// If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &enabledFeatures;
		
		// If a pNext(Chain) has been passed, we need to add it to the device creation info
		VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
		if (pNextChain) {
			physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
			physicalDeviceFeatures2.features = enabledFeatures;
			physicalDeviceFeatures2.pNext = pNextChain;
			deviceCreateInfo.pEnabledFeatures = nullptr;
			deviceCreateInfo.pNext = &physicalDeviceFeatures2;
		}

		// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
		if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		{
			deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			enableDebugMarkers = true;
		}

		if (deviceExtensions.size() > 0)
		{
			for (const char* enabledExtension : deviceExtensions)
			{
				if (!extensionSupported(enabledExtension)) {
					std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
				}
			}

			deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}

		this->enabledFeatures = enabledFeatures;

		VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &_device);
		if (result != VK_SUCCESS) 
		{
			return result;
		}

		// Create a default command pool for graphics command buffers
		commandPool = createCommandPool(queueFamilyIndices.graphics);

		return result;
	}
	
snDevice::snDevice(VkPhysicalDevice physicalDevice)
{
	assert(physicalDevice);
	this->physicalDevice = physicalDevice;

	// Store Properties features, limits and properties of the physical device for later use
	// Device properties also contain limits and sparse properties
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);
	// Features should be checked by the examples before using them
	vkGetPhysicalDeviceFeatures(physicalDevice, &features);
	// Memory properties are used regularly for creating all kinds of buffers
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
	// Queue family properties, used for setting up requested queues upon device creation
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// Get list of supported extensions
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		{
			for (auto ext : extensions)
			{
				supportedExtensions.push_back(ext.extensionName);
			}
		}
	}
}

snDevice::~snDevice()
	{
		if (commandPool)
		{
			vkDestroyCommandPool(_device, commandPool, nullptr);
		}
		if (_device)
		{
			vkDestroyDevice(_device, nullptr);
		}
	}


snDevice* pdevice = nullptr;
void getEnabledFeatures() {}
void getEnabledExtensions() {}
void sn_Wulkaninit(HINSTANCE hinstance,HWND hwnd)
{
    std::vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	// Get extensions supported by the instance and store for later use
	uint32_t extCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		std::vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		{
			for (VkExtensionProperties extension : extensions)
			{
				supportedInstanceExtensions.push_back(extension.extensionName);
			}
		}
	}

	// Enabled requested instance extensions
	if (enabledInstanceExtensions.size() > 0) 
	{
		for (const char * enabledExtension : enabledInstanceExtensions) 
		{
			// Output message if requested extension is not available
			if (std::find(supportedInstanceExtensions.begin(), supportedInstanceExtensions.end(), enabledExtension) == supportedInstanceExtensions.end())
			{
				std::cerr << "Enabled instance extension \"" << enabledExtension << "\" is not present at instance level\n";
			}
			instanceExtensions.push_back(enabledExtension);
		}
	}

	uint32_t vkversion;
	err = vkEnumerateInstanceVersion(&vkversion);

	uint32_t vkversion_variant = VK_API_VERSION_VARIANT(vkversion);
	uint32_t vkversion_major = VK_VERSION_MAJOR(vkversion);
	uint32_t vkversion_minor = VK_API_VERSION_MINOR(vkversion);
	uint32_t vkversion_patch = VK_VERSION_PATCH(vkversion);
	
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
											
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.pApplicationInfo = &appInfo;

	if (instanceExtensions.size() > 0)
	{
		if (KHRONOS_validation)
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);	// SRS - Dependency when VK_EXT_DEBUG_MARKER is enabled
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

	// The VK_LAYER_KHRONOS_validation contains all current validation functionality.
	// Note that on Android this layer requires at least NDK r20
	const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
	if (KHRONOS_validation)
	{
		// Check if this layer is available at instance level
		uint32_t instanceLayerCount;
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
		vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
		bool validationLayerPresent = false;
		for (VkLayerProperties layer : instanceLayerProperties) {
			if (strcmp(layer.layerName, validationLayerName) == 0) {
				validationLayerPresent = true;
				break;
			}
		}
		if (validationLayerPresent) {
			instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
			instanceCreateInfo.enabledLayerCount = 1;
		} else {
			std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
		}
	}

	err = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	if (err != VK_SUCCESS) {
		std::cerr << "Failed to create Vulkan instance!" << std::endl;
		return;
	}
	std::cout << "VULKAN INSTANCE OK" << std::endl;

	// Physical device
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	
	check_vk_result(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
	if (gpuCount == 0) {
		std::cerr << "No device with Vulkan support found" << std::endl;
		return;
	}
	// Enumerate devices
	std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
	err = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
	if (err) {
		std::cerr << "Could not enumerate physical devices" << std::endl;
		return;
	}

	// GPU selection

	// Select physical device to be used for the Vulkan example
	// Defaults to the first device unless specified by command line
	uint32_t selectedDevice = 0;

	carte_graphique = physicalDevices[selectedDevice];

	// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(carte_graphique, &deviceProperties);
	vkGetPhysicalDeviceFeatures(carte_graphique, &deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(carte_graphique, &deviceMemoryProperties);

	// Derived examples can override this to set actual features (based on above readings) to enable for logical device creation
	getEnabledFeatures();

	// Vulkan device creation
	// This is handled by a separate class that gets a logical device representation
	// and encapsulates functions related to a device
	pdevice = new snDevice(carte_graphique);

	// Derived examples can enable extensions based on the list of supported extensions read from the physical device
	getEnabledExtensions();

	err = pdevice->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
	if (err != VK_SUCCESS) {
		std::cerr << "Could not create Vulkan device\n";
		return;
	}
	device = pdevice->_device;

	/*std::vector<const char*> extensions_present_names;
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
    uint32_t deviceCount{0};
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	
	
	std::vector<uint32_t> transfert_family;
	
	//Teste tous les gpus
	uint32_t queueFamilyCount{0};
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
					transfert_family.push_back(n_family);
				}
			}
	}
	
	transferFamily = transfert_family[0];

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
            createInfo.enabledExtensionCount = static_cast<uint32_t>(extNames.size()); //extNames
            createInfo.ppEnabledExtensionNames = extNames.data();
            //createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
            //createInfo.ppEnabledLayerNames = layers.data();
            
            err = vkCreateDevice(carte_graphique, &createInfo, nullptr, &device);
        if(err != VK_SUCCESS){
            std::runtime_error("failed to create logical device!");
        }*/
        std::cout << "VULKAN GPU OK" << std::endl;
		
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = hinstance;
		surfaceCreateInfo.hwnd = hwnd;
		err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
	
		VkDebugUtilsMessengerCreateInfoEXT dbg_messenger_create_info;
		
		vkGetDeviceQueue(device, graphicsFamily, 0, &graphicsQueue);
   		vkGetDeviceQueue(device, presentFamily, 0, &presentQueue);
		vkGetDeviceQueue(device, transferFamily, 0, &transferQueue);
		
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

		swap_imageCount = capabilities.minImageCount + 1;

		#ifdef USE_IMGUI_PLEASE_IFYOUCAN
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
		//platform_io.Platform_CreateVkSurface = Platform_CreateVkSurface;


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
}
void sn_Vulkandestroy()
{
	vkDeviceWaitIdle(device);

	#ifdef USE_IMGUI_PLEASE_IFYOUCAN           
	   	ImGui_ImplVulkan_Shutdown();
		vkDestroyRenderPass(device, imgui_renderPass, nullptr);
		vkDestroyDescriptorPool(device,imgui_pDescriptorPool,NULL);
	#endif

    EcranOff();
	
	//Close Device and all
	delete pdevice;
    vkDestroySurfaceKHR(instance,surface,NULL);
    vkDestroyInstance(instance, NULL);
}
