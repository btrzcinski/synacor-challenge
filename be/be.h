#include <cstdint>

namespace Backend
{
	class VirtualMachine
	{
	public:
		VirtualMachine();
		virtual ~VirtualMachine();
		
		void next_word(std::uint16_t word);
	};
}
