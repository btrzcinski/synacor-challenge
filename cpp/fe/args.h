#include <string>

namespace Frontend
{
	struct Arguments
	{
		enum class InputType
		{
			None,
			File,
			Code
		};
		
		Arguments(int argc, char *argv[]);
		
		InputType type;
		std::string arg;
	};
}