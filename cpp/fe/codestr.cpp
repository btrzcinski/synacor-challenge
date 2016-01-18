#include "codestr.h"

#include "vm.h"

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

void Frontend::interpret_code_str(std::string const& code)
{
    auto code_points = code_points_from_str(code);
    
    VirtualMachine vm(code_points);
    vm.run();
}

std::vector<uint16_t> Frontend::code_points_from_str(std::string const& code)
{
    std::vector<uint16_t> code_points;
    
    auto start = 0;
    auto end = 0;
    while ((end = code.find(",", start)) != std::string::npos)
    {
        auto cp_s = code.substr(start, end - start);
        code_points.push_back(code_point_from_str(cp_s));
        start = end + 1;
    }
    
    code_points.emplace_back(code_point_from_str(code.substr(start)));
    return code_points;
}
