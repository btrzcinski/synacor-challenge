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
		
		void next_word(std::uint16_t word);

    private:
        enum class Expectation
        {
            Instruction,
            Argument
        };

        typedef void (VirtualMachine::*InstructionFn)(void);

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

        void nop_fn();

        Expectation expectation;
        Instruction const* instruction;
        std::vector<std::uint16_t> arguments;

        std::unordered_map<std::uint16_t, Instruction> opcodeInstructionMap;
	};
}
