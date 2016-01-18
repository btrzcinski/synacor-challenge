#include <iostream>

#include "args.h"
#include "codestr.h"
#include "file.h"

using namespace Frontend;

int main(int argc, char *argv[])
{
    Arguments args(argc, argv);

    try
    {    
        switch (args.type)
        {
            case Arguments::InputType::Code:
                interpret_code_str(args.arg);
                break;
            case Arguments::InputType::DisassembleFile:
                disassemble_file(args.arg);
                break;
            case Arguments::InputType::File:
                interpret_file(args.arg);
                break;
            default:
                std::cout << "Specify -d, -c, or -f" << std::endl;
        }
    }
    catch (std::exception const& ex)
    {
        std::cerr << "Error during VM execution: " << ex.what() << std::endl;
    }

    std::cout << std::endl;
    
    return 0;
}
