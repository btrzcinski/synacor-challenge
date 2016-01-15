#include <iostream>

#include "be.h"
#include "args.h"
#include "codestr.h"

using namespace Backend;
using namespace Frontend;

int main(int argc, char *argv[])
{
	Arguments args(argc, argv);
	
	switch (args.type)
	{
		case Arguments::InputType::Code:
			interpret_code_str(args.arg);
			break;
		default:
			std::cout << "Only -c option is supported" << std::endl;
	}
	
	return 0;
}
