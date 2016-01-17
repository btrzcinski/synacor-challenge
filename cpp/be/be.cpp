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
    add_instruction(19, 1, &VirtualMachine::out_fn);
    add_instruction(21, 0, &VirtualMachine::nop_fn);

    // use init_mem to initialize memory
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

uint16_t VirtualMachine::read_address(uint16_t address)
{
    // Is this address a register?
    if (address > 0x7f)
    {
        auto register_num = address & 0x7f;
        if (register_num > 7)
        {
            throw std::out_of_range("Register addresses must be in range [0,7]");
        }

        return registers.at(register_num);
    }

    return memory.at(address);
}

uint16_t VirtualMachine::write_address(uint16_t address, uint16_t value)
{
    // Is this address a register?
    if (address > 0x7f)
    {
        auto register_num = address & 0x7f;
        if (register_num > 7)
        {
            throw std::out_of_range("Register addresses must be in range [0,7]");
        }

        return registers.at(register_num);
    }

    return memory.at(address);
}

void VirtualMachine::add_instruction(uint16_t opcode, int numArguments, InstructionFn fn)
{
    opcodeInstructionMap.emplace(opcode, Instruction(opcode, numArguments, fn));
}

bool VirtualMachine::halt_fn()
{
    // Opcode 0
    // HALT
    // Stop execution and terminate the program.

    return false;
}

bool VirtualMachine::out_fn()
{
    // Opcode 19
    // OUT a
    // Write the character represented by ascii code <a> to the terminal
    
    auto arg = arguments.at(0);
    char ascii(arg);
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
