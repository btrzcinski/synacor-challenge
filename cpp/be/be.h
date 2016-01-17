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

        void add_instruction(std::uint16_t opcode, int numArguments, InstructionFn fn);

        // 0..32767 returns the value itself
        // 32768..32775 return the values from registers 0-7
        // 32776..65535 throw an exception
        std::uint16_t lookup_value(std::uint16_t value);

        // Turns an address [32768,32776] into [0,7], throwing an exception
        // if the address is not in that range.
        std::uint16_t check_register_address(std::uint16_t address);

        // Checks that an address is in the range [0,32767] and returns it back.
        std::uint16_t check_memory_address(std::uint16_t address);        

        bool halt_fn();
        bool set_fn();
        bool push_fn();
        bool pop_fn();
        bool eq_fn();
        bool gt_fn();
        bool jmp_fn();
        bool jt_fn();
        bool jf_fn();
        bool add_fn();
        bool mult_fn();
        bool mod_fn();
        bool and_fn();
        bool or_fn();
        bool not_fn();
        bool rmem_fn();
        bool wmem_fn();
        bool call_fn();
        bool ret_fn();
        bool out_fn();
        bool nop_fn();

        void jump_pc_to(std::uint16_t address);

        bool running;

        Expectation expectation;
        Instruction const* instruction;
        std::vector<std::uint16_t> arguments;

        std::unordered_map<std::uint16_t, Instruction> opcodeInstructionMap;

        std::uint16_t program_counter;

        std::stack<std::uint16_t> stack;
        std::array<std::uint16_t, 8> registers;
        std::array<std::uint16_t, 0x8000> memory;
	};
}
