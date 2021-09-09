#include <iostream>
#include "emulator.h"

int main(int argc, char const *argv[])
{
    if(argc < 2)
    {
        std::cout << "Program name not given" << std::endl;
        return -1;
    }
    emulator::instance()->loadMem(argv[1]);
    emulator::instance()->start();
    emulator::deleteInstance();
    return 0;
}
