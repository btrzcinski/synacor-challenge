#include "be.h"

#include <cassert>
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
    program_counter(0),
    input_log("input.log")
{
    registers.fill(0);
    memory.fill(0);

    add_instruction(0,  0, &VirtualMachine::halt_fn);
    add_instruction(1,  2, &VirtualMachine::set_fn);
    add_instruction(2,  1, &VirtualMachine::push_fn);
    add_instruction(3,  1, &VirtualMachine::pop_fn);
    add_instruction(4,  3, &VirtualMachine::eq_fn);
    add_instruction(5,  3, &VirtualMachine::gt_fn);
    add_instruction(6,  1, &VirtualMachine::jmp_fn);
    add_instruction(7,  2, &VirtualMachine::jt_fn);
    add_instruction(8,  2, &VirtualMachine::jf_fn);
    add_instruction(9,  3, &VirtualMachine::add_fn);
    add_instruction(10, 3, &VirtualMachine::mult_fn);
    add_instruction(11, 3, &VirtualMachine::mod_fn);
    add_instruction(12, 3, &VirtualMachine::and_fn);
    add_instruction(13, 3, &VirtualMachine::or_fn);
    add_instruction(14, 2, &VirtualMachine::not_fn);
    add_instruction(15, 2, &VirtualMachine::rmem_fn);
    add_instruction(16, 2, &VirtualMachine::wmem_fn);
    add_instruction(17, 1, &VirtualMachine::call_fn);
    add_instruction(18, 0, &VirtualMachine::ret_fn);
    add_instruction(19, 1, &VirtualMachine::out_fn);
    add_instruction(20, 1, &VirtualMachine::in_fn);
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
            throw std::out_of_range("Unknown opcode encountered");
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

uint16_t VirtualMachine::check_register_address(uint16_t address)
{
    if (address < 32768 || address > 32776)
    {
        throw std::out_of_range("Register addresses must be in [32768,32776]");
    }

    return address - 32768;
}

uint16_t VirtualMachine::check_memory_address(uint16_t address)
{
    if (address > 32767)
    {
        throw std::out_of_range("Memory addresses must be in [0,32767]");
    }

    return address;
}

bool VirtualMachine::halt_fn()
{
    // Opcode 0
    // HALT
    // Stop execution and terminate the program.

    return false;
}

bool VirtualMachine::set_fn()
{
    // Opcode 1
    // SET a b
    // Set register a to the value of b.
    
    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));

    registers.at(a) = b;

    return true;
}

bool VirtualMachine::push_fn()
{
    // Opcode 2
    // PUSH a
    // Push the value of a onto the stack.

    auto a = lookup_value(arguments.at(0));

    stack.push(a);

    return true;
}

bool VirtualMachine::pop_fn()
{
    // Opcode 3
    // POP a
    // Set register a to the popped value off the stack.

    if (stack.empty())
    {
        throw std::logic_error("Cannot pop off of an empty stack");
    }

    auto a = check_register_address(arguments.at(0));
    registers.at(a) = stack.top();
    stack.pop();

    return true;    
}

bool VirtualMachine::eq_fn()
{
    // Opcode 4
    // EQ a b c
    // Set a to 1 if b == c; else, set a to 0.

    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    if (b == c)
    {
        registers.at(a) = 1;
    }
    else
    {
        registers.at(a) = 0;
    }

    return true;
}

bool VirtualMachine::gt_fn()
{
    // Opcode 5
    // GT a b c
    // Set a to 1 if b > c; else, set a to 0.

    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    if (b > c)
    {
        registers.at(a) = 1;
    }
    else
    {
        registers.at(a) = 0;
    }

    return true;
}

bool VirtualMachine::jmp_fn()
{
    // Opcode 6
    // JMP a
    // Jump the PC to a.

    auto a = lookup_value(arguments.at(0));

    jump_pc_to(a);

    return true;
}

bool VirtualMachine::jt_fn()
{
    // Opcode 7
    // JT a b
    // If a != 0, jump the PC to b.

    auto a = lookup_value(arguments.at(0));
    auto b = lookup_value(arguments.at(1));

    if (a != 0)
    {
        jump_pc_to(b);
    }

    return true;
}

bool VirtualMachine::jf_fn()
{
    // Opcode 8
    // JF a b
    // If a == 0, jump the PC to b.

    auto a = lookup_value(arguments.at(0));
    auto b = lookup_value(arguments.at(1));

    if (a == 0)
    {
        jump_pc_to(b);
    }

    return true;
}

bool VirtualMachine::add_fn()
{
    // Opcode 9
    // ADD a b c
    // Store in a the sum of b and c.
    
    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    auto result = (b + c) % 32768;

    registers.at(a) = result;

    return true;
}

bool VirtualMachine::mult_fn()
{
    // Opcode 10
    // MULT a b c
    // Store in a the product of b and c modulo 32768.

    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    auto result = (b * c) % 32768;

    registers.at(a) = result;
    
    return true;
}

bool VirtualMachine::mod_fn()
{
    // Opcode 11
    // MOD a b c
    // Store in a the result of b modulo c.

    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    auto result = b % c;

    registers.at(a) = result;

    return true;
}

bool VirtualMachine::and_fn()
{
    // Opcode 12
    // AND a b c
    // Store in a the bitwise and of b and c.

    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    auto result = b & c;

    registers.at(a) = result;
    
    return true;
}

bool VirtualMachine::or_fn()
{
    // Opcode 13
    // OR a b c
    // Store in a the bitwise or of b and c.

    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));
    auto c = lookup_value(arguments.at(2));

    auto result = b | c;

    registers.at(a) = result;

    return true;
}

bool VirtualMachine::not_fn()
{
    // Opcode 14
    // NOT a b
    // Store in a the bitwise inverse of b, taking care not to
    // invert the most significant bit. (e.g., the inverse is
    // only across the lower 15 bits)
    
    auto a = check_register_address(arguments.at(0));
    auto b = lookup_value(arguments.at(1));

    auto result = 0x7fff & (~b);

    registers.at(a) = result;

    return true;
}

bool VirtualMachine::rmem_fn()
{
    // Opcode 15
    // RMEM a b
    // Store in a the value at memory address b.

    auto a = check_register_address(arguments.at(0));
    auto b = check_memory_address(lookup_value(arguments.at(1)));

    registers.at(a) = memory.at(b);

    return true;
}

bool VirtualMachine::wmem_fn()
{
    // Opcode 16
    // WMEM a b
    // Store in memory address a the value of b.
    
    auto a = check_memory_address(lookup_value(arguments.at(0)));
    auto b = lookup_value(arguments.at(1));

    memory.at(a) = b;

    return true;
}

bool VirtualMachine::call_fn()
{
    // Opcode 17
    // CALL a
    // Push (PC + 1) onto the stack, then jump to a.

    auto a = lookup_value(arguments.at(0));

    stack.push(program_counter + 1);
    jump_pc_to(a);

    return true;
}

bool VirtualMachine::ret_fn()
{
    // Opcode 18
    // RET
    // Pop a value off the stack, then jump to it.

    if (stack.empty())
    {
        // Halt
        return false;
    }

    auto jmp_loc = stack.top();
    stack.pop();
    jump_pc_to(jmp_loc);

    return true;
}

bool VirtualMachine::out_fn()
{
    // Opcode 19
    // OUT a
    // Write the character represented by ascii code <a> to the terminal
    
    auto a = lookup_value(arguments.at(0));
    char ascii(a);
    std::cout << ascii;

    return true;
}

bool VirtualMachine::in_fn()
{
    // Opcode 20
    // IN a
    // Read a character from the terminal and write its ascii code to <a>

    auto a = check_register_address(arguments.at(0));
    char val;
    std::cin.get(val);
    input_log.put(val);
    assert(!input_log.bad());
    input_log.flush();

    registers.at(a) = uint16_t(val);

    return true;
}

bool VirtualMachine::nop_fn()
{
    // Opcode 21
    // NOP
    // Do nothing.
    
    return true;
}

void VirtualMachine::jump_pc_to(std::uint16_t address)
{
    // Because the PC always increments by 1 after a call,
    // we want to jump the PC to a - 1.
    program_counter = address - 1;
}

