#include "../z80.hpp"
#include<iostream>

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " [filename]\n";
        return -1;
    }

    Z80::Z80 cpu;

    cpu.load(argv[1]);
    while(true)
    {
        cpu.step();
    }

    return 0;
}
