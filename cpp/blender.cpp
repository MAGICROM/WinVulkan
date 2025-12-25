#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>

///////////////////////////////////////////////////////////////////////////////////////
static std::vector<char> readFile(const std::string& filename) {
         std::ifstream file(filename, std::ios::ate | std::ios::binary);
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
        }
///////////////////////////////////////////////////////////////////////////////////////       
static std::vector<char> readValue(char** t,int x){
        std::vector<char> read;
        for(int i=0;i<x;i++)
            {
                read.push_back((*t)[i]);
            }
        (*t)+=x;
        return read;
        }
///////////////////////////////////////////////////////////////////////////////////////
bool ptr64 = false;
bool BigEndian = false;
///////////////////////////////////////////////////////////////////////////////////////
bool stringcompare(const char *a, char *b)
{
    int len_him = 0;
    int len_other = 0;
    const char *c = a;
    while(*(c++))len_him++;
    c = b;
    while(*(c++))len_other++;
    if(len_him != len_other)return false;
    while(*(a))if(*(a++)!=*(b++))return false;
    return true;
}

struct str_blend_block{
            std::vector<char> type;
            char* adress;
            int len;
            char* old_memory_address;
            int sdna_index;
            int nstruct;
        };
///////////////////////////////////////////////////////////////////////////////////////
struct str_blend_type {
            bool isstruct;
            int nstruct;
            char* literal;
            std::vector<char> type;
            short size;
            bool Is(char* him)
            {
                return stringcompare(him, literal);
            }
        }; 
///////////////////////////////////////////////////////////////////////////////////////       
struct str_blend_name{
            char* literal;
            std::vector<char> name;
        };
///////////////////////////////////////////////////////////////////////////////////////
struct str_blend_field{
    bool is_ptr;
    int is_array;
    
    short fields_type;
    char *literal_type;
    short fields_name;
    char *literal_name;
    
    int data_offset;
    char *data;
        };
///////////////////////////////////////////////////////////////////////////////////////
struct str_blend_struct{
            int size;
            int instanced;
            short type_str;
            char* literal; //short look wrong in Debug but type is right
            std::vector<str_blend_field> fields;
            void Map(char* Map, std::vector<str_blend_block> &blocks)
            {
                for (auto &field : fields)
                {
                    
                    field.data = Map + field.data_offset;
                    
                    if(field.is_ptr)        
                    {                        
                        char *old_adress = nullptr;
                        if(ptr64)
                            old_adress = (char*)*((unsigned long long*)field.data);
                        else
                            old_adress = (char*)*((unsigned long long*)field.data);

                        if(old_adress)
                        {
                        for (auto &testblock : blocks) // access by reference to avoid copying
                                {  
                                if(old_adress==testblock.old_memory_address)
                                    {
                                   
                                    field.data = testblock.adress;
                                    }
                                if(old_adress>testblock.old_memory_address &&
                                old_adress<(testblock.old_memory_address+testblock.len))
                                    {
                                   
                                    int offset = 0;
                                    if(ptr64)
                                        offset = (int)(long long)testblock.old_memory_address-(long long)old_adress;
                                    else
                                        offset = (int)(long long)testblock.old_memory_address-(int)(long long)old_adress;
                                    field.data = testblock.adress+offset;
                                    }
                                }

                        }
                        if(!old_adress)
                            {
                               
                                field.data = nullptr;
                            }
                    }
                    else
                    {
                        field.data = Map + field.data_offset;
                       
                    }
        
            }
            
            }
            bool Is(const char* him)
            {
                return stringcompare(him,literal);             
            }
            void* Value(const char* him,int ni,std::vector<str_blend_type> &types,std::vector<str_blend_block> &blocks,std::vector<str_blend_struct> &structs,int *nstruct)
            {
                for (int i=0;i<fields.size();i++)
                {
                    if(stringcompare(him,fields[i].literal_name))
                        {
                            if(types[fields[i].fields_type].isstruct)
                            {
                                *nstruct = types[fields[i].fields_type].nstruct;
                                int offset = ni*structs[*nstruct].size;
                                structs[*nstruct].Map(fields[i].data+offset,blocks);
                                return fields[i].data+offset;
                            }
                            int offset_array = 1;
                            if(fields[i].is_array)offset_array*=fields[i].is_array;
                            int offset = ni*offset_array*types[fields[i].fields_type].size;
                            return fields[i].data+offset;
                        }
                }
                return nullptr;
            }
        };
///////////////////////////////////////////////////////////////////////////////////////
#define READ_16(x) *((short*)x);x+=2
#define READ_32(x) *((int*)x);x+=4
#define READ_64(x) *((long long*)x);x+=8
///////////////////////////////////////////////////////////////////////////////////////

int readblend(const char* pathname){
    std::vector<str_blend_block> blocks;
    std::vector<str_blend_name> names;
    std::vector<str_blend_type> types;
    std::vector<str_blend_struct> structs;
    std::vector<char> gp_ex4;

    int len;
    long long ptr;
    int sdna_index;
    int nstruct;
    int size;

    gp_ex4 = readFile(pathname);
    char * file = gp_ex4.data();

            std::vector<char> read = readValue(&file,7);
            read = readValue(&file,1);
            if(read[0] == '-')ptr64=true;
            read = readValue(&file,1);
            if(read[0] == 'V')BigEndian=true;
            read = readValue(&file,3);
    bool cte = true;
        while(cte){
                    read = readValue(&file,4);

                    if(*((int*)read.data())==*((int*)"ENDB"))
                    cte=false;

                    len = READ_32(file);

                    if(ptr64)
                        {
                        ptr = READ_64(file);
                        }
                    else 
                        {
                        ptr = READ_32(file);
                        }
                    sdna_index = READ_32(file);
                    nstruct = READ_32(file);

                    str_blend_block tb;
                        tb.type = read;
                        tb.adress = file;
                        tb.len = len;
                        tb.old_memory_address = (char*)ptr;
                        tb.sdna_index = sdna_index;
                        tb.nstruct = nstruct;
                
                    blocks.push_back(tb);
                    file+=len;
    }

    size = blocks.size();
    char* dna = 0;

            str_blend_block dna1;
        for(int i=0;i<size;i++)
        {
            if(*((int*)blocks[i].type.data())==*((int*)"DNA1"))
            {
                dna = blocks[i].adress;
                dna1 = blocks[i];
                break;
            }
        }
                   READ_32(dna);
                   READ_32(dna);
            size = READ_32(dna);

        for(int i=0;i<size;i++)
                {
					str_blend_name name;
                    name.literal = dna;
                    while(*dna)name.name.push_back(*(dna++));
                    dna++;
                    names.push_back(name);
                }

    while(*( (int*)dna ) != *((int*)"TYPE") )dna++;
                   READ_32(dna);
            size = READ_32(dna);

        for(int i=0;i<size;i++)
                {
			str_blend_type type;
                    type.literal = dna;
                    while(*dna)type.type.push_back(*(dna++));
                    dna++;

                    types.push_back(type);
                }
        
    while(*( (int*)dna ) != *((int*)"TLEN") )dna++;
                   READ_32(dna);        

        for(int i=0;i<size;i++)
                {
                    types[i].size = READ_16(dna);
                }
    
    while(*( (int*)dna ) != *((int*)"STRC") )dna++;
                   READ_32(dna);     
            size = READ_32(dna);   
 
        for(int i=0;i<size;i++)
        {
            int struct_size = 0;
            str_blend_struct str;
            str.type_str = READ_16(dna);
            str.literal = types[str.type_str].literal;

            short howmany = READ_16(dna);
            for(short j=0;j<howmany;j++)
            {
                std::vector<char*> array;
                str_blend_field field;
                
                field.fields_type = READ_16(dna);
                field.literal_type = types[field.fields_type].literal;
                field.fields_name = READ_16(dna);
                field.literal_name = names[field.fields_name].literal;
                field.data_offset = struct_size;
                field.is_ptr = false;
                field.is_array = 0;

                    if(names[field.fields_name].literal[0] == '*')
                    {
                        field.is_ptr = true;
                        if(ptr64)
                            struct_size+= 8;
                        else
                            struct_size+= 4;
                    }   
                     else
                    {
                        array.clear();
                        int number = 1;
                        char* testarray = names[field.fields_name].literal;
                        while(*testarray)
                        {
                            if(*testarray == '['){
                                array.push_back(testarray+1);
                                field.is_array++;
                                }
                            testarray++;
                        }
                        for (auto &dimension : array)
                        {
                            int thisdim = 0;
                            while(*dimension!=']')
                            {
                                thisdim *= 10; 
                                thisdim += (*dimension - 48);
                                dimension++;
                            }
                            number*=thisdim;
                            field.is_array = number;
                        }
                        struct_size+= types[field.fields_type].size * number;
                    }

                field.data = nullptr;
                
                str.fields.push_back(field);
            }
            str.instanced = 0;
            str.size = struct_size;
            structs.push_back(str);
        }

    int i=0;

    for(auto &type : types)
    {
        int i=0;
        type.isstruct = false;
        for(auto &str : structs)
        {
            if(str.Is(type.literal))
            {
                type.isstruct = true;
                type.nstruct = i;

            }
            i++;
        }

    }

    for (auto &block : blocks) // access by reference to avoid copying
    {     
       std::cout << structs[block.sdna_index].literal << " : " << block.len << " ************************************************************* ";
       std::cout << std::endl;
       structs[block.sdna_index].instanced++;
       structs[block.sdna_index].Map(block.adress,blocks);    
       
            std::cout << "STRUCT " << structs[block.sdna_index].size <<" BLOCKLEN " << block.len << std::endl;
            if(structs[block.sdna_index].size * block.nstruct == block.len)
            {
                std::cout << "MATCH!!!!!" << std::endl;
                if(structs[block.sdna_index].Is("Mesh"))
                {
                        std::cout << "< Mesh >" << std::endl;
                        for(auto &field : structs[block.sdna_index].fields)
                        {
                            std::cout << field.literal_type << " " << field.literal_name << std::endl;
                            
                            if(types[field.fields_type].isstruct)
                            {
                                if(!field.is_ptr)
                                structs[types[field.fields_type].nstruct].Map(field.data,blocks);
                            }

                            void *datatest = field.data;
                            datatest = field.data;
                        } 
                        
                        
                        str_blend_field fd;
                        int nstruct;
                        int vert = *((int*)structs[block.sdna_index].Value("totvert",0,types,blocks,structs,&nstruct));
                        int edge = *((int*)structs[block.sdna_index].Value("totedge",0,types,blocks,structs,&nstruct));
                        int poly = *((int*)structs[block.sdna_index].Value("totpoly",0,types,blocks,structs,&nstruct));
                        int loop = *((int*)structs[block.sdna_index].Value("totloop",0,types,blocks,structs,&nstruct));
                        //float x[vert],y[vert],z[vert]; // il sait faire ça sans new GCC!!!! 
                        float *x = new float[vert];
                        float *y = new float[vert];
                        float *z = new float[vert];
                        
                        
                        
                        /*for(int i=0;i<vert;i++)
                        {
        
                            float * co = (float*)structs[block.sdna_index].Value("*mvert",i,types,blocks,structs,&nstruct);
                            x[i] = co[0];
                            y[i] = co[1];
                            z[i] = co[2];
                        }*/
                       
                        std::cout << "MESH!!!!!";
                        delete [] x;
                        delete [] y;
                        delete [] z;
                }
      
            }
            if(structs[block.sdna_index].size * block.nstruct > block.len)
            {
                std::cout << "GRAVE!!!!!";
            }
            if(structs[block.sdna_index].size * block.nstruct < block.len)
            {
                std::cout << "CELA PASSE!!!!!";
            }
       
    }
    if(*( (int*)dna ) == *((int*)"ENDB") )
    {
        blocks.clear();
        names.clear();
        types.clear();
        structs.clear();
        gp_ex4.clear();
        return 0;
    }

    blocks.clear();
    names.clear();
    types.clear();
    structs.clear();
    gp_ex4.clear();
    return 1;
}