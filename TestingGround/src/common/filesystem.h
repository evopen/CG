#include <string>
#include <filesystem>

namespace filesystem
{
	std::string getResourcesPath()
	{
		if (exists(std::filesystem::path("./resources")))
			return std::filesystem::absolute("./resources/").string();
		else if (exists(std::filesystem::path("../resources")))
			return std::filesystem::absolute("../resources/").string();
		else if (exists(std::filesystem::path("../../resources")))
			return std::filesystem::absolute("../../resources/").string();
		else if (exists(std::filesystem::path("../../../resources")))
			return std::filesystem::absolute("../../../resources/").string();
		else
			throw std::runtime_error("failed to find resources folder");
	}
}
