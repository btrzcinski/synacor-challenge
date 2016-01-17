#include "file.h"

#include "be.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>

using namespace Backend;
using namespace Frontend;
using std::uint16_t;

namespace
{
	uint16_t code_point_from_str(std::string const& cp)
	{
		auto cp_i = std::atoi(cp.c_str());
		if (cp_i > UINT16_MAX)
		{
			throw std::overflow_error{"Code point greater than max unsigned 16-bit int"};
		}
		return static_cast<uint16_t>(cp_i);
	}
}

void Frontend::interpret_file(std::string const& filename)
{
	auto code_points = code_points_from_file(filename);
	
	VirtualMachine vm(code_points);
    vm.run();
}

std::vector<uint16_t> Frontend::code_points_from_file(std::string const& filename)
{
	std::vector<uint16_t> code_points;
    std::ifstream ifile(filename, std::ifstream::binary);

    uint16_t cp;
    while (ifile)
    {
        // Assumption is that both input file and platform running this code
        // are little-endian
        ifile.read(reinterpret_cast<char*>(&cp), 2);
        if (ifile.gcount() < 2)
        {
            break;
        }
        code_points.push_back(cp);
    }

    return code_points;
}
