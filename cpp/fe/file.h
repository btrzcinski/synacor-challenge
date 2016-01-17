#include <string>
#include <vector>
#include <cstdint>

namespace Frontend
{
    void interpret_file(std::string const& filename);
    
    std::vector<std::uint16_t> code_points_from_file(std::string const& filename);
}

