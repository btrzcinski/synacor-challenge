#include "vm.h"

#include <cassert>
#include <cstdio>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <signal.h>
#include <string.h>
#include <sys/select.h>

#define CALL_MEMBER_FN(object,ptrToMember)  ((object)->*(ptrToMember))

using namespace Backend;
using std::uint16_t;

namespace
{
    VirtualMachine* g_vm = nullptr;

    void vm_signal_handler(int signum)
    {
        if (signum != SIGUSR1 && signum != SIGUSR2)
        {
            return;
        }

        if (g_vm != nullptr)
        {
            if (signum == SIGUSR1)
            {
                if (!g_vm->debugging())
                {                    
                    g_vm->start_debugging();
                    g_vm->dump();
                }
                else
                {
                    g_vm->stop_debugging();
                }
            }
            else
            {
                g_vm->code_7_override();
            }
        }
    }

    void setup_usr1_signal()
    {
        struct sigaction act;
        act.sa_handler = &vm_signal_handler;
        bzero(&act.sa_mask, sizeof(act.sa_mask));
        act.sa_flags = 0;

        if (sigaction(SIGUSR1, &act, nullptr) < 0)
        {
            throw std::runtime_error("Could not trap SIGUSR1: sigaction retval < 0");
        }
        
        if (sigaction(SIGUSR2, &act, nullptr) < 0)
        {
            throw std::runtime_error("Could not trap SIGUSR2: sigaction retval < 0");
        }
    }
}

VirtualMachine::VirtualMachine(std::vector<uint16_t> const& init_mem) :
    running(true),
    expectation(Expectation::Instruction),
    instruction(nullptr),
    program_counter(0),
    input_log("input.log"),
    debug_mode(false)
{
    registers.fill(0);
    memory.fill(0);

    add_instruction(0,  "HALT", 0, &VirtualMachine::halt_fn);
    add_instruction(1,  "SET",  2, &VirtualMachine::set_fn);
    add_instruction(2,  "PUSH", 1, &VirtualMachine::push_fn);
    add_instruction(3,  "POP",  1, &VirtualMachine::pop_fn);
    add_instruction(4,  "EQ",   3, &VirtualMachine::eq_fn);
    add_instruction(5,  "GT",   3, &VirtualMachine::gt_fn);
    add_instruction(6,  "JMP",  1, &VirtualMachine::jmp_fn);
    add_instruction(7,  "JT",   2, &VirtualMachine::jt_fn);
    add_instruction(8,  "JF",   2, &VirtualMachine::jf_fn);
    add_instruction(9,  "ADD",  3, &VirtualMachine::add_fn);
    add_instruction(10, "MULT", 3, &VirtualMachine::mult_fn);
    add_instruction(11, "MOD",  3, &VirtualMachine::mod_fn);
    add_instruction(12, "AND",  3, &VirtualMachine::and_fn);
    add_instruction(13, "OR",   3, &VirtualMachine::or_fn);
    add_instruction(14, "NOT",  2, &VirtualMachine::not_fn);
    add_instruction(15, "RMEM", 2, &VirtualMachine::rmem_fn);
    add_instruction(16, "WMEM", 2, &VirtualMachine::wmem_fn);
    add_instruction(17, "CALL", 1, &VirtualMachine::call_fn);
    add_instruction(18, "RET",  0, &VirtualMachine::ret_fn);
    add_instruction(19, "OUT",  1, &VirtualMachine::out_fn);
    add_instruction(20, "IN",   1, &VirtualMachine::in_fn);
    add_instruction(21, "NOOP", 0, &VirtualMachine::nop_fn);

    std::copy(init_mem.cbegin(), init_mem.cend(), memory.begin());

    g_vm = this;
    setup_usr1_signal();
}

VirtualMachine::~VirtualMachine()
{
    g_vm = nullptr;
}

void VirtualMachine::run()
{
    if (!running)
    {
        throw std::logic_error("The VM is halted");
    }

    while (running)
    {
        if (debug_mode)
        {
            dump();
        }

        auto word = memory.at(program_counter);
        next_word(word);
        ++program_counter;
    }
}

bool VirtualMachine::is_running() const
{
    return running;
}

bool VirtualMachine::debugging() const
{
    return debug_mode;
}

void VirtualMachine::start_debugging()
{
    debug_mode = true;
    std::cerr << "Started debugging" << std::endl;
}

void VirtualMachine::stop_debugging()
{
    debug_mode = false;
    std::cerr << "Stopped debugging" << std::endl;
}

void VirtualMachine::dump() const
{
    auto word = memory.at(program_counter);
    std::fprintf(stderr, "PC = 0x%04x -> 0x%04x", program_counter, word);
    auto mappedInstruction = opcodeInstructionMap.find(word);
    if (mappedInstruction != opcodeInstructionMap.end())
    {
        auto& pcInst = mappedInstruction->second;
        std::fprintf(stderr, " (%s, %d args)", pcInst.name.c_str(), pcInst.numArguments);
    }
    std::fprintf(stderr, "\n");
    std::fprintf(stderr, "R0 = 0x%04x, R1 = 0x%04x, R2 = 0x%04x, R3 = 0x%04x\n",
            registers.at(0), registers.at(1), registers.at(2), registers.at(3));
    std::fprintf(stderr, "R4 = 0x%04x, R5 = 0x%04x, R6 = 0x%04x, R7 = 0x%04x\n",
            registers.at(4), registers.at(5), registers.at(6), registers.at(7));
    std::fprintf(stderr, "\n");
}

void VirtualMachine::code_7_override()
{
    // Set R7 to 25734
    std::cerr << "Override: set reg 7 to 25734" << std::endl;
    registers.at(7) = 25734;
    
    // Set 0x1566 to JMP 0x157a
    std::cerr << "Override: set [0x1566, 0x1567] to JMP 0x157a" << std::endl;
    memory.at(0x1566) = 6;
    memory.at(0x1567) = 0x157a;
}

void VirtualMachine::disassemble_to_file(std::string const& filename) const
{
    using std::setw;
    using std::setfill;

    auto file_out = std::ofstream(filename);
    file_out << std::hex;
    file_out << "Byte    Addr    Inst  Args" << std::endl;

    auto local_pc = uint16_t(0);
    while (local_pc < memory.size())
    {
        file_out << "0x" << setw(4) << setfill('0') << local_pc * 2 << "  ";
        file_out << "0x" << setw(4) << setfill('0') << local_pc << "  ";

        auto inst_word = memory.at(local_pc);
        auto mappedInstruction = opcodeInstructionMap.find(inst_word);
        if (mappedInstruction == opcodeInstructionMap.end())
        {
            file_out << "Unknown: 0x" << setw(4) << setfill('0') << inst_word;
        }
        else
        {
            auto& inst = mappedInstruction->second;
            file_out << std::left << setw(4) << setfill(' ') << inst.name << std::right;
            if (inst.numArguments > 0)
            {
                ++local_pc;
                auto arg = memory.at(local_pc);
                file_out << "  0x" << setw(4) << setfill('0') << arg;
                for (auto i = 1; i < inst.numArguments; ++i)
                {
                    ++local_pc;
                    arg = memory.at(local_pc);
                    file_out << ", 0x" << setw(4) << setfill('0') << arg;
                }
            }
        }

        file_out << std::endl;
        ++local_pc;
    }
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

void VirtualMachine::add_instruction(uint16_t opcode, std::string name, int numArguments, InstructionFn fn)
{
    opcodeInstructionMap.emplace(opcode, Instruction(opcode, name, numArguments, fn));
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

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    select(1, &readfds, nullptr, nullptr, nullptr);

    std::cin.read(&val, 1);
    if (val != '\0')
    {
        input_log.put(val);
        assert(!input_log.bad());
        input_log.flush();
    }

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

