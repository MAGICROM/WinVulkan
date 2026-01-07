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
void CommandImgMemBarrier(VkCommandBuffer& buf,VkImage img,VkImageLayout de,VkImageLayout a,VkImageSubresourceRange* pS = nullptr)
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
		if(pS)
			memory_barrier.subresourceRange = *pS;
		else	
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
			memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			Source = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
			Destination = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
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
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normale;
	glm::vec2 uv;

	static VkPipelineVertexInputStateCreateInfo VertexInputState()
	{
		static VkPipelineVertexInputStateCreateInfo info;
		
		static VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		static VkVertexInputAttributeDescription attributeDescriptions[3];
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

		info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		info.vertexBindingDescriptionCount = 1;
		info.pVertexBindingDescriptions = &bindingDescription;
		info.vertexAttributeDescriptionCount = 3;
		info.pVertexAttributeDescriptions = attributeDescriptions;
		return info;
	}
};
uint32_t Read_Obj(const char* filename, void** ppVertex,void** ppIndex)
{
	std::vector<char> object = readFile(filename);
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
extern unsigned char* STBI_ReadImage(const char* filename,int* w, int* h, int* c);
extern void STBI_FreeImage(unsigned char* img);