#include <array>
#include <cstdint>
#include <functional>
#include <stack>
#include <unordered_map>
#include <vector>

namespace Backend
{
	class VirtualMachine
	{
	public:
		VirtualMachine(std::vector<std::uint16_t> const& init_mem);
		virtual ~VirtualMachine();

        void run();
        bool is_running();
		
    private:
        enum class Expectation
        {
            Instruction,
            Argument
        };

        // An instruction returns false if the VM is supposed to halt.
        typedef bool (VirtualMachine::*InstructionFn)(void);

        struct Instruction
        {
            Instruction(std::uint16_t opcode, int numArguments, InstructionFn fn) :
                opcode(opcode),
                numArguments(numArguments),
                fn(fn)
            {
            }

            std::uint16_t opcode;
            int numArguments;
            InstructionFn fn;
        };

        void next_word(std::uint16_t word);

        std::uint16_t read_address(std::uint16_t address);
        std::uint16_t write_address(std::uint16_t address, std::uint16_t value);

        void add_instruction(std::uint16_t opcode, int numArguments, InstructionFn fn);

        bool halt_fn();
        bool out_fn();
        bool nop_fn();

        bool running;

        Expectation expectation;
        Instruction const* instruction;
        std::vector<std::uint16_t> arguments;

        std::unordered_map<std::uint16_t, Instruction> opcodeInstructionMap;

        std::uint16_t program_counter;

        std::stack<std::uint16_t> stack;
        std::array<std::uint16_t, 8> registers;
        std::array<std::uint16_t, 0x80> memory;
	};
}
