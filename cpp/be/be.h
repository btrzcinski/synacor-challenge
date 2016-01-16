#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

namespace Backend
{
	class VirtualMachine
	{
	public:
		VirtualMachine();
		virtual ~VirtualMachine();
		
        // Returns true if the VM will continue to run after this word.
        // e.g.: false means the VM halted.
		bool next_word(std::uint16_t word);

    private:
        enum class Expectation
        {
            Instruction,
            Argument
        };

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

        void add_instruction(std::uint16_t opcode, int numArguments, InstructionFn fn);

        bool out_fn();
        bool nop_fn();

        bool running;

        Expectation expectation;
        Instruction const* instruction;
        std::vector<std::uint16_t> arguments;

        std::unordered_map<std::uint16_t, Instruction> opcodeInstructionMap;
	};
}
