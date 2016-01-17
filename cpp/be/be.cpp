#include "be.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

using namespace Backend;

using std::uint16_t;

VirtualMachine::VirtualMachine(std::vector<uint16_t> const& init_mem) :
    running(true),
    expectation(Expectation::Instruction),
    instruction(nullptr),
    program_counter(0)
{
    registers.fill(0);
    memory.fill(0);

    add_instruction(0,  0, &VirtualMachine::halt_fn);
    add_instruction(9,  3, &VirtualMachine::add_fn);
    add_instruction(19, 1, &VirtualMachine::out_fn);
    add_instruction(21, 0, &VirtualMachine::nop_fn);

    std::copy(init_mem.cbegin(), init_mem.cend(), memory.begin());
}

VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::run()
{
    if (!running)
    {
        throw std::logic_error("The VM is halted");
    }

    while (running)
    {
        auto word = memory.at(program_counter);
        next_word(word);
        ++program_counter;
    }
}

bool VirtualMachine::is_running()
{
    return running;
}

void VirtualMachine::next_word(uint16_t word)
{
    if (!running)
    {
        throw std::logic_error("The VM is halted");
    }

    if (expectation == Expectation::Instruction)
    {
        auto mappedInstruction = opcodeInstructionMap.find(word);
        if (mappedInstruction == opcodeInstructionMap.end())
        {
            std::cerr << "Warning: unknown opcode " << word << std::endl;
            return;
        }

        instruction = &(mappedInstruction->second);
        arguments.clear();

        if (instruction->numArguments > 0)
        {
            expectation = Expectation::Argument;
        }
        else
        {
            running = CALL_MEMBER_FN(this, instruction->fn)();
        }
    }
    else if (expectation == Expectation::Argument)
    {
        arguments.push_back(word);
        if (arguments.size() == instruction->numArguments)
        {
            running = CALL_MEMBER_FN(this, instruction->fn)();
            expectation = Expectation::Instruction;
        }
    }
}

void VirtualMachine::add_instruction(uint16_t opcode, int numArguments, InstructionFn fn)
{
    opcodeInstructionMap.emplace(opcode, Instruction(opcode, numArguments, fn));
}

uint16_t VirtualMachine::lookup_value(uint16_t value)
{
    if (value < 32768)
    {
        return value;
    }

    if (value > 32767 && value < 32776)
    {
        return registers.at(value - 32768);
    }

    throw std::out_of_range("Values cannot be any higher than 32776");
}

bool VirtualMachine::halt_fn()
{
    // Opcode 0
    // HALT
    // Stop execution and terminate the program.

    return false;
}

bool VirtualMachine::add_fn()
{
    // Opcode 9
    // ADD a b c
    // Store in a the sum of b and c.
    
    auto a = arguments.at(0);
    auto b = arguments.at(1);
    auto c = arguments.at(2);

    auto result = (lookup_value(b) + lookup_value(c)) % 32768;

    // a is expected to be a register
    if (a < 32768 || a > 32775)
    {
        throw std::out_of_range("Destination must be a register 0-7");
    }

    registers.at(a - 32768) = result;

    return true;
}

bool VirtualMachine::out_fn()
{
    // Opcode 19
    // OUT a
    // Write the character represented by ascii code <a> to the terminal
    
    auto arg = arguments.at(0);
    char ascii(lookup_value(arg));
    std::cout << ascii;

    return true;
}

bool VirtualMachine::nop_fn()
{
    // Opcode 21
    // NOP
    // Do nothing.
    
    return true;
}
