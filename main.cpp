#include "z80.hpp"
#include<iostream>


int main()
{
    Z80::Z80 cpu;

    cpu.load("ti83plus.rom");
    while(true)
    {
        cpu.step();
    }

    return 0;
}
