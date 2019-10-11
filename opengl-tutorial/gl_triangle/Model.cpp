#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(const std::string& filename) : filename(filename)
{
	Assimp::Importer importer;

}
