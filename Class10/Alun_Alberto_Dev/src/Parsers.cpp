#include "Parsers.h"
#include <fstream>
#include "extern.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

#include <unordered_map>
std::unordered_map<std::string, int> Parsers::geometries;
std::unordered_map<std::string, int> Parsers::textures;
std::unordered_map<std::string, int> Parsers::materials;
std::unordered_map<std::string, int> Parsers::shaders;

void split(std::string to_split, std::string delim, std::vector<std::string>& result) {

	size_t last_offset = 0;
	while (true) {
		//find first delim
		size_t offset = to_split.find_first_of(delim, last_offset); 
		result.push_back(to_split.substr(last_offset, offset - last_offset));
		if (offset == std::string::npos) // if at end of string
			break;
		else //otherwise continue
			last_offset = offset + 1;
	}
}

//parses a wavefront object into passed arrays
bool Parsers::parseOBJ(std::string filename, std::vector<float>& vertices, std::vector<float>& uvs, std::vector<float>& normals, std::vector<unsigned int>& indices) {
    
	std::string line;
	std::ifstream file(filename);
	if (file.is_open())
	{
		//declare containers for temporary and final attributes
		std::vector<lm::vec3> temp_vertices;
		std::vector<lm::vec2> temp_uvs;
		std::vector<lm::vec3> temp_normals;

		//container to store map for indices
		std::unordered_map<std::string, int> indices_map;
		int next_index = 0; //stores next available index

		//parse file line by line
		while (std::getline(file, line))
		{
			//split line string
			std::vector<std::string> words; 
			split(line, " ", words);
            for (auto it = words.begin(); it != words.end();)
            {
                if ((*it) == "") it = words.erase(it);
                else it++;
            }

			if (words.empty()) continue; //empty line, skip
			if (words[0][0] == '#')	continue; //first word starts with #, line is a comment

			if (words[0] == "v") { //line contains vertex data
				//read words to floats
				lm::vec3 pos( (float)atof( words[1].c_str() ), 
							  (float)atof( words[2].c_str() ),
							  (float)atof( words[3].c_str() ) );
				//add to temporary vector of positions
				temp_vertices.push_back(pos);
			}
			if (words[0] == "vt") { //line contains texture data
				//read words to floats
				lm::vec2 tex( (float)atof(words[1].c_str() ),
							  (float)atof(words[2].c_str() ) );
				//add to temporary vector of texture coords
				temp_uvs.push_back(tex);
			}
			if (words[0] == "vn") { //line contains vertex data
				//read words to floats
				lm::vec3 norm( (float)atof(words[1].c_str() ),
							   (float)atof(words[2].c_str() ),
							   (float)atof(words[3].c_str() ) );
				//add to temporary vector of normals
				temp_normals.push_back(norm);
			}

			//line contains face-vertex data
			if (words[0] == "f") {
				if (words.size() < 4) continue; // faces with fewer than 3 vertices??!

				bool quad_face = false; //boolean to help us deal with quad faces

				std::vector<std::string> nums; // container used for split indices
				//for each face vertex
				for (int i = 1; i < words.size(); i++) {

					//check if face vertex has already been indexed
					if (indices_map.count(words[i]) == 0) {
					    
                        if (words[i] == "") continue;
						//if not, start by getting all indices
						nums.clear();
						split(words[i], "/", nums);
						int v_ind = atoi(nums[0].c_str()) - 1; //subtract 1 to convert to 0-based array!
						int t_ind = atoi(nums[1].c_str()) - 1;
						int n_ind = atoi(nums[2].c_str()) - 1;

						//add vertices to final arrays of floats
						for (int j = 0; j < 3; j++)
							vertices.push_back(temp_vertices[v_ind].value_[j]);
						for (int j = 0; j < 2; j++)
							uvs.push_back(temp_uvs[t_ind].value_[j]);
						for (int j = 0; j < 3; j++)
							normals.push_back(temp_normals[n_ind].value_[j]);
						
						//add an index to final array
						indices.push_back(next_index);

						//record that this index is used for this face vertex
						indices_map[words[i]] = next_index;

						//increment index
						next_index++;
					}
					else {
						//face vertex was already added to final arrays
						//so search for its stored index
						int the_index = indices_map.at(words[i]); //safe to use as we know it exists
						//add it to final index array
						indices.push_back(the_index);
					}

					//***CHECK FOR QUAD FACES****
					//If the face is a quads (i.e. words.size() == 5), we need to create two triangles
					//out of the quad. We have already created one triangle with words[1], [2] and [3]
					//now we make another with [4], [1] and [3]. 
					if (i == 4 ) { 
						//face-vertex 4 is already added, so we search for indices of 1 and 3 and add them
						int index_1 = indices_map.at(words[1]);
						indices.push_back(index_1);
						int index_3 = indices_map.at(words[3]);
						indices.push_back(index_3);
					}

				}

			}

		}
		file.close();
		return true;
	}
	return false;
}

bool Parsers::parseBin(std::string filename, std::vector<float>& vertices, std::vector<float>& uvs, std::vector<float>& normals, std::vector<unsigned int>& indices)
{
    FILE* f = nullptr;
    f = fopen(filename.c_str(), "rb");

    //declare containers for temporary and final attributes
    THeader header;
    std::vector<float> temp_vtxs;

    //parse file line by line
    bool eof_found = false;

    while (!eof_found) {

        //split line string
        TChunk chunk;
        auto bytes_read = fread(&chunk, 1, sizeof(chunk), f);
        assert(bytes_read == sizeof(chunk));

        switch (chunk.magic_id) {

        case magicHeader:

            bytes_read = fread(&header, 1, chunk.num_bytes, f);
            assert(bytes_read == chunk.num_bytes);
            break;

        case magicVtxs:

            temp_vtxs.resize(chunk.num_bytes);
            bytes_read = fread(temp_vtxs.data(), 1, chunk.num_bytes, f);
            assert(bytes_read == chunk.num_bytes);
            break;

        case magicIdxs:

            indices.resize(chunk.num_bytes);
            bytes_read = fread(indices.data(), 1, chunk.num_bytes, f);
            assert(bytes_read == chunk.num_bytes);
            break;

        case magicSubGroups:

            // add subgroups here

            break;

        case magicEoF:

            eof_found = true;
            break;

        default:
            //printf("Unknown chunk data type %08x of %d bytes while reading file %s\n", chunk.magic_id, chunk.num_bytes, filename.c_str());
            break;
        }
    }

    // Replace this with an efficient system without hardcoded sizes!
    int vertex_size = header.bytes_per_vtx / sizeof(float);

    for (unsigned int i = 0; i < temp_vtxs.size() - vertex_size; i = i + vertex_size) {

        vertices.insert(vertices.end(), { temp_vtxs[i], temp_vtxs[i + 1], temp_vtxs[i + 2] });
        normals.insert(normals.end(), { temp_vtxs[i + 3], temp_vtxs[i + 4], temp_vtxs[i + 5] });
        //uvs.insert(uvs.end(), { temp_vtxs[i + 6], temp_vtxs[i + 7] });
        uvs.push_back(temp_vtxs[i + 6]);
        uvs.push_back(temp_vtxs[i + 7]);
    }

    fclose(f);
    return true;
}

// load uncompressed RGB targa file into an OpenGL texture
GLint Parsers::parseTexture(std::string filename) {
	std::string str = filename;
	std::string ext = str.substr(str.size() - 4, 4);


	GLuint texture_id;

	if (ext == ".tga" || ext == ".TGA")
	{
		TGAInfo* tgainfo = loadTGA(filename);
		if (tgainfo == NULL) {
			std::cerr << "ERROR: Could not load TGA file" << std::endl;
			return false;
		}

		//generate new openGL texture and bind it (tell openGL we want to do stuff with it)
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id); //we are making a regular 2D texture

												  //screen pixels will almost certainly not be same as texture pixels, so we need to
												  //set some parameters regarding the filter we use to deal with these cases
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//set the mag filter
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //set the min filter
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4); //use anisotropic filtering

																		  //this is function that actually loads texture data into OpenGL
		glTexImage2D(GL_TEXTURE_2D, //the target type, a 2D texture
			0, //the base level-of-detail in the mipmap
			(tgainfo->bpp == 24 ? GL_RGB : GL_RGBA), //specified the color channels for opengl
			tgainfo->width, //the width of the texture
			tgainfo->height, //the height of the texture
			0, //border - must always be 0
			(tgainfo->bpp == 24 ? GL_BGR : GL_BGRA), //the format of the incoming data
			GL_UNSIGNED_BYTE, //the type of the incoming data
			tgainfo->data); // a pointer to the incoming data

							//we want to use mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);

		//clean up memory
		delete tgainfo->data;
		delete tgainfo;
		return texture_id;
	}
	else {
		std::cerr << "ERROR: No extension or extension not supported" << std::endl;
		return -1;
	}
}

// this reader supports only uncompressed RGB targa files with no colour table
TGAInfo* Parsers::loadTGA(std::string filename)
{
	//the TGA header is 18 bytes long. The first 12 bytes are for specifying the compression
	//and various fields that are very infrequently used, and hence are usually 0.
	//for this limited file parser, we start by reading the first 12 bytes and compare
	//them against the pattern that identifies the file a simple, uncompressed RGB file.
	//more info about the TGA format cane be found at http://www.paulbourke.net/dataformats/tga/

	char TGA_uncompressed[12] = { 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	char TGA_compare[12];
	char info_header[6];
	GLuint bytes_per_pixel;
	GLuint image_size;

	//open file
	std::ifstream file(filename, std::ios::binary);

	//read first 12 bytes
	file.read(&TGA_compare[0], 12);
	std::streamsize read_header_12 = file.gcount();
	//compare to check that file in uncompressed (or not corrupted)
	int header_compare = memcmp(TGA_uncompressed, TGA_compare, sizeof(TGA_uncompressed));
	if (read_header_12 != sizeof(TGA_compare) || header_compare != 0) {
		std::cerr << "ERROR: TGA file is not in correct format or corrupted: " << filename << std::endl;
		file.close();
		return nullptr;
	}

	//read in next 6 bytes, which contain 'important' bit of header
	file.read(&info_header[0], 6);

	TGAInfo* tgainfo = new TGAInfo;

	tgainfo->width = info_header[1] * 256 + info_header[0]; //width is stored in first two bytes of info_header
	tgainfo->height = info_header[3] * 256 + info_header[2]; //height is stored in next two bytes of info_header

	if (tgainfo->width <= 0 || tgainfo->height <= 0 || (info_header[4] != 24 && info_header[4] != 32)) {
		file.close();
		delete tgainfo;
		std::cerr << "ERROR: TGA file is not 24 or 32 bits, or has no width or height: " << filename << std::endl;
		return NULL;
	}

	//calculate bytes per pixel and then total image size in bytes
	tgainfo->bpp = info_header[4];
	bytes_per_pixel = tgainfo->bpp / 8;
	image_size = tgainfo->width * tgainfo->height * bytes_per_pixel;

	//reserve memory for the image data
	tgainfo->data = (GLubyte*)malloc(image_size);

	//read data into memory
	file.read((char*)tgainfo->data, image_size);
	std::streamsize image_read_size = file.gcount();

	//check it has been read correctly
	if (image_read_size != image_size) {
		if (tgainfo->data != NULL)
			free(tgainfo->data);
		file.close();
		std::cerr << "ERROR: Could not read tga data: " << filename << std::endl;
		delete tgainfo;
		return NULL;
	}

	file.close();

	return tgainfo;
}

// Method to parse the scene with new json structure
// No managers implemented, resources are loaded more than once!!

bool Parsers::parseScene(std::string filename, GraphicsSystem & graphics_system)
{
    // Set the json stream to be read
    std::ifstream json_file(filename);
    rapidjson::IStreamWrapper json_stream(json_file);
    rapidjson::Document json;
    json.ParseStream(json_stream);

    printf("Parsing Scene Name = %s\n", filename);

    //check if its valid JSON
    if (json.HasParseError()) std::cerr << "JSON format is not valid!" << std::endl;
    if (!json.HasMember("entities")) { std::cerr << "JSON file is incomplete! Needs entry: entities" << std::endl; return false; }

    // Create a default shader by now
    Shader* new_shader = graphics_system.loadShader("data/shaders/phong.vert", "data/shaders/phong.frag");
    new_shader->name = "phong";
    shaders["phong"] = new_shader->program;

    std::unordered_map<std::string, std::string> child_parent;

    for (rapidjson::SizeType i = 0; i < json["entities"].Size(); i++) {

        //get values from json
        rapidjson::Value & entity = json["entities"][i];      
        int entity_id = parseEntity(entity, graphics_system);
        Entity ent = ECS.entities[entity_id];

        // Stablish a parent/children relationship if needed
        if (entity.HasMember("parent")) {
            std::string json_parent = entity["parent"].GetString();
            if (ent.name == "" || json_parent == "") std::cerr << "ERROR: Parser: Either parent or child has no name";
            child_parent[ent.name] = json_parent;
        }
    }

    //now link hierarchy need to get transform id from parent entity, 
    //and link to transform object from child entity
    for (std::pair<std::string, std::string> relationship : child_parent)
    {
       //TO-DO
       // Call the parseEntity function
        int parent_entity_id = ECS.getEntity(relationship.second);
        Entity & parent = ECS.entities[parent_entity_id];
        int parent_transform_id = parent.components[0];

        Transform & transform_comp = ECS.getComponentFromEntity<Transform>(relationship.first);
        transform_comp.parent = parent_transform_id;
    }

    return false;
}

// I read my json object per entity
int Parsers::parseEntity(rapidjson::Value & entity, GraphicsSystem & graphics_system)
{
    std::string name = "";
    if (entity.HasMember("name"))
        name = entity["name"].GetString();

    int ent_id = -1;
    if (entity.HasMember("prefab")) {
        /// In case of prefab entity, load the entity and then ignore it's default transform and name
        std::ifstream json_file(entity["prefab"].GetString());
        rapidjson::IStreamWrapper json_stream(json_file);
        rapidjson::Document json;
        json.ParseStream(json_stream);

        // Add support for multiple entities in prefab
        ent_id = parseEntity(json["entities"][0], graphics_system);
        ECS.entities[ent_id].name = name;
    }
    else {
        // Create the entity with the given name
        ent_id = ECS.createEntity(name);
    }

    //load transform component, mandatory field
    if (entity.HasMember("transform")) {

        auto& ent_transform = ECS.getComponentFromEntity<Transform>(ent_id);
        ent_transform.Load(entity, ent_id);
    }

    // Load render, geometry and materials
    if (entity.HasMember("render")) {

        int geo_id = Geometry::Load(graphics_system, entity, ent_id);
        int mat_id = Material::Load(graphics_system, entity, ent_id);

        Mesh& ent_mesh = ECS.createComponentForEntity<Mesh>(ent_id);
        ent_mesh.geometry = geo_id;
        ent_mesh.material = mat_id;
    }

    // Load collider parameters
    if (entity.HasMember("collider")) {

        Collider& collider = ECS.createComponentForEntity<Collider>(ent_id);
        collider.Load(entity, ent_id);
    }

    // Add the light component
    if (entity.HasMember("light")) {

        Light& light = ECS.createComponentForEntity<Light>(ent_id);
        light.Load(entity, ent_id);
    }

    // Custom components here!

    // Parse custom components here
    // Add the light component
    if (entity.HasMember("rotator")) {

        // Call rotator load method
        // Parse his information and create the component.

    }

    return ent_id;
}
