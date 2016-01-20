#include "file.h"

#include "vm.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <stdexcept>

using namespace Backend;
using namespace Frontend;
using std::uint16_t;

void Frontend::disassemble_file(std::string const& filename)
{
    auto code_points = code_points_from_file(filename);

    VirtualMachine vm(code_points);
    vm.disassemble_to_file(filename + ".sasm");

    std::cout << "Disassembled " << filename << " to " << filename + ".sasm" << std::endl;
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
