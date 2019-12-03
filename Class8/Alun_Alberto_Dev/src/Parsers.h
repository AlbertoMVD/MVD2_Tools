#pragma once
#include "includes.h"
#include <vector>
#include "GraphicsSystem.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"

struct TGAInfo //stores info about TGA file
{
	GLuint width;
	GLuint height;
	GLuint bpp; //bits per pixel
	GLubyte* data; //bytes with the pixel information
};

class Parsers {
private:
	static TGAInfo* loadTGA(std::string filename);

    static int parseEntity(rapidjson::Value & entity,
                                GraphicsSystem & graphics_system);

public:
	static bool parseOBJ(std::string filename, 
						 std::vector<float>& vertices, 
						 std::vector<float>& uvs, 
						 std::vector<float>& normals,
						 std::vector<unsigned int>& indices);

    static bool parseBin(std::string filename,
        std::vector<float>& vertices,
        std::vector<float>& uvs,
        std::vector<float>& normals,
        std::vector<unsigned int>& indices);

	static GLint parseTexture(std::string filename);

    static bool parseScene(std::string filename, GraphicsSystem& graphics_system);
};
