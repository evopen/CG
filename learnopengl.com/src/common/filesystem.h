#include <string>
#include <filesystem>

namespace filesystem
{
	std::string getResourcesPath()
	{
		return std::filesystem::absolute("../../../resources/").string();
	}
}
