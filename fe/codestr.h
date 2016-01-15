#include <string>
#include <vector>
#include <cstdint>

namespace Frontend
{
	void interpret_code_str(std::string const& code);
	
	std::vector<std::uint16_t> code_points_from_str(std::string const& code);
}