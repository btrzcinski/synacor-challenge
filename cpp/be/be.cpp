#include "be.h"

#include <iostream>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

using namespace Backend;

using std::uint16_t;

VirtualMachine::VirtualMachine() :
    expectation(Expectation::Instruction),
    instruction(nullptr)
{
    add_instruction(21, 0, &VirtualMachine::nop_fn);
}

VirtualMachine::~VirtualMachine()
{
}

void VirtualMachine::next_word(uint16_t word)
{
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
            CALL_MEMBER_FN(this, instruction->fn)();
        }
    }
    else if (expectation == Expectation::Argument)
    {
        arguments.push_back(word);
        if (arguments.size() == instruction->numArguments)
        {
            CALL_MEMBER_FN(this, instruction->fn)();
            expectation = Expectation::Instruction;
        }
    }
}

void VirtualMachine::add_instruction(std::uint16_t opcode, int numArguments, InstructionFn fn)
{
    opcodeInstructionMap.emplace(opcode, Instruction(opcode, numArguments, fn));
}

void VirtualMachine::nop_fn()
{
    // NOP = opcode 21
    // Do nothing.
 
    std::cout << "NOP" << std::endl;   
}
