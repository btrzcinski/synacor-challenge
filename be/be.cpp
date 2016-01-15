#include "be.h"

#include <iostream>

using namespace Backend;

using std::uint16_t;

VirtualMachine::VirtualMachine()
{
	
}

VirtualMachine::~VirtualMachine()
{
	
}

void VirtualMachine::next_word(uint16_t word)
{
	std::cout << word << std::endl;
}
