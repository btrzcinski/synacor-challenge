#include "args.h"

using namespace Frontend;

Arguments::Arguments(int argc, char *argv[])
{
    if (argc < 3)
    {
        type = InputType::None;
    }
    else
    {
        auto typeArg = std::string{argv[1]};
        arg = std::string{argv[2]};
        
        if (typeArg == "-f")
        {
            type = InputType::File;
        }
        else if (typeArg == "-c")
        {
            type = InputType::Code;
        }
        else if (typeArg == "-d")
        {
            type = InputType::DisassembleFile;
        }
        else
        {
            type = InputType::None;
        }
    }
}
